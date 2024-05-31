#ifndef LEX_H_
#define LEX_H_

#include <string>
#include <vector>
#include <variant>

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

// Brain Areas
extern const std::string LEX;
extern const std::string DET;
extern const std::string SUBJ;
extern const std::string OBJ;
extern const std::string VERB;
extern const std::string PREP;
extern const std::string PREP_P;
extern const std::string ADJ;
extern const std::string ADVERB;

// Fixed area stats for explicit areas
extern const int LEX_SIZE;

// Actions
extern const std::string DISINHIBIT;
extern const std::string INHIBIT;

extern const std::vector<std::string> AREAS;
extern const std::vector<std::string> EXPLICIT_AREAS;
extern const std::vector<std::string> RECURRENT_AREAS;

#endif