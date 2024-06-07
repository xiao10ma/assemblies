#include "brain.h"
#include <numeric> // Include for std::iota
#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>

unordered_set<int> findTopKIndices(const vector<double>& arr, int k) {
    vector<pair<double, int>> valueIndexPairs;
    for (long long unsigned int i = 0; i < arr.size(); ++i) {
        valueIndexPairs.emplace_back(arr[i], i);
    }
    sort(valueIndexPairs.begin(), valueIndexPairs.end(),  greater<>());
    unordered_set<int> topKIndices;
    for (int i = 0; i < k; ++i) {
        topKIndices.insert(valueIndexPairs[i].second);
    }
    return topKIndices;
}

Brain::Brain(int a, double p, double brain_beta, bool debug)
    : a(a), p(p), beta(brain_beta), debug(debug), rng(std::random_device{}()) {
    isfixed = false;
    a_now = 0;

    fiber_active.resize(a, vector<bool>(a, false));
    for (int i = 0; i < a; i++) {
        fiber_active[i][i] = true;
    }
}

void Brain::init_parser_area() {
    add_Area("LEX", 100, 10); // Assuming LEX_K * WORD_NUM = 100, LEX_K = 10
    add_Area("DET", 100, 10); // Assuming AREA_N = 100, AREA_K = 10
    add_Area("SUBJ", 100, 10);
    add_Area("OBJ", 100, 10);
    add_Area("VERB", 100, 10);
    add_Area("PREP", 100, 10);
    add_Area("PREP_P", 100, 10);
    add_Area("ADJ", 100, 10);
    add_Area("ADVERB", 100, 10);
}

void Brain::fixBrain() {
    isfixed = true;
}

void Brain::unfixBrain() {
    isfixed = false;
}

void Brain::addArea(const std::string& area_name, int n, int k, double beta) {
    auto the_area = std::make_shared<Area>(area_name, n, k, beta);
    area_by_name[area_name] = the_area;

    std::unordered_map<std::string, std::vector<float>> new_connectomes;
    for (const auto& other_area_pair : area_by_name) {
        const auto& other_area_name = other_area_pair.first;
        const auto& other_area = other_area_pair.second;
        int other_area_size = other_area->explicit_area ? other_area->n : 0;
        new_connectomes[other_area_name] = std::vector<float>(other_area_size, 0.0f);

        if (other_area_name != area_name) {
            connectomes[other_area_name][area_name] = std::vector<float>(other_area_size, 0.0f);
        }
        other_area->beta_by_area[area_name] = other_area->beta;
        the_area->beta_by_area[other_area_name] = beta;
    }
    connectomes[area_name] = new_connectomes;
}

void Brain::add_Area(const string& area_name, int n, int k) {
    Area new_area(area_name, a_now, n, k);
    areas.push_back(new_area);
    Areabyname[area_name] = a_now;
    a_now++;
}

void Brain::addExplicitArea(const std::string& area_name, int n, int k, double beta,
                            double custom_inner_p, double custom_out_p, double custom_in_p) {
    auto the_area = std::make_shared<Area>(area_name, n, k, beta, n, true);
    area_by_name[area_name] = the_area;
    the_area->ever_fired.resize(n, false);
    the_area->num_ever_fired = 0;

    for (auto& stim_pair : connectomes_by_stimulus) {
        const auto& stim_name = stim_pair.first;
        stim_pair.second[area_name] = generateBinomialVector(stimulus_size_by_name[stim_name], p, n);
        the_area->beta_by_stimulus[stim_name] = beta;
    }

    double inner_p = (custom_inner_p != -1) ? custom_inner_p : p;
    double in_p = (custom_in_p != -1) ? custom_in_p : p;
    double out_p = (custom_out_p != -1) ? custom_out_p : p;

    std::unordered_map<std::string, std::vector<float>> new_connectomes;
    for (const auto& other_area_pair : area_by_name) {
        const auto& other_area_name = other_area_pair.first;
        const auto& other_area = other_area_pair.second;

        if (other_area_name == area_name) {
            new_connectomes[other_area_name] = generateBinomialMatrix(inner_p, n, n);
        } else {
            if (other_area->explicit_area) {
                int other_n = other_area->n;
                new_connectomes[other_area_name] = generateBinomialMatrix(out_p, n, other_n);
                connectomes[other_area_name][area_name] = generateBinomialMatrix(in_p, other_n, n);
            } else {
                new_connectomes[other_area_name].resize(n * 0, 0.0f);
                connectomes[other_area_name][area_name].resize(0 * n, 0.0f);
            }
        }
        other_area->beta_by_area[area_name] = other_area->beta;
        the_area->beta_by_area[other_area_name] = beta;
    }
    connectomes[area_name] = new_connectomes;
}

