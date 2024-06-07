#ifndef AREA_H
#define AREA_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace std;

class Area {
public:
    // 构造函数
    Area(string name, int order, int n, int k, double beta = 0.05, int w = 0, bool explicit_area = false);

    // 更新胜者
    void updateWinners();

    bool isActive() const;

    // 激活assembly，其对应激活神经元下标连续
    void activate(int start_index);

    // 更新刺激的 beta 值
    void updateBetaByStimulus(const std::string& name, double new_beta);

    // 更新区域的 beta 值
    void updateAreaBeta(const std::string& name, double new_beta);

    void setFire(int i);

    // 将所有下标神经元设置为不活跃
    void setAllNotFire();

    // 获取当前激活神经元的下标信息
    unordered_set<int> getTopKindex() const;

    // 固定组件
    void fixAssembly();

    // 取消固定组件
    void unfixAssembly();

    void update_active_index();

    void Init_SI_t();

    void activate_by_now_active_index();

    // 抑制性群体i抑制了本脑区
    void inhibit(int i);

    // 解除抑制性群体i对本脑区的抑制
    void disinhibit(int i);

    // 返回脑区是否被抑制的信息
    bool is_inhibited() const;

    // 获取激活过的神经元数量
    int getNumEverFired() const;

    std::string name;
    int order;
    int n;
    int k;
    double beta;
    int w;
    int _new_w;
    bool explicit_area;
    int num_first_winners;
    bool fixed_assembly;
    int num_ever_fired;
    std::unordered_map<std::string, double> beta_by_stimulus;
    std::unordered_map<std::string, double> beta_by_area;
    std::vector<int> winners;
    std::vector<int> _new_winners;
    std::vector<std::vector<int>> saved_winners;
    std::vector<int> saved_w;
    std::vector<bool> ever_fired;
    bool is_active;
    bool is_inhibit;
    bool is_lex;
    std::vector<int> assembly;
    std::vector<double> SI_t;
    std::unordered_set<int> support;
    std::unordered_set<int> inhibitors;
    std::unordered_set<int> active_index;

    std::unordered_set<int> findTopKIndices(const std::vector<double>& vec, int k) const;
};

#endif // AREA_H
