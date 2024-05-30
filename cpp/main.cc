#include "brain.h"
#include <iostream>
#include <string>
#include <variant>
#include <set>

// Brain Areas
const std::string LEX = "LEX";
const std::string DET = "DET";
const std::string SUBJ = "SUBJ";
const std::string OBJ = "OBJ";
const std::string VERB = "VERB";
const std::string PREP = "PREP";
const std::string PREP_P = "PREP_P";
const std::string ADJ = "ADJ";
const std::string ADVERB = "ADVERB";

// Fixed area stats for explicit areas
const int LEX_SIZE = 20;

// Actions
const std::string DISINHIBIT = "DISINHIBIT";
const std::string INHIBIT = "INHIBIT";

std::vector<std::string> AREAS = {LEX, DET, SUBJ, OBJ, VERB, ADJ, ADVERB, PREP, PREP_P};
std::vector<std::string> EXPLICIT_AREAS = {LEX};
std::vector<std::string> RECURRENT_AREAS = {SUBJ, OBJ, VERB, ADJ, ADVERB, PREP, PREP_P};

// 定义 AreaRule 结构体
struct AreaRule {
    std::string action;
    std::string area;
    int index;
};

// 定义 FiberRule 结构体
struct FiberRule {
    std::string action;
    std::string area1;
    std::string area2;
    int index;
};

// 定义 Generic 结构体
struct Generic {
    int index;
    std::vector<std::variant<AreaRule, FiberRule>> preRules;
    std::vector<std::variant<AreaRule, FiberRule>> postRules;
};

Generic generic_noun(int index) {
    Generic noun;
    noun.index = index;
    noun.preRules = {
        FiberRule{DISINHIBIT, LEX, SUBJ, 0},
        FiberRule{DISINHIBIT, LEX, OBJ, 0},
        FiberRule{DISINHIBIT, LEX, PREP_P, 0},
        FiberRule{DISINHIBIT, DET, SUBJ, 0},
        FiberRule{DISINHIBIT, DET, OBJ, 0},
        FiberRule{DISINHIBIT, DET, PREP_P, 0},
        FiberRule{DISINHIBIT, ADJ, SUBJ, 0},
        FiberRule{DISINHIBIT, ADJ, OBJ, 0},
        FiberRule{DISINHIBIT, ADJ, PREP_P, 0},
        FiberRule{DISINHIBIT, VERB, OBJ, 0},
        FiberRule{DISINHIBIT, PREP_P, PREP, 0},
        FiberRule{DISINHIBIT, PREP_P, SUBJ, 0},
        FiberRule{DISINHIBIT, PREP_P, OBJ, 0}
    };
    noun.postRules = {
        AreaRule{INHIBIT, DET, 0},
		AreaRule{INHIBIT, ADJ, 0},
		AreaRule{INHIBIT, PREP_P, 0},
		AreaRule{INHIBIT, PREP, 0},
		FiberRule{INHIBIT, LEX, SUBJ, 0},
		FiberRule{INHIBIT, LEX, OBJ, 0},
		FiberRule{INHIBIT, LEX, PREP_P, 0},
		FiberRule{INHIBIT, ADJ, SUBJ, 0},
		FiberRule{INHIBIT, ADJ, OBJ, 0},
		FiberRule{INHIBIT, ADJ, PREP_P, 0},
		FiberRule{INHIBIT, DET, SUBJ, 0},
		FiberRule{INHIBIT, DET, OBJ, 0},
		FiberRule{INHIBIT, DET, PREP_P, 0},
		FiberRule{INHIBIT, VERB, OBJ, 0},
		FiberRule{INHIBIT, PREP_P, PREP, 0},
		FiberRule{INHIBIT, PREP_P, VERB, 0},
		FiberRule{DISINHIBIT, LEX, SUBJ, 1},
		FiberRule{DISINHIBIT, LEX, OBJ, 1},
		FiberRule{DISINHIBIT, DET, SUBJ, 1},
		FiberRule{DISINHIBIT, DET, OBJ, 1},
		FiberRule{DISINHIBIT, ADJ, SUBJ, 1},
		FiberRule{DISINHIBIT, ADJ, OBJ, 1},
		FiberRule{INHIBIT, PREP_P, SUBJ, 0},
		FiberRule{INHIBIT, PREP_P, OBJ, 0},
		FiberRule{INHIBIT, VERB, ADJ, 0}
    };

    return noun;
}