void Brain::init_fiber_weight() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < a; i++) {
        for (int j = 0; j < a; j++) {
            vector<vector<double>> temp;
            temp.resize(areas[i].n, vector<double>(areas[j].n, 0.0));
            for (int i_ = 0; i_ < areas[i].n; i_++) {
                for (int j_ = 0; j_ < areas[j].n; j_++) {
                    if (dis(gen) < p) {
                        temp[i_][j_] = 1.0;
                    }
                    if (i > j) {
                        temp[i_][j_] = fiber_weight[{j, i}][j_][i_];
                    }
                }
            }
            fiber_weight[{i, j}] = temp;
            fiber_inhibitor[{i, j}] = unordered_set<int>({0});
            if (i == j) {
                fiber_inhibitor[{i, j}] = unordered_set<int>();
            }
        }
    }
}

void Brain::calculate_SI(int i, int j) {
    if (!isfixed) {
        unordered_set<int> active_index_i = areas[i].getTopKindex();
        for (int j_ = 0; j_ < areas[j].n; j_++) {
            double SI_j = 0;
            for (int i_ : active_index_i) {
                SI_j += fiber_weight[{i, j}][i_][j_];
            }
            areas[j].SI_t[j_] += SI_j;
        }
    }
}

vector<double> Brain::calculate_SI_fix(int i, int j) {
    unordered_set<int> active_index_i = areas[i].getTopKindex();
    if (active_index_i.empty()) {
        return {};
    }

    vector<double> result;
    for (int j_ = 0; j_ < areas[j].n; j_++) {
        double SI_j = 0;
        for (int i_ : active_index_i) {
            SI_j += fiber_weight[{i, j}][i_][j_];
        }
        result.push_back(SI_j);
    }

    return result;
}

vector<double> Brain::calculate_SI_fix2(const unordered_set<int>& active_index_i, int i, int j) {
    vector<double> result;
    for (int j_ = 0; j_ < areas[j].n; j_++) {
        double SI_j = 0;
        for (int i_ : active_index_i) {
            SI_j += fiber_weight[{i, j}][i_][j_];
        }
        result.push_back(SI_j);
    }
    return result;
}

void Brain::updatePlasticity(const std::string& from_area, const std::string& to_area, double new_beta) {
    if (area_by_name.find(to_area) != area_by_name.end()) {
        area_by_name[to_area]->beta_by_area[from_area] = new_beta;
    } else {
        std::cerr << "Error: Area " << to_area << " not found." << std::endl;
    }
}

void Brain::updatePlasticities(const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>& area_update_map,
                               const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>& stim_update_map) {
    for (const auto& to_area_pair : area_update_map) {
        const auto& to_area = to_area_pair.first;
        const auto& update_rules = to_area_pair.second;

        for (const auto& rule : update_rules) {
            const auto& from_area = rule.first;
            double new_beta = rule.second;

            if (area_by_name.find(to_area) != area_by_name.end()) {
                area_by_name[to_area]->beta_by_area[from_area] = new_beta;
            }
        }
    }

    for (const auto& area_pair : stim_update_map) {
        const auto& area = area_pair.first;
        const auto& update_rules = area_pair.second;

        if (area_by_name.find(area) != area_by_name.end()) {
            auto& the_area = area_by_name[area];
            for (const auto& rule : update_rules) {
                const auto& stim = rule.first;
                double new_beta = rule.second;
                the_area->beta_by_stimulus[stim] = new_beta;
            }
        }
    }
}

