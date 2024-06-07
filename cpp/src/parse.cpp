#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <random>
#include <memory>
#include <map>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <functional>
#include <set>
#include <variant>
#include <stack>
#include <iterator>
// #include "parse.h"
#include "area.h"
#include "brain.h"
#include "parserBrain.h"
# define MAX_AREAS_NUM 9
# define P 0.1
# define BETA 0.15
# define LEX_K 60
# define AREA_N 400
# define AREA_K 50
# define WORD_NUM 60
# define DEBUG false
# define THRESHOLD 0.8

using namespace std;

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
const int LEX_SIZE = 60;

// Actions
const std::string DISINHIBIT = "DISINHIBIT";
const std::string INHIBIT = "INHIBIT";
const std::string ACTIVATE_ONLY = "ACTIVATE_ONLY";
const std::string CLEAR_DET = "CLEAR_DET";

const std::vector<std::string> AREAS = {LEX, DET, SUBJ, OBJ, VERB, ADJ, ADVERB, PREP, PREP_P};
const std::vector<std::string> EXPLICIT_AREAS = {LEX};
const std::vector<std::string> RECURRENT_AREAS = {SUBJ, OBJ, VERB, ADJ, ADVERB, PREP, PREP_P};

generic generic_noun(int index) {
    return {
            index,
            {}, // PRE_RULES (AreaRule)
            {
                    // POST_RULES (AreaRule)
                    {INHIBIT, DET, 0}, {INHIBIT, ADJ, 0}, {INHIBIT, PREP_P, 0}, {INHIBIT, PREP, 0}
            },
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, SUBJ, 0}, {DISINHIBIT, LEX, OBJ, 0}, {DISINHIBIT, LEX, PREP_P, 0},
                    {DISINHIBIT, DET, SUBJ, 0}, {DISINHIBIT, DET, OBJ, 0}, {DISINHIBIT, DET, PREP_P, 0},
                    {DISINHIBIT, ADJ, SUBJ, 0}, {DISINHIBIT, ADJ, OBJ, 0}, {DISINHIBIT, ADJ, PREP_P, 0},
                    {DISINHIBIT, VERB, OBJ, 0}, {DISINHIBIT, PREP_P, PREP, 0}, {DISINHIBIT, PREP_P, SUBJ, 0},
                    {DISINHIBIT, PREP_P, OBJ, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, SUBJ, 0}, {INHIBIT, LEX, OBJ, 0}, {INHIBIT, LEX, PREP_P, 0},
                    {INHIBIT, ADJ, SUBJ, 0}, {INHIBIT, ADJ, OBJ, 0}, {INHIBIT, ADJ, PREP_P, 0},
                    {INHIBIT, DET, SUBJ, 0}, {INHIBIT, DET, OBJ, 0}, {INHIBIT, DET, PREP_P, 0},
                    {INHIBIT, VERB, OBJ, 0}, {INHIBIT, PREP_P, PREP, 0}, {INHIBIT, PREP_P, VERB, 0},
                    {DISINHIBIT, LEX, SUBJ, 1}, {DISINHIBIT, LEX, OBJ, 1}, {DISINHIBIT, DET, SUBJ, 1},
                    {DISINHIBIT, DET, OBJ, 1}, {DISINHIBIT, ADJ, SUBJ, 1}, {DISINHIBIT, ADJ, OBJ, 1},
                    {INHIBIT, PREP_P, SUBJ, 0}, {INHIBIT, PREP_P, OBJ, 0}, {INHIBIT, VERB, ADJ, 0}
            }
    };
}

generic generic_trans_verb(int index) {
    return {
            index,
            {
                    // PRE_RULES (AreaRule)
                    {DISINHIBIT, ADVERB, 1}
            },
            {
                    // POST_RULES (AreaRule)
                    {DISINHIBIT, OBJ, 0}, {INHIBIT, SUBJ, 0}, {INHIBIT, ADVERB, 0}
            },
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, VERB, 0}, {DISINHIBIT, VERB, SUBJ, 0}, {DISINHIBIT, VERB, ADVERB, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, VERB, 0}, {DISINHIBIT, PREP_P, VERB, 0}
            }
    };
}

