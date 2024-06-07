#ifndef BRAIN_H
#define BRAIN_H

#include "area.h"
#include <random>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <set>

using namespace std;

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

struct generic {
    int index;
    vector<AreaRule> preRules;
    vector<AreaRule> postRules;
    vector<FiberRule> preFiberRules;
    vector<FiberRule> postFiberRules;
};

unordered_set<int> findTopKIndices(const vector<double>& arr, int k);

class Brain {
public:
    explicit Brain(int a = 9, double p = 0.1, double brain_beta = 0.15, bool debug = false);

    void init_parser_area();
    void fixBrain();
    void unfixBrain();
    void addArea(const std::string& area_name, int n, int k, double beta);
    void add_Area(const string& area_name, int n, int k);
    void addExplicitArea(const std::string& area_name, int n, int k, double beta,
                         double custom_inner_p = -1, double custom_out_p = -1, double custom_in_p = -1);
    void init_fiber_weight();
    void calculate_SI(int i, int j);
    vector<double> calculate_SI_fix(int i, int j);
    vector<double> calculate_SI_fix2(const unordered_set<int>& active_index_i, int i, int j);
    void updatePlasticity(const std::string& from_area, const std::string& to_area, double new_beta);
    void updatePlasticities(const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>& area_update_map = EMPTY_MAPPING,
                            const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>& stim_update_map = EMPTY_MAPPING);
    void activate(const std::string& area_name, int index);
    void update_active_index(int i);
    void inhibit_fiber(int i, int j, int n_index);
    void disinhibit_fiber(int i, int j, int n_index);
    int projectInto(Area& target_area, const std::vector<std::string>& from_stimuli, const std::vector<std::string>& from_areas, int verbose = 0);
    void updateConnectomes(Area& target_area, const std::vector<std::string>& from_stimuli, const std::vector<std::string>& from_areas,
                           const std::vector<int>& new_winner_indices, const std::vector<float>& prev_winner_inputs, int verbose);
    void project(const std::unordered_map<std::string, std::vector<std::string>>& areas_by_stim,
                 const std::unordered_map<std::string, std::vector<std::string>>& dst_areas_by_src_area,
                 int verbose = 0);
    void project_();
    unordered_set<int> try_project(const string& src_area_name, int dst_index);
    unordered_set<int> try_project2(const unordered_set<int>& src_active_index, int src_index, int dst_index);
    std::vector<float> generateBinomialVector(int size, double p, int n);
    std::vector<float> generateBinomialMatrix(double p, int rows, int cols);
    double binom_ppf(double quantile, int total_k, double p);
    double truncnorm_rvs(double a, double b, double scale, std::mt19937& rng);
    std::vector<int> sampleFromSet(const std::vector<int>& winners, int sample_size);
    void applypostRule(const generic& rule);
    void applypreRule(const generic& rule);

    double p;
    bool save_size;
    bool save_winners;
    bool disable_plasticity;
    std::unordered_map<std::string, std::shared_ptr<Area>> area_by_name;
    std::unordered_map<std::string, int> stimulus_size_by_name;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float>>> connectomes_by_stimulus;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float>>> connectomes;
    std::mt19937 rng;
    bool use_normal_ppf;
    static const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> EMPTY_MAPPING;
    int a; // 脑区限制数量
    int a_now; // 当前脑区数量
    vector<Area> areas; //不同脑区
    map<string, int> Areabyname; //脑区对应下标
    double beta; // 突触权重更新参数beta
    vector<vector<bool>> fiber_active; // 脑区之间的纤维是否有效
    map<pair<int, int>, vector<vector<double>>> fiber_weight; // 纤维连接权重
    map<pair<int, int>, unordered_set<int>> fiber_inhibitor; // 纤维抑制性神经元记录
    bool isfixed; // Readout时，设置为1，保持各脑区不变
    bool debug; // 调试输出标志
};

#endif // BRAIN_H