void Brain::activate(const std::string& area_name, int index) {
    if (area_by_name.find(area_name) != area_by_name.end()) {
        auto& area = area_by_name[area_name];
        int k = area->k;
        int assembly_start = k * index;
        if (assembly_start + k > area->n) {
            throw std::runtime_error("Activation index out of range for area: " + area_name);
        }
        area->winners.clear();
        for (int i = 0; i < k; ++i) {
            area->winners.push_back(assembly_start + i);
        }
        area->w = k;
        area->fixAssembly();
    } else {
        std::cerr << "Error: Area " << area_name << " not found." << std::endl;
    }
}

void Brain::update_active_index(int i) {
    if (!isfixed) {
        areas[i].update_active_index();
    }
}

void Brain::inhibit_fiber(int i, int j, int n_index) {
    if (!isfixed) {
        fiber_inhibitor[{i, j}].insert(n_index);
        fiber_active[i][j] = false;
        fiber_inhibitor[{j, i}].insert(n_index);
        fiber_active[j][i] = false;
    }
}

void Brain::disinhibit_fiber(int i, int j, int n_index) {
    if (!isfixed) {
        fiber_inhibitor[{i, j}].erase(n_index);
        fiber_inhibitor[{j, i}].erase(n_index);
        if (fiber_inhibitor[{i, j}].empty()) {
            fiber_active[i][j] = true;
            fiber_active[j][i] = true;
        }
    }
}

