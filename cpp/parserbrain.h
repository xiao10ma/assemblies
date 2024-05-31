#ifndef PB_PARSER_BRAIN_H_
#define PB_PARSER_BRAIN_H_

#include "brain.h"
#include "lex.h"
#include <variant>
#include <set>

namespace pb {

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
    virtual std::map<std::string, std::vector<std::string>> getProjectMap();
    void activateWord(const std::string& area_name, const std::string& word);
    void activateIndex(const std::string& area_name, int index);
    std::string interpretAssemblyAsString(const std::string& area_name);
    virtual std::string getWord(const std::string& area_name, double min_overlap = 0.7);
    std::map<std::string, std::set<std::string>> getActivatedFibers();

private:
    std::map<std::string, Generic> lexeme_dict;
    std::vector<std::string> all_areas;
    std::vector<std::string> recurrent_areas;
    std::vector<std::string> initial_areas;
    std::map<std::string, std::vector<std::string>> readout_rules;
    std::map<std::string, std::map<std::string, std::set<int>>> fiber_states;
    std::map<std::string, std::set<int>> area_states;
    std::map<std::string, std::set<std::string>> activated_fibers;
    std::map<std::string, nemo::Area> area_by_name; 
};

}

#endif