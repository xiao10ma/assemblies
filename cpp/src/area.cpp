#include "area.h"
#include <numeric>

Area::Area(string name, int order, int n, int k, double beta, int w, bool explicit_area)
        : name(std::move(name)), order(order), n(n), k(k), beta(beta), w(w), explicit_area(explicit_area),
          _new_w(0), num_first_winners(-1), fixed_assembly(false), num_ever_fired(0) {
    ever_fired.resize(n, false); // 初始化 ever_fired
    is_active = false;
    is_inhibit = true;
    inhibitors = {0};
    SI_t.resize(n, 0.0);
    is_lex = false;
    if (order == 0) {
        is_lex = true;
    }
}

void Area::updateWinners() {
    winners = _new_winners;
    if (!explicit_area) {
        w = _new_w;
    }
    std::cout << std::endl;
}

bool Area::isActive() const {
    return is_active;
}

void Area::activate(int start_index) {
    active_index.clear();
    assembly.clear();
    for (int i = 0; i < k; i++) {
        assembly.push_back(start_index + i);
        support.insert(start_index + i);
        active_index.insert(start_index + i);
    }
    is_active = true;
}

void Area::updateBetaByStimulus(const std::string& name, double new_beta) {
    beta_by_stimulus[name] = new_beta;
}

void Area::updateAreaBeta(const std::string& name, double new_beta) {
    beta_by_area[name] = new_beta;
}

void Area::setFire(int i) {
    if (is_inhibit == 1) {
        return;
    }
    support.insert(i);
    active_index.insert(i);
}

void Area::setAllNotFire() {
    if (is_inhibit == 1) {
        return;
    }
    active_index.clear();
}

unordered_set<int> Area::getTopKindex() const {
    return active_index;
}

void Area::fixAssembly() {
    if (winners.empty()) {
        throw std::runtime_error("Area " + name + " does not have assembly; cannot fix.");
    }
    fixed_assembly = true;
}

void Area::unfixAssembly() {
    fixed_assembly = false;
}

void Area::update_active_index() {
    if (is_inhibit == 1 || is_lex == 1) {
        return;
    }
    unordered_set<int> new_active_index;
    vector<double> tempVec = SI_t;
    new_active_index = findTopKIndices(tempVec, k);
    active_index = new_active_index;
}

void Area::Init_SI_t() {
    SI_t.resize(n, 0.0);
}

void Area::activate_by_now_active_index() {
    if (is_inhibit == 1 || is_lex == 1) {
        return;
    }
    assembly.clear();
    for (auto it : active_index) {
        assembly.push_back(it);
    }
    is_active = true;
}

void Area::inhibit(int i) {
    inhibitors.insert(i);
    is_inhibit = true;
}

void Area::disinhibit(int i) {
    inhibitors.erase(i);
    if (inhibitors.empty()) {
        is_inhibit = false;
    }
}

bool Area::is_inhibited() const {
    return is_inhibit;
}

int Area::getNumEverFired() const {
    if (explicit_area) {
        return num_ever_fired;
    } else {
        return w;
    }
}

unordered_set<int> Area::findTopKIndices(const std::vector<double>& vec, int k) const {
    vector<int> idx(vec.size());
    iota(idx.begin(), idx.end(), 0);
    partial_sort(idx.begin(), idx.begin() + k, idx.end(),
                 [&vec](int i1, int i2) { return vec[i1] > vec[i2]; });

    return unordered_set<int>(idx.begin(), idx.begin() + k);
}