int Brain::projectInto(Area& target_area, const std::vector<std::string>& from_stimuli, const std::vector<std::string>& from_areas, int verbose) {
    auto rng = std::mt19937{ std::random_device{}() };
    std::string target_area_name = target_area.name;

    int num_first_winners_processed;
    std::vector<std::vector<int>> inputs_by_first_winner_index(num_first_winners_processed);

    if (target_area.fixed_assembly) {
        target_area._new_winners = target_area.winners;
        target_area._new_w = target_area.w;
        std::vector<float> first_winner_inputs;
        num_first_winners_processed = 0;
    } else {
        std::vector<float> prev_winner_inputs(target_area.w, 0.0f);

        for (const auto& from_area_name : from_areas) {
            auto& connectome = connectomes[from_area_name][target_area_name];
            int num_rows = area_by_name[from_area_name]->winners.size();
            int num_cols = connectome.size() / num_rows;

            for (const auto& w : area_by_name[from_area_name]->winners) {
                if ((w * num_cols + num_cols > connectome.size())) {
                    continue;
                }

                std::vector<float> connectome_row(connectome.begin() + w * num_cols, connectome.begin() + (w + 1) * num_cols);

                for (size_t i = 0; i < num_cols; ++i) {
                    prev_winner_inputs[i] += connectome_row[i];
                }
            }

            if (verbose >= 2) {
                std::cout << "After adding inputs from area " << from_area_name << ": ";
                for (const auto& input : prev_winner_inputs) {
                    std::cout << input << " ";
                }
                std::cout << std::endl;
            }
        }

        if (verbose >= 2) {
            std::cout << "prev_winner_inputs: ";
            for (const auto& val : prev_winner_inputs) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }

        std::vector<float> all_potential_winner_inputs = prev_winner_inputs;
        int total_k = 0;
        int num_inputs = 0;
        std::vector<int> input_size_by_from_area_index;
        if (!target_area.explicit_area) {
            for (const auto& stim : from_stimuli) {
                int local_k = stimulus_size_by_name[stim];
                input_size_by_from_area_index.push_back(local_k);
                num_inputs += 1;
                total_k += local_k;
            }
            for (const auto& from_area_name : from_areas) {
                int effective_k = area_by_name[from_area_name]->winners.size();
                input_size_by_from_area_index.push_back(effective_k);
                num_inputs += 1;
                total_k += effective_k;
            }

            int effective_n = target_area.n - target_area.w;
            if (effective_n <= target_area.k) {
                throw std::runtime_error("Remaining size of area \"" + target_area_name + "\" too small to sample k new winners.");
            }

            double quantile = static_cast<double>(effective_n - target_area.k) / effective_n;
            double alpha = binom_ppf(quantile, total_k, p);
            double mu = total_k * p;
            double std = std::sqrt(total_k * p * (1.0 - p));
            double a = (alpha - mu) / std;
            double b = (total_k - mu) / std;
            std::vector<float> potential_new_winner_inputs(target_area.k);
            for (int i = 0; i < target_area.k; ++i) {
                potential_new_winner_inputs[i] = std::round(mu + truncnorm_rvs(a, b, std, rng));
            }

            all_potential_winner_inputs.insert(all_potential_winner_inputs.end(), potential_new_winner_inputs.begin(), potential_new_winner_inputs.end());
        } else {
            all_potential_winner_inputs = prev_winner_inputs;
        }

        target_area._new_winners.clear();
        std::vector<int> new_winner_indices(target_area.k);
        std::partial_sort_copy(all_potential_winner_inputs.begin(), all_potential_winner_inputs.end(), new_winner_indices.begin(), new_winner_indices.end(), std::greater<float>());

        if (target_area.explicit_area) {
            for (const auto& winner : new_winner_indices) {
                if (!target_area.ever_fired[winner]) {
                    target_area.ever_fired[winner] = true;
                    ++target_area.num_ever_fired;
                }
            }
        }

        std::vector<float> first_winner_inputs;
        num_first_winners_processed = 0;
        if (!target_area.explicit_area) {
            for (int i = 0; i < target_area.k; ++i) {
                if (new_winner_indices[i] >= target_area.w) {
                    first_winner_inputs.push_back(all_potential_winner_inputs[new_winner_indices[i]]);
                    new_winner_indices[i] = target_area.w + num_first_winners_processed;
                    ++num_first_winners_processed;
                }
            }
        }

        target_area._new_winners = new_winner_indices;
        target_area._new_w = target_area.w + num_first_winners_processed;

        if (verbose >= 2) {
            std::cout << "new_winners: ";
            for (const auto& winner : target_area._new_winners) {
                std::cout << winner << " ";
            }
            std::cout << std::endl;
        }

        inputs_by_first_winner_index.resize(num_first_winners_processed);
        for (int i = 0; i < num_first_winners_processed; ++i) {
            std::vector<int> input_indices(total_k);
            std::iota(input_indices.begin(), input_indices.end(), 0);
            std::shuffle(input_indices.begin(), input_indices.end(), rng);
            input_indices.resize(static_cast<size_t>(first_winner_inputs[i]));

            std::vector<int> num_connections_by_input_index(num_inputs, 0);
            int total_so_far = 0;
            for (int j = 0; j < num_inputs; ++j) {
                for (const auto& w : input_indices) {
                    if (total_so_far + input_size_by_from_area_index[j] > w && w >= total_so_far) {
                        ++num_connections_by_input_index[j];
                    }
                }
                total_so_far += input_size_by_from_area_index[j];
            }
            inputs_by_first_winner_index[i] = num_connections_by_input_index;

            if (verbose >= 2) {
                std::cout << "For first_winner # " << i << " with input " << first_winner_inputs[i] << " split as so: ";
                for (const auto& val : num_connections_by_input_index) {
                    std::cout << val << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    int num_inputs_processed = 0;

    for (const auto& stim : from_stimuli) {
        auto& connectomes = connectomes_by_stimulus[stim];
        auto& target_connectome = connectomes[target_area_name];
        if (target_connectome.empty()) {
            target_connectome.resize(target_area._new_w, 0.0f);
        } else if (num_first_winners_processed > 0) {
            target_connectome.resize(target_area._new_w);
        }

        auto first_winner_synapses = std::vector<float>(target_connectome.begin() + target_area.w, target_connectome.end());
        for (int i = 0; i < num_first_winners_processed; ++i) {
            first_winner_synapses[i] = inputs_by_first_winner_index[i][num_inputs_processed];
        }

        float stim_to_area_beta = target_area.beta_by_stimulus[stim];
        if (disable_plasticity) {
            stim_to_area_beta = 0.0f;
        }
        for (const auto& i : target_area._new_winners) {
            target_connectome[i] *= 1.0f + stim_to_area_beta;
        }

        if (verbose >= 2) {
            std::cout << stim << " now looks like: ";
            for (const auto& val : target_connectome) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
        ++num_inputs_processed;
    }

    if (!target_area.explicit_area && num_first_winners_processed > 0) {
        for (auto& [stim_name, connectomes] : connectomes_by_stimulus) {
            if (std::find(from_stimuli.begin(), from_stimuli.end(), stim_name) != from_stimuli.end()) {
                continue;
            }
            auto& the_connectome = connectomes[target_area_name];
            the_connectome.resize(target_area._new_w);
            std::generate(the_connectome.begin() + target_area.w, the_connectome.end(), [this, stim_name] {
                return std::binomial_distribution<>(this->stimulus_size_by_name[stim_name], this->p)(this->rng);
            });
        }
    }

    for (const auto& from_area_name : from_areas) {
        int from_area_w = area_by_name[from_area_name]->w;
        auto& from_area_winners = area_by_name[from_area_name]->winners;
        std::set<int> from_area_winners_set(from_area_winners.begin(), from_area_winners.end());
        auto& from_area_connectomes = connectomes[from_area_name];

        if (from_area_connectomes[target_area_name].empty()) {
            from_area_connectomes[target_area_name].resize(from_area_w * target_area._new_w, 0.0f);
        } else {
            from_area_connectomes[target_area_name].resize(from_area_w * target_area._new_w, 0.0f);
        }
        auto& the_connectome = from_area_connectomes[target_area_name];

        for (int i = 0; i < num_first_winners_processed; ++i) {
            int total_in = inputs_by_first_winner_index[i][num_inputs_processed];
            auto sample_indices = std::vector<int>(total_in);
            std::sample(from_area_winners.begin(), from_area_winners.end(), sample_indices.begin(), total_in, rng);

            for (const auto& j : sample_indices) {
                the_connectome[j * target_area._new_w + target_area.w + i] = 1.0f;
            }

            for (int j = 0; j < from_area_w; ++j) {
                if (from_area_winners_set.find(j) == from_area_winners_set.end()) {
                    the_connectome[j * target_area._new_w + target_area.w + i] = std::binomial_distribution<>(1, p)(rng);
                }
            }
        }

        float area_to_area_beta = disable_plasticity ? 0.0f : target_area.beta_by_area[from_area_name];
        for (const auto& i : target_area._new_winners) {
            for (const auto& j : from_area_winners) {
                the_connectome[j * target_area._new_w + i] *= 1.0f + area_to_area_beta;
            }
        }

        if (verbose >= 2) {
            std::cout << "Connectome of " << from_area_name << " to " << target_area_name << " is now: ";
            for (const auto& val : the_connectome) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
        ++num_inputs_processed;
    }

    for (auto& [other_area_name, other_area] : area_by_name) {
        if (std::find(from_areas.begin(), from_areas.end(), other_area_name) == from_areas.end()) {
            auto& the_other_area_connectome = connectomes[other_area_name][target_area_name];
            the_other_area_connectome.resize(other_area->w * target_area._new_w);
            std::generate(the_other_area_connectome.begin() + other_area->w * target_area.w, the_other_area_connectome.end(), [this, &rng] {
                return std::binomial_distribution<>(1, p)(rng);
            });
        }

        auto& the_target_area_connectome = connectomes[target_area_name][other_area_name];
        the_target_area_connectome.resize(target_area._new_w * other_area->w);
        std::generate(the_target_area_connectome.begin() + target_area.w * other_area->w, the_target_area_connectome.end(), [this, &rng] {
            return std::binomial_distribution<>(1, p)(rng);
        });

        if (verbose >= 2) {
            std::cout << "Connectome of " << target_area_name << " to " << other_area_name << " is now: ";
            for (const auto& val : the_target_area_connectome) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }

    return num_first_winners_processed;
}

void Brain::updateConnectomes(Area& target_area, const std::vector<std::string>& from_stimuli, const std::vector<std::string>& from_areas,
                              const std::vector<int>& new_winner_indices, const std::vector<float>& prev_winner_inputs, int verbose) {
    int num_first_winners_processed = new_winner_indices.size();
    int num_inputs_processed = 0;

    for (const auto& stim : from_stimuli) {
        auto& connectomes = connectomes_by_stimulus[stim];
        if (num_first_winners_processed > 0) {
            connectomes[target_area.name].resize(target_area._new_w);
        }

        auto& target_connectome = connectomes[target_area.name];
        for (int i = target_area.w; i < target_area._new_w; ++i) {
            target_connectome[i] = 1.0 + target_area.beta_by_stimulus[stim];
        }

        for (int i : target_area._new_winners) {
            target_connectome[i] *= 1 + target_area.beta_by_stimulus[stim];
        }

        if (verbose >= 2) {
            std::cout << stim << " now looks like: ";
            for (const auto& val : connectomes[target_area.name]) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }

        num_inputs_processed++;
    }

    if (!target_area.explicit_area && num_first_winners_processed > 0) {
        for (auto& [stim_name, connectomes] : connectomes_by_stimulus) {
            if (std::find(from_stimuli.begin(), from_stimuli.end(), stim_name) != from_stimuli.end()) continue;
            connectomes[target_area.name].resize(target_area._new_w);
            std::generate(connectomes[target_area.name].begin() + target_area.w, connectomes[target_area.name].end(), [this] {
                return std::binomial_distribution<>(1, p)(rng);
            });
        }
    }

    for (const auto& from_area_name : from_areas) {
        auto& from_area = area_by_name[from_area_name];
        auto& from_area_connectomes = connectomes[from_area_name];
        from_area_connectomes[target_area.name].resize(from_area->w * target_area._new_w);

        for (int i = target_area.w; i < target_area._new_w; ++i) {
            auto sample_indices = sampleFromSet(from_area->winners, prev_winner_inputs[i]);
            for (int idx : sample_indices) {
                from_area_connectomes[target_area.name][idx * target_area._new_w + i] = 1.0;
            }
        }

        for (int i : target_area._new_winners) {
            for (int j : from_area->winners) {
                from_area_connectomes[target_area.name][j * target_area._new_w + i] *= 1.0 + target_area.beta_by_area[from_area_name];
            }
        }

        if (verbose >= 2) {
            std::cout << "Connectome of " << from_area_name << " to " << target_area.name << " is now: ";
            for (const auto& val : from_area_connectomes[target_area.name]) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }

        num_inputs_processed++;
    }

    for (auto& [other_area_name, other_area] : area_by_name) {
        if (std::find(from_areas.begin(), from_areas.end(), other_area_name) == from_areas.end()) {
            auto& the_other_area_connectome = connectomes[other_area_name][target_area.name];
            the_other_area_connectome.resize(other_area->w * target_area._new_w);
            std::generate(the_other_area_connectome.begin() + other_area->w * target_area.w, the_other_area_connectome.end(), [this] {
                return std::binomial_distribution<>(1, p)(rng);
            });
        }

        auto& target_area_connectome = connectomes[target_area.name][other_area_name];
        target_area_connectome.resize(target_area._new_w * other_area->w);
        std::generate(target_area_connectome.begin() + target_area.w * other_area->w, target_area_connectome.end(), [this, other_area_name, target_area_name=target_area.name] {
            return std::binomial_distribution<>(1, p)(rng);
        });

        if (verbose >= 2) {
            std::cout << "Connectome of " << target_area.name << " to " << other_area_name << " is now: ";
            for (const auto& val : target_area_connectome) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
}

void Brain::project(const std::unordered_map<std::string, std::vector<std::string>>& areas_by_stim,
                    const std::unordered_map<std::string, std::vector<std::string>>& dst_areas_by_src_area,
                    int verbose) {
    std::unordered_map<std::string, std::vector<std::string>> stim_in;
    std::unordered_map<std::string, std::vector<std::string>> area_in;

    for (const auto& [from_area_name, to_area_names] : dst_areas_by_src_area) {
        if (area_by_name.find(from_area_name) == area_by_name.end()) {
            throw std::runtime_error(from_area_name + " not in brain.area_by_name");
        }
        for (const auto& to_area_name : to_area_names) {
            if (area_by_name.find(to_area_name) == area_by_name.end()) {
                throw std::runtime_error("Not in brain.area_by_name: " + to_area_name);
            }
            area_in[to_area_name].push_back(from_area_name);
        }
    }

    std::unordered_set<std::string> to_update_area_names;

    for (const auto& [key, _] : stim_in) {
        to_update_area_names.insert(key);
    }
    for (const auto& [key, _] : area_in) {
        to_update_area_names.insert(key);
    }

    for (const auto& area_name : to_update_area_names) {
        auto& area = *area_by_name[area_name];
        int num_first_winners = projectInto(area, stim_in[area_name], area_in[area_name], verbose);
        area.num_first_winners = num_first_winners;
        if (save_winners) {
            area.saved_winners.push_back(area._new_winners);
        }
    }

    for (const auto& area_name : to_update_area_names) {
        auto& area = *area_by_name[area_name];
        area.updateWinners();
        if (save_size) {
            area.saved_w.push_back(area.w);
        }
    }
}

void Brain::project_() {
    if (!isfixed) {
        for (int i = 0; i < a; i++) {
            if (areas[i].isActive()) {
                areas[i].setAllNotFire();
                for (auto it : areas[i].assembly) {
                    areas[i].setFire(it);
                }
            }
        }

        for (int round = 1; round < 20; round++) {
            for (int i = 0; i < a; i++) {
                areas[i].Init_SI_t();
            }

            for (int i = 0; i < a; i++) {
                if (areas[i].is_inhibited()) {
                    continue;
                }
                for (int j = 0; j < a; j++) {
                    if (areas[j].is_inhibited()) {
                        continue;
                    }
                    if (i == j) {
                        continue;
                    }
                    if (!fiber_active[i][j]) {
                        continue;
                    }
                    calculate_SI(i, j);
                    update_active_index(j);
                }
            }

            vector<unordered_set<int>> active_t_cache(a);

            for (int i = 0; i < a; i++) {
                if (areas[i].is_inhibited()) {
                    continue;
                }
                active_t_cache[i] = areas[i].getTopKindex();
            }

            for (int i = 0; i < a; i++) {
                if (areas[i].is_inhibited()) {
                    continue;
                }

                for (int j = 0; j < a; j++) {
                    if (areas[j].is_inhibited()) {
                        continue;
                    }
                    if (i == j) {
                        continue;
                    }

                    if (!fiber_active[i][j]) {
                        continue;
                    }
                    unordered_set<int> active_index_i = active_t_cache[i];
                    unordered_set<int> active_index_j = areas[j].getTopKindex();
                    for (int i_ : active_index_i) {
                        for (int j_ : active_index_j) {
                            fiber_weight[{i, j}][i_][j_] *= (1 + beta);
                            fiber_weight[{j, i}][j_][i_] *= (1 + beta);
                        }
                    }
                }

                areas[i].activate_by_now_active_index();
            }
        }
    }
}

unordered_set<int> Brain::try_project(const string& src_area_name, int dst_index) {
    int src_index = Areabyname[src_area_name];

    if (isfixed) {
        vector<double> SI_dst = calculate_SI_fix(src_index, dst_index);
        if (SI_dst.empty()) {
            return {};
        }

        unordered_set<int> new_active_index;
        const vector<double>& tempVec = SI_dst;
        new_active_index = findTopKIndices(tempVec, areas[dst_index].k);
        if (debug) {
            cout << "try_project top k index from [" << src_area_name << "] to " << "Area" << dst_index << ":";
            for (int index : new_active_index) {
                cout << index << " ";
            }

            cout << endl;
        }
        return new_active_index;
    } else {
        calculate_SI(src_index, dst_index);
        update_active_index(dst_index);
        return {};
    }
}

unordered_set<int> Brain::try_project2(const unordered_set<int>& src_active_index, int src_index, int dst_index) {
    vector<double> SI_dst = calculate_SI_fix2(src_active_index, src_index, dst_index);

    unordered_set<int> new_active_index;
    const vector<double>& tempVec = SI_dst;
    new_active_index = findTopKIndices(tempVec, areas[dst_index].k);

    if (debug) {
        cout << "try_project2 top k index from Area " << src_index << " to " << "Area" << dst_index << ":";
        for (int index : new_active_index) {
            cout << index << " ";
        }

        cout << endl;
    }
    return new_active_index;
}

std::vector<float> Brain::generateBinomialVector(int size, double p, int n) {
    std::vector<float> result(n);
    std::binomial_distribution<int> binom_dist(size, p);
    for (auto& val : result) {
        val = static_cast<float>(binom_dist(rng));
    }
    return result;
}

std::vector<float> Brain::generateBinomialMatrix(double p, int rows, int cols) {
    std::vector<float> matrix(rows * cols);
    std::binomial_distribution<int> binom_dist(1, p);
    for (auto& val : matrix) {
        val = static_cast<float>(binom_dist(rng));
    }
    return matrix;
}

double Brain::binom_ppf(double quantile, int total_k, double p) {
    std::binomial_distribution<int> binom_dist(total_k, p);
    std::vector<int> results;
    for (int i = 0; i < 10000; ++i) {
        results.push_back(binom_dist(rng));
    }
    std::sort(results.begin(), results.end());
    return results[static_cast<int>(quantile * results.size())];
}

double Brain::truncnorm_rvs(double a, double b, double scale, std::mt19937& rng) {
    std::normal_distribution<double> norm_dist(0.0, scale);
    double result;
    do {
        result = norm_dist(rng);
    } while (result < a || result > b);
    return result;
}

std::vector<int> Brain::sampleFromSet(const std::vector<int>& winners, int sample_size) {
    std::vector<int> sampled_indices(sample_size);
    std::sample(winners.begin(), winners.end(), sampled_indices.begin(), sample_size, rng);
    return sampled_indices;
}

void Brain::applypostRule(const generic& rule) {
    for (const AreaRule& rule1 : rule.postRules) {
        if (rule1.action == "INHIBIT") {
            areas[Areabyname[rule1.area]].inhibit(rule1.index);
        } else {
            areas[Areabyname[rule1.area]].disinhibit(rule1.index);
        }
    }
    for (const FiberRule& rule2 : rule.postFiberRules) {
        if (rule2.action == "INHIBIT") {
            inhibit_fiber(Areabyname[rule2.area1], Areabyname[rule2.area2], rule2.index);
        } else {
            disinhibit_fiber(Areabyname[rule2.area1], Areabyname[rule2.area2], rule2.index);
        }
    }
}

void Brain::applypreRule(const generic& rule) {
    for (const AreaRule& rule1 : rule.preRules) {
        if (rule1.action == "INHIBIT") {
            areas[Areabyname[rule1.area]].inhibit(rule1.index);
        } else {
            areas[Areabyname[rule1.area]].disinhibit(rule1.index);
        }
    }
    for (const FiberRule& rule2 : rule.preFiberRules) {
        if (rule2.action == "INHIBIT") {
            inhibit_fiber(Areabyname[rule2.area1], Areabyname[rule2.area2], rule2.index);
        } else {
            disinhibit_fiber(Areabyname[rule2.area1], Areabyname[rule2.area2], rule2.index);
        }
    }
}