#include "parserbrain.h"
#include <algorithm>

namespace pb {

ParserBrain::ParserBrain(float p, const std::map<std::string, Generic>& lexeme_dict,
                const std::vector<std::string>& all_areas,
                const std::vector<std::string>& recurrent_areas,
                const std::vector<std::string>& initial_areas,
                const std::map<std::string, std::vector<std::string>>& readout_rules) 
    : Brain(p, 0.2f, 100000.f, 0),
    lexeme_dict(lexeme_dict),
    all_areas(all_areas),
    recurrent_areas(recurrent_areas),
    initial_areas(initial_areas),
    readout_rules(readout_rules) {
    initialize_states();
};

void ParserBrain::initialize_states() {
    // 更新 fiber states
    for (const auto& from_area : all_areas) {
        fiber_states[from_area] = std::map<std::string, std::set<int>>();
        for (const auto& to_area : all_areas) {
            fiber_states[from_area][to_area].insert(0);
        }
    }

    // 更新 area states，除了 initial areas，都 add 0
    for (const auto& area : all_areas) {
        area_states[area].insert(0);
    }

    for (const auto& area : initial_areas) {
        area_states[area].erase(0);
    }  
}

void ParserBrain::applyFiberRule(const FiberRule& rule){
    if (rule.action == INHIBIT) {
        fiber_states[rule.area1][rule.area2].insert(rule.index);
        fiber_states[rule.area2][rule.area1].insert(rule.index);
    } else if (rule.action == DISINHIBIT) {
        fiber_states[rule.area1][rule.area2].erase(rule.index);
        fiber_states[rule.area2][rule.area1].erase(rule.index);
    }
}

void ParserBrain::applyAreaRule(const AreaRule& rule) {
    if (rule.action == INHIBIT) {
        area_states[rule.area].insert(rule.index);
    } else if (rule.action == DISINHIBIT) {
        area_states[rule.area].erase(rule.index);
    }
}

bool ParserBrain::applyRule(const std::variant<AreaRule, FiberRule>& rule) {
    if (std::holds_alternative<FiberRule>(rule)) {
        applyFiberRule(std::get<FiberRule>(rule));
        return true;
    } else if (std::holds_alternative<AreaRule>(rule)) {
        applyAreaRule(std::get<AreaRule>(rule));
        return true;
    }
    return false;
}

void ParserBrain::parse_project(){
    auto project_map = getProjectMap();
    remember_fibers(project_map);
    nemo::Brain::Project(project_map, 20, true);
}

void ParserBrain::remember_fibers(const std::map<std::string, std::vector<std::string>>& project_map) {
    for (const auto& [from_area, to_areas] : project_map) {
        activated_fibers[from_area].insert(to_areas.begin(), to_areas.end());
    }
}


// 不一致
std::map<std::string, std::vector<std::string>> ParserBrain::getProjectMap() {
    std::map<std::string, std::vector<std::string>> proj_map;
    for (const auto& area1 : all_areas) {
        if (area_states[area1].empty()) {
            for (const auto& area2 : all_areas) {
                if (area1 == LEX && area2 == LEX) {
                    continue;
                }
                if (area_states[area2].empty() && fiber_states[area1][area2].empty()) {
                    // 1. area2 states 长度为0
                    // 2. area1-area2 fiber disinhibit
                    if (area_by_name[area1].support) { 
                        // 此处与 parser.py 不一致。
                        // 我的理解：
                        // ⽀持集:⼀个脑区的⽀持集定义为在所有时间步中曾经活跃过的神经元集的并集
                        // winners： List of winners, as set by previous action.
                        // 所以只要 support > 0 与 .winners 等价
                        proj_map[area1].push_back(area2);
                    }
                    if (area_by_name[area2].support) { // Assuming winners is a special key
                        proj_map[area2].push_back(area2);
                    }
                }
            }
        }
    }
    return proj_map;
}

// 不一致
void ParserBrain::activateWord(const std::string& area_name, const std::string& word) {
    nemo::Area& area = area_by_name[area_name];
    int k = area.k;
    int assembly_start = lexeme_dict[word].index * k;

    // 不一致
    area.activated.clear();
    for (int i = 0; i < k; ++i) {
        area.activated.push_back(assembly_start + i);
    }
    // 不一致
    area.is_fixed = true;
}

void ParserBrain::activateIndex(const std::string& area_name, int index) {
    nemo::Area& area = area_by_name[area_name];
    int k = area.k;
    int assembly_start = index * k;
    // 不一致
    area.activated.clear();
    for (int i = 0; i < k; ++i) {
        area.activated.push_back(assembly_start + i);
    }
    // 不一致
    area.is_fixed = true;
}

std::string ParserBrain::interpretAssemblyAsString(const std::string& area_name) {
    return getWord(area_name, 0.7);
}

std::string ParserBrain::getWord(const std::string& area_name, double min_overlap) {
    if (area_by_name[area_name].activated.empty()) {
        throw std::runtime_error("Cannot get word because no assembly in " + area_name);
    }
    std::set<int> winners(area_by_name[area_name].activated.begin(), area_by_name[area_name].activated.end());
    int area_k = area_by_name[area_name].k;
    int threshold = static_cast<int>(min_overlap * area_k);
    
    for (const auto& [word, lexeme] : lexeme_dict) {
        int word_index = lexeme.index;
        std::set<int> word_assembly;
        for (int i = 0; i < area_k; ++i) {
            word_assembly.insert(word_index * area_k + i);
        }
        std::set<int> intersection;
        set_intersection(winners.begin(), winners.end(), word_assembly.begin(), word_assembly.end(), std::inserter(intersection, intersection.begin()));
        if (static_cast<int>(intersection.size()) >= threshold) {
            return word;
        }
    }
    return {};
}

std::map<std::string, std::set<std::string>> ParserBrain::getActivatedFibers() {
    std::map<std::string, std::set<std::string>> pruned_activated_fibers;
    for (const auto& [from_area, to_areas] : activated_fibers) {
        for (const auto& to_area : to_areas) {
            if (std::find(readout_rules[from_area].begin(), readout_rules[from_area].end(), to_area) != readout_rules[from_area].end()) {
                pruned_activated_fibers[from_area].insert(to_area);
            }
        }
    }
    return pruned_activated_fibers;
}

}