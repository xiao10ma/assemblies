#include "brain.h"
#include "parserbrain.h"
#include "lex.h"
#include <iostream>
#include <string>
#include <set>
#include <algorithm>
#include <variant>
#include <sstream>

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


class EnglishParserBrain : public pb::ParserBrain {
public:
    EnglishParserBrain(float p, int non_LEX_n = 10000, int non_LEX_k = 100, int LEX_k = 20,
                       float default_beta = 0.2, float LEX_beta = 1.0, float recurrent_beta = 0.05,
                       float interarea_beta = 0.5, bool verbose = false);
    std::map<std::string, std::vector<std::string>> getProjectMap() override;
    std::string getWord(const std::string& area_name, double min_overlap = 0.7) override;

private:
    bool verbose;
};

EnglishParserBrain::EnglishParserBrain(float p, int non_LEX_n, int non_LEX_k, int LEX_k,
                                       float default_beta, float LEX_beta, float recurrent_beta,
                                       float interarea_beta, bool verbose)
    : ParserBrain(p, LEXEME_DICT, AREAS, RECURRENT_AREAS, {LEX, SUBJ, VERB}, ENGLISH_READOUT_RULES), 
      verbose(verbose) {
    
    int LEX_n = LEX_SIZE * LEX_k;
    AddArea(LEX, LEX_n, LEX_k, false, true);   // explicit

    int DET_k = LEX_k;
    AddArea(SUBJ, non_LEX_n, non_LEX_k, true, false);      // recurrent
    AddArea(OBJ, non_LEX_n, non_LEX_k, true, false);       // recurrent
    AddArea(VERB, non_LEX_n, non_LEX_k, true, false);      // recurrent
    AddArea(ADJ, non_LEX_n, non_LEX_k, true, false);       // recurrent
    AddArea(PREP, non_LEX_n, non_LEX_k, true, false);      // recurrent
    AddArea(PREP_P, non_LEX_n, non_LEX_k, true, false);    // recurrent
    AddArea(DET, non_LEX_n, DET_k, false, false);          // 啥也不是
    AddArea(ADVERB, non_LEX_n, non_LEX_k, true, false);    // recurrent

    // Define custom plasticities
    std::map<std::string, std::vector<std::pair<std::string, float>>> custom_plasticities;
    for (const auto& area : RECURRENT_AREAS) {
        custom_plasticities[LEX].emplace_back(area, LEX_beta);
        custom_plasticities[area].emplace_back(LEX, LEX_beta);
        custom_plasticities[area].emplace_back(area, recurrent_beta);
        for (const auto& other_area : RECURRENT_AREAS) {
            if (other_area == area) {
                continue;
            }
            custom_plasticities[area].emplace_back(other_area, interarea_beta);
        }
    }

    // UpdatePlasticity(custom_plasticities);
}

std::map<std::string, std::vector<std::string>> EnglishParserBrain::getProjectMap() {
    auto proj_map = ParserBrain::getProjectMap();
    // "War of fibers"
    if (proj_map.find(LEX) != proj_map.end() && proj_map[LEX].size() > 2) {  // because LEX->LEX
        std::ostringstream oss;
        oss << "Got that LEX projecting into many areas: ";
        for (const auto& area : proj_map[LEX]) {
            oss << area << " ";
        }
        throw std::runtime_error(oss.str());
    }
    return proj_map;
}


std::string EnglishParserBrain::getWord(const std::string& area_name, double min_overlap) {
    std::string word = ParserBrain::getWord(area_name, min_overlap);
    if (!word.empty()) {
        return word;
    }

    // 可以忽略 py 代码这里是有错的
    // if (area_name == "DET") {
    //     const auto& area = area_by_name[area_name];
    //     std::set<int> winners(area.winners.begin(), area.winners.end());
    //     int area_k = area.k;
    //     int threshold = static_cast<int>(min_overlap * area_k);
    //     int nodet_index = DET_SIZE - 1;
    //     std::set<int> nodet_assembly;
    //     for (int i = 0; i < area_k; ++i) {
    //         nodet_assembly.insert(nodet_index * area_k + i);
    //     }
    //     std::set<int> intersection;
    //     std::set_intersection(winners.begin(), winners.end(), nodet_assembly.begin(), nodet_assembly.end(), std::inserter(intersection, intersection.begin()));
    //     if (static_cast<int>(intersection.size()) > threshold) {
    //         return "<null-det>";
    //     }
    // }

    // If nothing matched, at least we can see that in the parse output.
    return "<NON-WORD>";
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