generic generic_intrans_verb(int index) {
    return {
            index,
            {
                    // PRE_RULES (AreaRule)
                    {DISINHIBIT, ADVERB, 1}
            },
            {
                    // POST_RULES (AreaRule)
                    {INHIBIT, SUBJ, 0}, {INHIBIT, ADVERB, 0}
            },
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, VERB, 0}, {DISINHIBIT, VERB, SUBJ, 0}, {DISINHIBIT, VERB, ADVERB, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, VERB, 0}, {DISINHIBIT, PREP_P, VERB, 0}
            }
    };
}

generic generic_copula(int index) {
    return {
            index,
            {}, // PRE_RULES (AreaRule)
            {
                    // POST_RULES (AreaRule)
                    {DISINHIBIT, OBJ, 0}, {INHIBIT, SUBJ, 0}
            },
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, VERB, 0}, {DISINHIBIT, VERB, SUBJ, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, VERB, 0}, {DISINHIBIT, ADJ, VERB, 0}
            }
    };
}

generic generic_adverb(int index) {
    return {
            index,
            {
                    // PRE_RULES (AreaRule)
                    {DISINHIBIT, ADVERB, 0}
            },
            {
                    // POST_RULES (AreaRule)
                    {INHIBIT, ADVERB, 1}
            },
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, ADVERB, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, ADVERB, 0}
            }
    };
}

generic generic_determinant(int index) {
    return {
            index,
            {
                    // PRE_RULES (AreaRule)
                    {DISINHIBIT, DET, 0}
            },
            {}, // POST_RULES (AreaRule)
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, DET, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, DET, 0}, {INHIBIT, VERB, ADJ, 0}
            }
    };
}

generic generic_adjective(int index) {
    return {
            index,
            {
                    // PRE_RULES (AreaRule)
                    {DISINHIBIT, ADJ, 0}
            },
            {}, // POST_RULES (AreaRule)
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, ADJ, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, ADJ, 0}, {INHIBIT, VERB, ADJ, 0}
            }
    };
}

generic generic_preposition(int index) {
    return {
            index,
            {
                    // PRE_RULES (AreaRule)
                    {DISINHIBIT, PREP, 0}
            },
            {
                    // POST_RULES (AreaRule)
                    {DISINHIBIT, PREP_P, 0}
            },
            {
                    // PRE_FIBER_RULES
                    {DISINHIBIT, LEX, PREP, 0}
            },
            {
                    // POST_FIBER_RULES
                    {INHIBIT, LEX, PREP, 0}, {INHIBIT, LEX, SUBJ, 1}, {INHIBIT, LEX, OBJ, 1},
                    {INHIBIT, DET, SUBJ, 1}, {INHIBIT, DET, OBJ, 1}, {INHIBIT, ADJ, SUBJ, 1}, {INHIBIT, ADJ, OBJ, 1}
            }
    };
}


std::unordered_map<std::string, generic> lexeme_dict = {
        {"the", generic_determinant(0)},
        {"a", generic_determinant(1)},
        {"dog", generic_noun(2)},
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
        {"my", generic_determinant(20)},
        {"his", generic_determinant(21)},
        {"this", generic_determinant(22)},
        {"some", generic_determinant(23)},
        {"that", generic_determinant(24)},
        {"bird", generic_noun(25)},
        {"tree", generic_noun(26)},
        {"car", generic_noun(27)},
        {"apple", generic_noun(28)},
        {"book", generic_noun(29)},
        {"see", generic_trans_verb(30)},
        {"make", generic_trans_verb(31)},
        {"take", generic_trans_verb(32)},
        {"want", generic_trans_verb(33)},
        {"find", generic_trans_verb(34)},        
        {"sing", generic_intrans_verb(35)},
        {"sleep", generic_intrans_verb(36)},
        {"jump", generic_intrans_verb(37)},
        {"cry", generic_intrans_verb(38)},
        {"laugh", generic_intrans_verb(39)},        
        {"slowly", generic_adverb(40)},
        {"loudly", generic_adverb(41)},
        {"well", generic_adverb(42)},
        {"often", generic_adverb(43)},
        {"here", generic_adverb(44)},
        {"is", generic_copula(45)},
        {"was", generic_copula(46)},
        {"seems", generic_copula(47)},
        {"feels", generic_copula(48)},
        {"becomes", generic_copula(49)},
        {"small", generic_adjective(50)},
        {"happy", generic_adjective(51)},
        {"bright", generic_adjective(52)},
        {"soft", generic_adjective(53)},
        {"tall", generic_adjective(54)},
        {"on", generic_preposition(55)},
        {"under", generic_preposition(56)},
        {"behind", generic_preposition(57)},
        {"beside", generic_preposition(58)},
        {"between", generic_preposition(59)},        
};