Generic generic_trans_verb(int index) {
    Generic trans_verb;
    trans_verb.index = index;
    trans_verb.preRules = {
        FiberRule{DISINHIBIT, LEX, VERB, 0},
		FiberRule{DISINHIBIT, VERB, SUBJ, 0},
		FiberRule{DISINHIBIT, VERB, ADVERB, 0},
		AreaRule{DISINHIBIT, ADVERB, 1},
    };
    trans_verb.postRules = {
		FiberRule{INHIBIT, LEX, VERB, 0},
		AreaRule{DISINHIBIT, OBJ, 0},
		AreaRule{INHIBIT, SUBJ, 0},
		AreaRule{INHIBIT, ADVERB, 0},
		FiberRule{DISINHIBIT, PREP_P, VERB, 0},
    };
    return trans_verb;
}

Generic generic_intrans_verb(int index) {
    Generic intrans_verb;
    intrans_verb.index = index;
    intrans_verb.preRules = {
		FiberRule{DISINHIBIT, LEX, VERB, 0},
		FiberRule{DISINHIBIT, VERB, SUBJ, 0},
		FiberRule{DISINHIBIT, VERB, ADVERB, 0},
		AreaRule{DISINHIBIT, ADVERB, 1}
    };

    intrans_verb.postRules = {
		FiberRule{INHIBIT, LEX, VERB, 0},
		AreaRule{INHIBIT, SUBJ, 0},
		AreaRule{INHIBIT, ADVERB, 0},
		FiberRule{DISINHIBIT, PREP_P, VERB, 0},       
    };
    return intrans_verb;
}

Generic generic_copula(int index) {
    Generic copula;
    copula.index = index;
    copula.preRules = {
        FiberRule{DISINHIBIT, LEX, VERB, 0},
		FiberRule{DISINHIBIT, VERB, SUBJ, 0},
    };
    copula.postRules = {
		FiberRule{INHIBIT, LEX, VERB, 0},
		AreaRule{DISINHIBIT, OBJ, 0},
		AreaRule{INHIBIT, SUBJ, 0},
		FiberRule{DISINHIBIT, ADJ, VERB, 0}
    };
    return copula;
}

Generic generic_adverb(int index) {
    Generic adverb;
    adverb.index = index;
    adverb.preRules = {
		AreaRule{DISINHIBIT, ADVERB, 0},
		FiberRule{DISINHIBIT, LEX, ADVERB, 0}
    };
    adverb.postRules = {
		FiberRule{INHIBIT, LEX, ADVERB, 0},
		AreaRule{INHIBIT, ADVERB, 1},
    };
    return adverb;
}

Generic generic_determinant(int index) {
    Generic determinant;
    determinant.index = index;
    determinant.preRules = {
		AreaRule{DISINHIBIT, DET, 0},
		FiberRule{DISINHIBIT, LEX, DET, 0}
    };
    determinant.postRules = {
		FiberRule{INHIBIT, LEX, DET, 0},
		FiberRule{INHIBIT, VERB, ADJ, 0},
    };
    return determinant;
}

Generic generic_adjective(int index) {
    Generic adjective;
    adjective.index = index;
    adjective.preRules = {
		AreaRule{DISINHIBIT, ADJ, 0},
		FiberRule{DISINHIBIT, LEX, ADJ, 0}
    };
    adjective.postRules = {
		FiberRule{INHIBIT, LEX, ADJ, 0},
		FiberRule{INHIBIT, VERB, ADJ, 0},
    };
    return adjective;
}

Generic generic_preposition(int index) {
    Generic preposition;
    preposition.index = index;
    preposition.preRules = {
		AreaRule{DISINHIBIT, PREP, 0},
		FiberRule{DISINHIBIT, LEX, PREP, 0},
    };
    preposition.postRules = {
        FiberRule{INHIBIT, LEX, PREP, 0},
        AreaRule{DISINHIBIT, PREP_P, 0},
        FiberRule{INHIBIT, LEX, SUBJ, 1},
        FiberRule{INHIBIT, LEX, OBJ, 1},
        FiberRule{INHIBIT, DET, SUBJ, 1},
        FiberRule{INHIBIT, DET, OBJ, 1},
        FiberRule{INHIBIT, ADJ, SUBJ, 1},
        FiberRule{INHIBIT, ADJ, OBJ, 1},
    };
    return preposition;
}

