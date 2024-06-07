#include "parserBrain.h"

ParserBrain::ParserBrain(double p, const map<string, Generic>& lexeme_dict,
                         const vector<string>& all_areas, const vector<string>& recurrent_areas,
                         const vector<string>& initial_areas, const map<string, vector<string>>& readout_rules)
    : Brain(p), lexeme_dict(lexeme_dict), all_areas(all_areas),
      recurrent_areas(recurrent_areas), initial_areas(initial_areas), readout_rules(readout_rules) {
    initialize_states();
}

void ParserBrain::initialize_states() {
    for (const auto& from_area : all_areas) {
        fiber_states[from_area] = map<string, set<int>>();
        for (const auto& to_area : all_areas) {
            fiber_states[from_area][to_area].insert(0);
        }
    }

    for (const auto& area : all_areas) {
        area_states[area].insert(0);
    }

    for (const auto& area : initial_areas) {
        area_states[area].erase(0);
    }
}

void ParserBrain::applyFiberRule(const FiberRule& rule) {
    if (rule.action == "INHIBIT") {
        fiber_states[rule.area1][rule.area2].insert(rule.index);
        fiber_states[rule.area2][rule.area1].insert(rule.index);
    } else if (rule.action == "DISINHIBIT") {
        fiber_states[rule.area1][rule.area2].erase(rule.index);
        fiber_states[rule.area2][rule.area1].erase(rule.index);
    }
}

void ParserBrain::applyAreaRule(const AreaRule& rule) {
    if (rule.action == "INHIBIT") {
        area_states[rule.area].insert(rule.index);
    } else if (rule.action == "DISINHIBIT") {
        area_states[rule.area].erase(rule.index);
    }
}

bool ParserBrain::applyRule(const variant<AreaRule, FiberRule>& rule) {
    if (holds_alternative<FiberRule>(rule)) {
        applyFiberRule(get<FiberRule>(rule));
        return true;
    } else if (holds_alternative<AreaRule>(rule)) {
        applyAreaRule(get<AreaRule>(rule));
        return true;
    }
    return false;
}

void ParserBrain::parse_project() {
    auto project_map = getProjectMap();
    remember_fibers(project_map);
    // project({}, project_map);
}

void ParserBrain::remember_fibers(const std::unordered_map<std::string, std::vector<std::string>>& project_map) {
    for (const auto& [from_area, to_areas] : project_map) {
        for (const auto& to_area : to_areas) {
            if (std::find(activated_fibers[from_area].begin(), activated_fibers[from_area].end(), to_area) == activated_fibers[from_area].end()) {
                activated_fibers[from_area].push_back(to_area);
            }
        }
    }
}

bool ParserBrain::recurrent(const string& area) {
    return find(recurrent_areas.begin(), recurrent_areas.end(), area) != recurrent_areas.end();
}

std::unordered_map<std::string, std::vector<std::string>> ParserBrain::getProjectMap() {
    std::unordered_map<std::string, std::vector<std::string>> proj_map;
    for (const auto& area1 : all_areas) {
        if (area_states[area1].empty()) {
            for (const auto& area2 : all_areas) {
                if (area1 == "LEX" && area2 == "LEX") continue;
                if (area_states[area2].empty()) {
                    if (fiber_states[area1][area2].empty()) {
                        if (!area_by_name[area1]->winners.empty()) {
                            proj_map[area1].push_back(area2);
                        }
                        if (!area_by_name[area2]->winners.empty()) {
                            proj_map[area2].push_back(area2);
                        }
                    }
                }
            }
        }
    }
    return proj_map;
}

void ParserBrain::activateWord(const string& area_name, const string& word) {
    auto& area = area_by_name[area_name];
    int k = area->k;
    int assembly_start = lexeme_dict[word].index * k;
    area->winners.clear();
    for (int i = 0; i < k; ++i) {
        area->winners.push_back(assembly_start + i);
    }
    area->fixAssembly();
}

void ParserBrain::activateIndex(const string& area_name, int index) {
    auto& area = area_by_name[area_name];
    int k = area->k;
    int assembly_start = index * k;
    area->winners.clear();
    for (int i = 0; i < k; ++i) {
        area->winners.push_back(assembly_start + i);
    }
    area->fixAssembly();
}

string ParserBrain::interpretAssemblyAsString(const string& area_name) {
    return getWord(area_name, 0.7);
}

string ParserBrain::getWord(const string& area_name, double min_overlap) {
    if (area_by_name[area_name]->winners.empty()) {
        throw runtime_error("Cannot get word because no assembly in " + area_name);
    }
    set<int> winners(area_by_name[area_name]->winners.begin(), area_by_name[area_name]->winners.end());
    int area_k = area_by_name[area_name]->k;
    int threshold = min_overlap * area_k;

    for (const auto& [word, lexeme] : lexeme_dict) {
        int word_index = lexeme.index;
        set<int> word_assembly;
        for (int i = 0; i < area_k; ++i) {
            word_assembly.insert(word_index * area_k + i);
        }

        set<int> intersection;
        set_intersection(winners.begin(), winners.end(), word_assembly.begin(), word_assembly.end(),
                         inserter(intersection, intersection.begin()));

        if (intersection.size() >= threshold) {
            return word;
        }
    }
    return "";
}

std::unordered_map<std::string, std::vector<std::string>> ParserBrain::getActivatedFibers() const {
    std::unordered_map<std::string, std::vector<std::string>> pruned_activated_fibers;
    for (const auto& [from_area, to_areas] : activated_fibers) {
        for (const auto& to_area : to_areas) {
            if (std::find(readout_rules.at(from_area).begin(), readout_rules.at(from_area).end(), to_area) != readout_rules.at(from_area).end()) {
                pruned_activated_fibers[from_area].push_back(to_area);
            }
        }
    }
    return pruned_activated_fibers;
}