map<string, vector<string>> READOUT_RULE = {
        {VERB,{LEX, SUBJ, OBJ, PREP_P, ADVERB}},
        {SUBJ,{LEX, DET, ADJ, PREP_P}},
        {OBJ, {LEX, DET, ADJ, PREP_P}},
        {PREP_P, {LEX, PREP, ADJ, DET}},
        {PREP, {LEX}},
        {ADJ, {LEX}},
        {DET, {LEX}},
        {ADVERB, {LEX}},
        {LEX,{}},
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

const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> Brain::EMPTY_MAPPING = {};

Generic Generic_noun(int index) {
    Generic noun;
    noun.index = index;
    noun.preRules = {
            FiberRule{DISINHIBIT, LEX, SUBJ, 0},
            FiberRule{DISINHIBIT, LEX, OBJ, 0},
            FiberRule{DISINHIBIT, LEX, PREP_P, 0},
            FiberRule{DISINHIBIT, LEX, DET, 0},
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

Generic Generic_trans_verb(int index) {
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

Generic Generic_intrans_verb(int index) {
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

Generic Generic_copula(int index) {
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

Generic Generic_adverb(int index) {
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

Generic Generic_determinant(int index) {
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

Generic Generic_adjective(int index) {
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

Generic Generic_preposition(int index) {
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

struct dependency{
    string from;
    string to;
    int area_index{};
};
string root;
map<string, vector<string>> tree;

void buildTree(const vector<dependency>& deps) {
    tree.clear();
    for (const auto& dep : deps) {
        if (find(tree[dep.from].begin(), tree[dep.from].end(), dep.to) == tree[dep.from].end()){
            tree[dep.from].push_back(dep.to);
        }
    }
}

void printTree(const string& node, int depth, string& res) {
    res += string(depth * 4, ' '); // Increase indentation
    if (depth > 0) { // Add tree connectors for non-root nodes
        res += "└── ";
    }
    res += node + "\n";
    if (tree.find(node) != tree.end()) {
        for (const auto& child : tree[node]) {
            printTree(child, depth + 1, res);
        }
    }
}

vector<string> splitSentence(const string& sentence) {
    vector<string> words;
    istringstream iss(sentence);
    string word;
    while (iss >> word) {
        words.push_back(word);
    }
    return words;
}

/*函数操作：获取在LEX脑区中，top_k_in_lex下标神经元构成的聚集体所对应的单词*/
string get_word(const unordered_set<int>& top_k_in_lex, int k, double threshold){
    if (top_k_in_lex.empty()){
        return "NULL3";
    }
    vector<int> indices(WORD_NUM, 0);
    for(auto it : top_k_in_lex){
        indices[it / k]++;
    }
    auto maxit = max_element(indices.begin(), indices.end());
    auto maxIndex = distance(indices.begin(), maxit);
    int maxLength = indices[maxIndex];
    if(maxLength < int(k * threshold)){
        return "NULL1";
    }
    for(const auto& temp : lexeme_dict){
        int now_index = temp.second.index;
        if(maxIndex == now_index){
            return temp.first;
        }
    }
    return "NULL2";
}

enum ReadoutMethod {
    FIXED_MAP_READOUT,
    FIBER_READOUT,
    NATURAL_READOUT
};

std::map<std::string, Generic> LEXEME_DICT = {
        {"the", Generic_determinant(0)},
};

class EnglishParserBrain : public ParserBrain {
public:
    EnglishParserBrain(double p, int non_LEX_n = 10000, int non_LEX_k = 100, int LEX_k = 60,
                       double default_beta = 0.2, double LEX_beta = 1.0, double recurrent_beta = 0.05,
                       double interarea_beta = 0.5, bool verbose = false)
            : ParserBrain(p, LEXEME_DICT, AREAS, RECURRENT_AREAS, {LEX, SUBJ, VERB}, ENGLISH_READOUT_RULES), verbose(verbose) {

        int LEX_n = LEX_SIZE * LEX_k;
        addExplicitArea(LEX, LEX_n, LEX_k, default_beta);

        // cout<<("between")<<endl;
        // for (const auto& from_area_pair : connectomes) {
        //     const auto& from_area_name = from_area_pair.first;
        //     const auto& to_areas = from_area_pair.second;
        //     for (const auto& to_area_pair : to_areas) {
        //         const auto& to_area_name = to_area_pair.first;
        //         const auto& connectome = to_area_pair.second;
        //         std::cout << "Connectome from " << from_area_name << " to " << to_area_name << ": size = " << connectome.size() << std::endl;
        //     }
        // }
        // cout<<endl;

        int DET_k = LEX_k;
        addArea(SUBJ, non_LEX_n, non_LEX_k, default_beta);
        addArea(OBJ, non_LEX_n, non_LEX_k, default_beta);
        addArea(VERB, non_LEX_n, non_LEX_k, default_beta);
        addArea(ADJ, non_LEX_n, non_LEX_k, default_beta);
        addArea(PREP, non_LEX_n, non_LEX_k, default_beta);
        addArea(PREP_P, non_LEX_n, non_LEX_k, default_beta);
        addArea(DET, non_LEX_n, DET_k, default_beta);
        addArea(ADVERB, non_LEX_n, non_LEX_k, default_beta);

        // std::cout << "Initialized areas in EnglishParserBrain:" << std::endl;
        // for (const auto& pair : area_by_name) {
        //     std::cout << pair.first << " ";
        // }
        // std::cout << std::endl;

        // std::cout << "Connectomes sizes after initialization:" << std::endl;
        // for (const auto& from_area_pair : connectomes) {
        //     const auto& from_area_name = from_area_pair.first;
        //     const auto& to_areas = from_area_pair.second;
        //     for (const auto& to_area_pair : to_areas) {
        //         const auto& to_area_name = to_area_pair.first;
        //         const auto& connectome = to_area_pair.second;
        //         std::cout << "Connectome from " << from_area_name << " to " << to_area_name << ": size = " << connectome.size() << std::endl;
        //     }
        // }

        std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> custom_plasticities;
        for (const auto& area : RECURRENT_AREAS) {
            custom_plasticities[LEX].emplace_back(area, LEX_beta);
            custom_plasticities[area].emplace_back(LEX, LEX_beta);
            custom_plasticities[area].emplace_back(area, recurrent_beta);
            for (const auto& other_area : RECURRENT_AREAS) {
                if (other_area == area) continue;
                custom_plasticities[area].emplace_back(other_area, interarea_beta);
            }
        }

        updatePlasticities(custom_plasticities, {});


    }

    std::unordered_map<std::string, std::vector<std::string>> getProjectMap() override {
        auto proj_map = ParserBrain::getProjectMap();
        // cout<<"haha"<<endl;
        if (proj_map.find(LEX) != proj_map.end() && proj_map[LEX].size() > 2) {
            std::string areas;
            for (const auto& area : proj_map[LEX]) {
                areas += area + " ";
            }
            throw std::runtime_error("Got that LEX projecting into many areas: " + areas);
        }
        return proj_map;
    }

    std::string getWord(const std::string& area_name, double min_overlap = 0.7) override {
        std::string word = ParserBrain::getWord(area_name, min_overlap);
        if (!word.empty()) {
            return word;
        }
        if (word.empty() && area_name == DET) {
            std::set<int> winners(area_by_name[area_name]->winners.begin(), area_by_name[area_name]->winners.end());
            int area_k = area_by_name[area_name]->k;
            int threshold = min_overlap * area_k;
            int DET_SIZE = 100;
            int nodet_index = DET_SIZE - 1;
            std::set<int> nodet_assembly;
            for (int i = 0; i < area_k; ++i) {
                nodet_assembly.insert(nodet_index * area_k + i);
            }
            std::set<int> intersection;
            std::set_intersection(winners.begin(), winners.end(), nodet_assembly.begin(), nodet_assembly.end(),
                                  std::inserter(intersection, intersection.begin()));
            if (intersection.size() > threshold) {
                return "<null-det>";
            }
        }
        return "<NON-WORD>";
    }

private:
    bool verbose;

    std::string join(const std::set<std::string>& set) {
        std::string result;
        for (const auto& s : set) {
            if (!result.empty()) {
                result += ", ";
            }
            result += s;
        }
        return result;
    }
};

void parser(const string& sentence, Brain& brain){
    //printf("Start Parser: \n");
    // 获取句子中的所有单词，放入words中
    vector<string> words = splitSentence(sentence);
    // 解除0号抑制神经元对LEX, SUBJ, VERB脑区的限制
    brain.areas[brain.Areabyname[LEX]].disinhibit(0);
    brain.areas[brain.Areabyname[SUBJ]].disinhibit(0);
    brain.areas[brain.Areabyname[VERB]].disinhibit(0);
    // 对于句子中的所有单词
    for (const string& word : words){
        //cout << endl << "Parsing Word: " << word << endl;
        // 获取它在LEX脑区中对应的下标
        int index = brain.Areabyname[LEX];
        // 通过LEXEME_DICT获取当前单词对应的规则集
        generic rule = lexeme_dict[word];
        // 激活当前单词在LEX脑区中的聚集体，注意index得到索引，所以需要乘偏移量
        brain.areas[index].activate(rule.index * LEX_K);

        if(DEBUG){
            cout << "activate index in LEX: ";
            for(auto it : brain.areas[brain.Areabyname[LEX]].assembly){
                cout << it << " ";
            }
            cout << endl << endl;

            cout << "\n--Before Parsing Word: " << word << endl << endl;
            // brain.test_all_areas();
        }
        // 根据之前的描述，应用单词的前规则
        brain.applypreRule(rule);

        if(DEBUG){
            cout << "\n--After pre-rule for Word: " << word << endl << endl;
            // brain.test_all_areas();
        }
        // 执行强投影操作
        brain.project_();

        if(DEBUG){
            cout << "\n--After project* for Word: " << word << endl << endl;
            // brain.test_all_areas();
        }
        // 投影完毕，应用单词的后规则
        brain.applypostRule(rule);

        if(DEBUG){
            cout << "\n--After post-rule for Word: " << word << endl << endl;
            // brain.test_all_areas();
        }
    }
    //printf("End Parser: \n\n");
}

vector<dependency> ReadOut(Brain brain){
    /*栈初始含VERB，依赖关系数组初始为空*/
    stack<string> s;
    vector<dependency> D;
    s.push(VERB);
    /*栈不为空时，持续运行*/
    while(!s.empty()){
        /*now_area存储此次解析过程的源脑区信息*/
        string now_area = s.top();
        if(DEBUG){
            cout<< "Now Area: " << now_area << "------------strat" << endl;
        }
        s.pop();

        // empty?
        /*调用try_project函数，投影结果到LEX中，获取LEX的激活下标*/
        unordered_set<int> top_k_in_lex = brain.try_project(now_area, brain.Areabyname[LEX]);
        string project_word1 = get_word(top_k_in_lex, brain.areas[brain.Areabyname[LEX]].k, THRESHOLD);
        if (now_area == VERB) {
            root.clear();
            root = project_word1;
        }
        if(DEBUG){
            cout << endl << "getWord1:" <<endl;
            cout<<project_word1<<endl;
        }

        /*循环其他脑区，尝试找到存在依赖关系的脑区，并解析出具有依赖关系的单词*/
        for(int dst_area_num = 0; dst_area_num < brain.a; dst_area_num++){
            // 跳过自己的脑区号、并且只会遍历Readout规则中规定的对应脑区
            if(dst_area_num == brain.Areabyname[now_area] || find(READOUT_RULE[now_area].begin(),READOUT_RULE[now_area].end(),brain.areas[dst_area_num].name) == READOUT_RULE[now_area].end()){
                continue;
            }

            // 获取脑区A到脑区B的投影
            unordered_set<int> top_k_in_B = brain.try_project(now_area, dst_area_num);
            if(top_k_in_B.empty()){
                continue;
            }
            // 获取脑区B到脑区LEX的投影，并根据投影结果解析对应单词
            unordered_set<int> top_k_in_lex_from_B = brain.try_project2(top_k_in_B, dst_area_num, brain.Areabyname[LEX]);
            string project_word2 = get_word(top_k_in_lex_from_B, brain.areas[brain.Areabyname[LEX]].k, THRESHOLD);

            if(DEBUG){
                cout << endl << "getWord2:" <<endl;
                cout<<"project_word1"<<project_word1<<endl;
                cout<<"project_word2"<<project_word2<<endl;
            }

            // 解析出来的单词为无效信息时，继续下一次循环
            if(project_word2 == "NULL1" || project_word2 == "NULL2"){
                continue;
            }
            // 修改：投到B再投到LEX才getword

            //string project_word2 = get_word(top_k_in_B, brain.k, THRESHOLD);
            //if(project_word2 == "NULL1" || project_word2 == "NULL2"){
            //	continue;
            //}

            // 解析出来的单词有效时，将依赖关系添加到D中
            dependency temp;
            temp.area_index = dst_area_num;
            temp.from = project_word1;
            temp.to = project_word2;
            D.push_back(temp);

            // 获取下一入栈的脑区名称
            string B_name;
            for(const auto& it : brain.Areabyname){
                if(it.second == dst_area_num){
                    B_name = it.first;
                }
            }
            s.push(B_name);
        }
        if(DEBUG){
            cout<< "Now Area: " << now_area << "------------end" << endl << endl;
        }
    }
    return D;
}

void parseHelper(
        ParserBrain& b,
        const std::string& sentence,
        double p,
        int LEX_k,
        int project_rounds,
        bool verbose,
        bool debug,
        const std::unordered_map<std::string, Generic>& lexeme_dict,
        const std::vector<std::string>& all_areas,
        const std::vector<std::string>& explicit_areas,
        int readout_method,
        const std::unordered_map<std::string, std::vector<std::string>>& readout_rules) {

    // Split sentence into words
    std::istringstream iss(sentence);
    std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    bool extreme_debug = false;

    for (const auto& word : words) {
        auto lexeme = lexeme_dict.at(word);
        b.activateWord(LEX, word);

        if (verbose) {
            std::cout << "Activated word: " << word << std::endl;
            std::cout << b.area_by_name.size() << std::endl;
            for (const auto& winner : b.area_by_name[LEX]->winners) {
                std::cout << winner << " ";
            }
            std::cout << std::endl;
        }

        for (const auto& rule : lexeme.preRules) {
            std::visit([&b](auto&& r) { b.applyRule(r); }, rule);
        }

        std::cout << "After applying preRules for word: " << word << std::endl;
        for (const auto& area : all_areas) {
            std::cout << "Area: " << area << " has winners: ";
            for (const auto& winner : b.area_by_name[area]->winners) {
                std::cout << winner << " ";
            }
            std::cout << std::endl;
        }



        auto proj_map = b.getProjectMap();
        if (verbose) {
            std::cout << "Got proj_map = " << std::endl;
            for (const auto& [key, value] : proj_map) {
                std::cout << key << ": ";
                for (const auto& v : value) {
                    std::cout << v << " ";
                }
                std::cout << std::endl;
            }
        }


        for (int i = 0; i < project_rounds; ++i) {
            // cout<<"haha"<<endl;
            b.parse_project();

            if (verbose) {
                proj_map = b.getProjectMap();
                std::cout << "Got proj_map = " << std::endl;
                for (const auto& [key, value] : proj_map) {
                    std::cout << key << ": ";
                    for (const auto& v : value) {
                        std::cout << v << " ";
                    }
                    std::cout << std::endl;
                }
            }
            if (extreme_debug && word == "a") {
                std::cout << "Starting debugger after round " << i << " for word " << word << std::endl;
                // debugger.run();
            }
        }

        for (const auto& rule : lexeme.postRules) {
            std::visit([&b](auto&& r) { b.applyRule(r); }, rule);
        }

        if (debug) {
            std::cout << "Starting debugger after the word " << word << std::endl;
            // debugger.run();
        }
    }

    std::cout << "Before readout:" << std::endl;
    for (const auto& area : all_areas) {
        std::cout << "Area: " << area << " has winners: ";
        for (const auto& winner : b.area_by_name[area]->winners) {
            std::cout << winner << " ";
        }
        std::cout << std::endl;
    }

    // Readout
    b.disable_plasticity = true;
    for (const auto& area : all_areas) {
        b.area_by_name[area]->unfixAssembly();
    }

    std::vector<std::tuple<std::string, std::string, std::string>> dependencies;

    std::function<void(const std::string&, const std::unordered_map<std::string, std::vector<std::string>>&)>
            read_out = [&](const std::string& area, const std::unordered_map<std::string, std::vector<std::string>>& mapping) {
        auto to_areas = mapping.at(area);
        b.project({}, {{area, to_areas}});
        auto this_word = b.getWord(LEX);

        for (const auto& to_area : to_areas) {
            if (to_area == LEX) {
                continue;
            }
            b.project({}, {{to_area, {LEX}}});
            auto other_word = b.getWord(LEX);
            dependencies.emplace_back(this_word, other_word, to_area);
        }

        for (const auto& to_area : to_areas) {
            if (to_area != LEX) {
                read_out(to_area, mapping);
            }
        }
    };

    if (readout_method == 0) { // Assuming ReadoutMethod.FIXED_MAP_READOUT = 0
        read_out(VERB, readout_rules);
        std::unordered_map<std::string, std::vector<std::string>> parsed = {{VERB, {}}};
        for (const auto& dep : dependencies) {
            if (std::get<0>(dep) == "VERB") {
                parsed[VERB].push_back(std::get<1>(dep));
            }
        }
        std::cout << "Final parse dict: " << std::endl;
        for (const auto& [key, value] : parsed) {
            std::cout << key << ": ";
            for (const auto& v : value) {
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }
    }

    if (readout_method == 1) { // Assuming ReadoutMethod.FIBER_READOUT = 1
        auto activated_fibers = b.getActivatedFibers();
        if (verbose) {
            std::cout << "Got activated fibers for readout:" << std::endl;
            for (const auto& [key, value] : activated_fibers) {
                std::cout << key << ": ";
                for (const auto& v : value) {
                    std::cout << v << " ";
                }
                std::cout << std::endl;
            }
        }
        read_out(VERB, activated_fibers);
        std::cout << "Got dependencies: " << std::endl;
        for (const auto& dep : dependencies) {
            std::cout << std::get<0>(dep) << " " << std::get<1>(dep) << " " << std::get<2>(dep) << std::endl;
        }
    }
}

std::unordered_map<std::string, Generic> convertMapToUnorderedMap(const std::map<std::string, Generic>& map) {
    return std::unordered_map<std::string, Generic>(map.begin(), map.end());
}

std::unordered_map<std::string, std::vector<std::string>> convertMapToUnorderedMap(const std::map<std::string, std::vector<std::string>>& map) {
    std::unordered_map<std::string, std::vector<std::string>> unordered_map(map.begin(), map.end());
    return unordered_map;
}


void parse_main(const std::string& sentence) {
    Brain brain(MAX_AREAS_NUM, P, BETA, DEBUG);
    brain.init_parser_area();
    brain.init_fiber_weight();
    parser(sentence, brain);
    brain.fixBrain();
    vector<dependency> result = ReadOut(brain);
    cout<<"Dependency Parsing Result:"<<endl;
    for(const auto& temp : result){
        cout << temp.from << "->" << temp.to << " ("<< brain.areas[temp.area_index].name << ")" << endl;
    }
    cout<<endl;
    buildTree(result);
    string res;
    res.clear();
    cout<<"Parsing Tree:"<<endl;
    printTree(root, 0, res);
    brain.unfixBrain();
    cout<<res;
}