std::map<std::string, Generic> LEXEME_DICT = {
    {"the", generic_determinant(0)},
	{"a", generic_determinant(1)},
	{"dogs", generic_noun(2)},
	{"cats", generic_noun(3)},
	{"mice", generic_noun(4)},
	{"people", generic_noun(5)},
	{"chase", generic_trans_verb(6)},
	{"love", generic_trans_verb(7)},
	{"bite", generic_trans_verb(8)},
	{"of", generic_preposition(9)},
	{"big", generic_adjective(10)},
	{"bad", generic_adjective(11)},
	{"run", generic_intrans_verb(12)},
	{"fly", generic_intrans_verb(13)},
	{"quickly", generic_adverb(14)},
	{"in", generic_preposition(15)},
	{"are", generic_copula(16)},
	{"man", generic_noun(17)},
	{"woman", generic_noun(18)},
	{"saw", generic_trans_verb(19)},
};

std::map<std::string, std::vector<std::string>> ENGLISH_READOUT_RULES = {
    {VERB, {LEX, SUBJ, OBJ, PREP_P, ADVERB, ADJ}},
	{SUBJ, {LEX, DET, ADJ, PREP_P}},
	{OBJ, {LEX, DET, ADJ, PREP_P}},
	{PREP_P, {LEX, PREP, ADJ, DET}},
	{PREP, {LEX}},
	{ADJ, {LEX}},
	{DET, {LEX}},
	{ADVERB, {LEX}},
	{LEX, {}},
};

enum ReadoutMethod {
    FIXED_MAP_READOUT = 1,
    FIBER_READOUT,
    NATURAL_READOUT
};

class ParserBrain : public nemo::Brain {
public:
    ParserBrain(float p, const std::map<std::string, Generic>& lexeme_dict,
                const std::vector<std::string>& all_areas,
                const std::vector<std::string>& recurrent_areas,
                const std::vector<std::string>& initial_areas,
                const std::map<std::string, std::vector<std::string>>& readout_rules);
    void initialize_states();
    bool applyRule(const std::variant<AreaRule, FiberRule>& rule);
    void applyFiberRule(const FiberRule& rule);
    void applyAreaRule(const AreaRule& rule);
    void parse_project();
    void remember_fibers(const std::map<std::string, std::vector<std::string>>& project_map);
    bool recurrent(const std::string& area) const;
    std::map<std::string, std::vector<std::string>> getProjectMap();
    void activateWord(const std::string& area_name, const std::string& word);
    void activateIndex(const std::string& area_name, int index);
    std::string interpretAssemblyAsString(const std::string& area_name);

private:
    std::map<std::string, Generic> lexeme_dict;
    std::vector<std::string> all_areas;
    std::vector<std::string> recurrent_areas;
    std::vector<std::string> initial_areas;
    std::map<std::string, std::vector<std::string>> readout_rules;
    std::map<std::string, std::map<std::string, std::set<int>>> fiber_states;
    std::map<std::string, std::set<int>> area_states;
    std::map<std::string, std::set<int>> activated_fibers;
    std::map<std::string, std::set<std::string>> area_by_name; 
};

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
                    if (area_by_name[area1].count("winners")) { // Assuming winners is a special key
                        proj_map[area1].push_back(area2);
                    }
                    if (area_by_name[area2].count("winners")) { // Assuming winners is a special key
                        proj_map[area2].push_back(area2);
                    }
                }
            }
        }
    }
    return proj_map;
}




int main() {
    std::string sentence = "cats chase mice";
    std::string language = "English";
    float p = 0.1;
    int LEX_k = 20;
    int project_rounds = 20;
    bool verbose = true;
    bool debug = false;
    ReadoutMethod readout_method = FIBER_READOUT;

    

    return 0;
}