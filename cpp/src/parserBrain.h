#ifndef PARSER_BRAIN_H
#define PARSER_BRAIN_H

#include "brain.h"
#include <map>
#include <vector>
#include <string>
#include <variant>
#include <set>
#include <unordered_map>

using namespace std;

// 定义 Generic 结构体
struct Generic {
    int index;
    std::vector<std::variant<AreaRule, FiberRule>> preRules;
    std::vector<std::variant<AreaRule, FiberRule>> postRules;
};

class ParserBrain : public Brain {
public:
    ParserBrain(double p, const map<string, Generic>& lexeme_dict = {},
                const vector<string>& all_areas = {},
                const vector<string>& recurrent_areas = {},
                const vector<string>& initial_areas = {},
                const map<string, vector<string>>& readout_rules = {});

    void initialize_states();
    void applyFiberRule(const FiberRule& rule);
    void applyAreaRule(const AreaRule& rule);
    bool applyRule(const variant<AreaRule, FiberRule>& rule);
    void parse_project();
    void remember_fibers(const std::unordered_map<std::string, std::vector<std::string>>& project_map);
    bool recurrent(const string& area);
    virtual std::unordered_map<std::string, std::vector<std::string>> getProjectMap();
    void activateWord(const string& area_name, const string& word);
    void activateIndex(const string& area_name, int index);
    string interpretAssemblyAsString(const string& area_name);
    virtual string getWord(const string& area_name, double min_overlap = 0.7);
    std::unordered_map<std::string, std::vector<std::string>> getActivatedFibers() const;

    map<string, Generic> lexeme_dict;
    vector<string> all_areas;
    vector<string> recurrent_areas;
    vector<string> initial_areas;
    map<string, vector<string>> readout_rules;
    map<string, map<string, set<int>>> fiber_states;
    map<string, set<int>> area_states;
    std::unordered_map<std::string, std::vector<std::string>> activated_fibers;
};

#endif // PARSER_BRAIN_H
