# 自然语言依赖解析

| 姓名   | 学号     | Contribution |
| ------ | -------- | ------------ |
| 卢敦杰 | 21312028 | 40%          |
| 马梓培 | 21311525 | 40%          |
| 李涛   | 21310330 | 20%          |

代码详见附件或[github](https://github.com/xiao10ma/assemblies)

## 1. 数据对象

作者基于**AC**来实现，神经元和突触，有4个属性：

1. 脑区内的神经元随机连接
2. 神经元线性输入
3. 每个区域内的抑制机制，使得最活跃的前k个神经元触发
4. Hebbian可塑性的简单模型，即当神经元触发时，突触的强度会增加。

我们的实验中，Brain以及脑区Area均基本是基于作者开源代码实现。

### 1.1 Area

**`Area` 类分析**

`Area` 类用于表示神经网络中的一个区域，该区域包含一组神经元及其相关的属性和方法。以下是对该类各个部分的详细分析。

**类的成员变量**

- **`std::string name`**：区域的名称。
- **`int n`**：区域中的神经元数量。
- **`int k`**：在任何时间步中可以触发的神经元数量。
- **`double beta`**：区域的默认 `beta` 值，用于突触可塑性计算。
- **`int w`**：胜者神经元数量。
- **`int _new_w`**：新的胜者神经元数量，用于更新过程中的临时存储。
- **`bool explicit_area`**：标识区域是否为显式模拟区域。
- **`int num_first_winners`**：记录首批胜者神经元的数量，初始化为 -1。
- **`bool fixed_assembly`**：标识组件是否固定。
- **`int num_ever_fired`**：记录激活过的神经元数量。
- **`std::unordered_map<std::string, double> beta_by_stimulus`**：记录每个刺激对应的 `beta` 值。
- **`std::unordered_map<std::string, double> beta_by_area`**：记录每个区域对应的 `beta` 值。
- **`std::vector<int> winners`**：当前的胜者神经元列表。
- **`std::vector<int> _new_winners`**：新的胜者神经元列表，用于更新过程中的临时存储。
- **`std::vector<std::vector<int>> saved_winners`**：保存每轮的胜者神经元列表。
- **`std::vector<int> saved_w`**：保存每轮的胜者神经元数量。
- **`std::vector<bool> ever_fired`**：记录每个神经元是否激活过。

#### 构造函数

```cpp
Area(const std::string& name, int n, int k, double beta = 0.05, int w = 0, bool explicit_area = false) 
    : name(name), n(n), k(k), beta(beta), w(w), explicit_area(explicit_area),
      _new_w(0), num_first_winners(-1), fixed_assembly(false), num_ever_fired(0) {
        ever_fired.resize(n, false); // 初始化 ever_fired
}
```

- **作用**：初始化 `Area` 对象，设置成员变量的初始值，并将 `ever_fired` 向量初始化为 `n` 个 `false`。

#### 方法

##### `updateWinners`

```cpp
void updateWinners() {
    winners = _new_winners;
    if (!explicit_area) {
        w = _new_w;
    }
    // std::cout << "Updated winners: ";
    // for (const auto& winner : winners) {
    //     std::cout << winner << " ";
    // }
    std::cout << std::endl;
}
```

- **作用**：更新胜者神经元列表和数量。如果区域不是显式模拟区域，则更新 `w`。
- **未注释代码**：注释部分用于打印更新的胜者神经元列表。

##### `updateBetaByStimulus`

```cpp
void updateBetaByStimulus(const std::string& name, double new_beta) {
    beta_by_stimulus[name] = new_beta;
}
```

- **作用**：更新特定刺激对应的 `beta` 值。

##### `updateAreaBeta`

```cpp
void updateAreaBeta(const std::string& name, double new_beta) {
    beta_by_area[name] = new_beta;
}
```

- **作用**：更新特定区域对应的 `beta` 值。

##### `fixAssembly`

```cpp
void fixAssembly() {
    if (winners.empty()) {
        throw std::runtime_error("Area " + name + " does not have assembly; cannot fix.");
    }
    fixed_assembly = true;
}
```

- **作用**：固定组件。如果当前没有胜者神经元，则抛出异常。

##### `unfixAssembly`

```cpp
void unfixAssembly() {
    fixed_assembly = false;
}
```

- **作用**：取消固定组件。

##### `getNumEverFired`

```cpp
int getNumEverFired() const {
    if (explicit_area) {
        return num_ever_fired;
    } else {
        return w;
    }
}
```

- **作用**：获取激活过的神经元数量。如果区域是显式模拟区域，则返回 `num_ever_fired`，否则返回 `w`。

#### 总结

`Area` 类用于表示神经网络中的一个区域，包含该区域的神经元数量、胜者神经元、突触可塑性参数以及固定和取消固定组件的方法。通过这些属性和方法，`Area` 类能够灵活地模拟神经网络的行为，并在不同刺激和区域之间进行交互。

### 1.2 Brain

`Brain` 类用于表示一个包含多个 `Area` 的神经网络，负责管理这些区域的连接、刺激以及各种模拟操作。以下是对类的部分成员变量的详细分析。

#### 类的成员变量

##### 基础属性

- **`double p`**：神经元连接的概率，用于生成连接矩阵时的参数。
  - **作用**：控制神经元之间连接的概率，这对于模拟连接的稀疏性和密度非常重要。

- **`bool save_size`**：标志是否保存区域大小。
  - **作用**：决定是否在模拟过程中保存每个区域的大小信息。

- **`bool save_winners`**：标志是否保存胜者神经元。
  - **作用**：决定是否在模拟过程中保存胜者神经元的信息。

- **`bool disable_plasticity`**：标志是否禁用突触可塑性。
  - **作用**：用于调试或特定实验场景中禁用突触可塑性变化。

##### 区域和刺激相关属性

- **`std::unordered_map<std::string, std::shared_ptr<Area>> area_by_name`**：一个映射，键为区域名称，值为区域对象的指针。
  - **作用**：存储所有的区域，允许通过区域名称快速访问区域对象。

- **`std::unordered_map<std::string, int> stimulus_size_by_name`**：一个映射，键为刺激名称，值为刺激对应的神经元数量。
  - **作用**：记录每个刺激对应的神经元数量，用于模拟刺激对不同区域的影响。

##### 连接相关属性

- **`std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float>>> connectomes_by_stimulus`**：一个嵌套映射，记录刺激到区域的连接。
  - **外层键**：刺激名称。
  - **内层键**：目标区域名称。
  - **值**：连接强度的向量。
  - **作用**：存储每个刺激对每个目标区域的连接强度，模拟刺激传播的路径和强度。

- **`std::unordered_map<std::string, std::unordered_map<std::string, std::vector<float>>> connectomes`**：一个嵌套映射，记录区域间的连接。
  - **外层键**：源区域名称。
  - **内层键**：目标区域名称。
  - **值**：连接强度的向量。
  - **作用**：存储区域间的连接强度，模拟不同区域间的信息传递。

##### 随机数生成和其他属性

- **`std::mt19937 rng`**：Mersenne Twister 随机数生成器。
  - **作用**：用于生成随机数，以支持模拟过程中涉及的随机操作，如生成连接矩阵。

- **`bool use_normal_ppf`**：标志是否使用正态分布的累积分布函数。
  - **作用**：控制是否在模拟过程中使用正态分布的累积分布函数，可能用于某些高级模拟操作或统计分析。

- **`static const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> EMPTY_MAPPING`**：一个静态常量映射，表示空映射。
  - **作用**：用于表示默认的空映射，可能在需要返回空结果或未初始化的情况下使用。

#### 构造函数

```cpp
Brain(double p, bool save_size = true, bool save_winners = false, int seed = 0)
    : p(p), save_size(save_size), save_winners(save_winners), disable_plasticity(false), rng(seed), use_normal_ppf(false) {
}
```

- **参数**：
  - `p`：神经元连接概率。
  - `save_size`：是否保存区域大小（默认为 `true`）。
  - `save_winners`：是否保存胜者神经元（默认为 `false`）。
  - `seed`：随机数生成器的种子（默认为 `0`）。
- **初始化**：
  - 初始化神经元连接概率 `p`、是否保存区域大小 `save_size`、是否保存胜者神经元 `save_winners`、是否禁用突触可塑性 `disable_plasticity`。
  - 初始化随机数生成器 `rng` 并设置随机种子。
  - 设置 `use_normal_ppf` 为 `false`。

#### 添加脑区方法

##### `addArea` 方法

```cpp
void addArea(const std::string& area_name, int n, int k, double beta) {
    // std::cout << "Adding area: " << area_name << " with n = " << n << ", k = " << k << ", beta = " << beta << std::endl;
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
```

- **参数**：
  - `area_name`：区域名称。
  - `n`：神经元数量。
  - `k`：触发的神经元数量。
  - `beta`：默认 `beta` 值。
- **功能**：
  - 创建一个新的 `Area` 对象，并将其添加到 `area_by_name` 映射中。
  - 为新区域和现有区域之间创建连接矩阵，并初始化连接强度为 `0.0f`。
  - 更新 `beta_by_area` 映射，记录每个区域的 `beta` 值。

##### `addExplicitArea` 方法

```cpp
void addExplicitArea(const std::string& area_name, int n, int k, double beta, 
                     double custom_inner_p = -1, double custom_out_p = -1, double custom_in_p = -1) {
    // 显式地将 w 设置为 n，使得涉及该区域的所有计算都是显式的
    std::cout << "Connectomes sizes before addExplicitArea:" << std::endl;
    for (const auto& from_area : connectomes) {
        for (const auto& to_area : from_area.second) {
            std::cout << "Connectome from " << from_area.first << " to " << to_area.first << ": size = " << to_area.second.size() << std::endl;
        }
    }
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

        if (other_area_name == area_name) {  // 创建显式的
            new_connectomes[other_area_name] = generateBinomialMatrix(inner_p, n, n);
        } else {
            if (other_area->explicit_area) {
                int other_n = other_area->n;
                new_connectomes[other_area_name] = generateBinomialMatrix(out_p, n, other_n);
                connectomes[other_area_name][area_name] = generateBinomialMatrix(in_p, other_n, n);
            } else { // 我们将在运行时填充这些
                new_connectomes[other_area_name].resize(n * 0, 0.0f);
                connectomes[other_area_name][area_name].resize(0 * n, 0.0f);
            }
        }
        other_area->beta_by_area[area_name] = other_area->beta;
        the_area->beta_by_area[other_area_name] = beta;
    }
    connectomes[area_name] = new_connectomes;
    std::cout << "Connectomes sizes after addExplicitArea:" << std::endl;
    for (const auto& from_area : connectomes) {
        for (const auto& to_area : from_area.second) {
            std::cout << "Connectome from " << from_area.first << " to " << to_area.first << ": size = " << to_area.second.size() << std::endl;
        }
    }
}
```

- **参数**：
  - `area_name`：区域名称。
  - `n`：神经元数量。
  - `k`：触发的神经元数量。
  - `beta`：默认 `beta` 值。
  - `custom_inner_p`：自定义内部连接概率（默认为 `-1` 表示不使用）。
  - `custom_out_p`：自定义输出连接概率（默认为 `-1` 表示不使用）。
  - `custom_in_p`：自定义输入连接概率（默认为 `-1` 表示不使用）。
- **功能**：
  - 打印添加显式区域前的连接矩阵大小。
  - 创建一个新的显式 `Area` 对象，并将其添加到 `area_by_name` 映射中。
  - 初始化 `ever_fired` 向量和 `num_ever_fired`。
  - 为每个刺激生成连接矩阵，并更新 `beta_by_stimulus` 映射。
  - 根据提供的自定义概率或默认概率生成连接矩阵。
  - 打印添加显式区域后的连接矩阵大小。

#### 更新可塑性方法

##### `updatePlasticity` 方法

```cpp
void updatePlasticity(const std::string& from_area, const std::string& to_area, double new_beta) {
    if (area_by_name.find(to_area) != area_by_name.end()) {
        area_by_name[to_area]->beta_by_area[from_area] = new_beta;
    } else {
        std::cerr << "Error: Area " << to_area << " not found." << std::endl;
    }
}
```

- **参数**：
  - `from_area`：源区域名称。
  - `to_area`：目标区域名称。
  - `new_beta`：新的 `beta` 值。
- **功能**：
  - 更新目标区域中从源区域到目标区域的 `beta` 值。
  - 如果目标区域不存在，打印错误信息。

##### `updatePlasticities` 方法

```cpp
void updatePlasticities(const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>& area_update_map = EMPTY_MAPPING,
                    const std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>& stim_update_map = EMPTY_MAPPING) {
    // 更新区域间的可塑性
    for (const auto& to_area_pair : area_update_map) {
        const auto& to_area = to_area_pair.first;
        const auto& update_rules = to_area_pair.second;

        // std::cout << "Updating plasticities for area: " << to_area << std::endl;

        for (const auto& rule : update_rules) {
            const auto& from_area = rule.first;
            double new_beta = rule.second;

            // std::cout << " - from area: " << from_area << ", new beta: " << new_beta << std::endl;

            if (area_by

_name.find(to_area) != area_by_name.end()) {
                area_by_name[to_area]->beta_by_area[from_area] = new_beta;
                // std::cout << " - Updated " << to_area << " beta_by_area from " << from_area << " to " << new_beta << std::endl;
            } else {
                // std::cerr << "Error: Area " << to_area << " not found." << std::endl;
            }
        }
    }

    // 更新刺激的可塑性
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
        } else {
            // std::cerr << "Error: Area " << area << " not found." << std::endl;
        }
    }
}
```

- **参数**：
  - `area_update_map`：区域间 `beta` 值更新映射（默认为空映射）。
  - `stim_update_map`：刺激 `beta` 值更新映射（默认为空映射）。
- **功能**：
  - 遍历 `area_update_map`，更新目标区域中从源区域到目标区域的 `beta` 值。
  - 遍历 `stim_update_map`，更新区域中每个刺激的 `beta` 值。
  - 如果目标区域或刺激不存在，打印错误信息（注释掉的部分）。

#### `activate` 方法

```cpp
void activate(const std::string& area_name, int index) {
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
        area->w = k; // 确保设置 w 值
        area->fixAssembly();
    } else {
        std::cerr << "Error: Area " << area_name << " not found." << std::endl;
    }
}
```

- **参数**：
  - `area_name`：区域名称。
  - `index`：激活索引。
- **功能**：
  - 激活指定区域的 `k` 个神经元，从索引 `k * index` 开始。
  - 清除之前的胜者神经元列表，更新为新的胜者神经元。
  - 更新 `w` 的值，并固定组件。
  - 如果区域名称不存在，打印错误信息。

#### `projectInto` 方法

`projectInto` 函数是 `Brain` 类的核心方法之一，用于模拟大脑中神经元之间的投影过程，更新目标区域的胜者神经元，并处理来自其他区域和刺激的连接。下面是这个函数的详细分析。

```cpp
int projectInto(Area& target_area, const std::vector<std::string>& from_stimuli, const std::vector<std::string>& from_areas, int verbose = 0)
```

- **参数**：
  - `target_area`：目标区域对象。
  - `from_stimuli`：来源刺激的名称列表。
  - `from_areas`：来源区域的名称列表。
  - `verbose`：用于控制调试信息输出的级别（默认为 0）。

##### 初始化变量

```cpp
auto rng = std::mt19937{ std::random_device{}() };
auto area_by_name = this->area_by_name;
std::string target_area_name = target_area.name;

int num_first_winners_processed;
std::vector<std::vector<int>> inputs_by_first_winner_index(num_first_winners_processed);
```

- **作用**：初始化随机数生成器 `rng`，设置目标区域名称 `target_area_name`，以及其他辅助变量。

##### fixed assembly的处理

```cpp
if (target_area.fixed_assembly) {
    target_area._new_winners = target_area.winners;
    target_area._new_w = target_area.w;
    std::vector<float> first_winner_inputs;
    num_first_winners_processed = 0;
} 
```

- **作用**：如果目标区域的组件已固定，则直接使用现有的胜者神经元，不进行新的计算。初始化新的胜者神经元和相应的输入。

##### 计算来自其他区域的投影输入

```cpp
else {
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
    }

    if (verbose >= 2) {
        std::cout << "prev_winner_inputs: ";
        for (const auto& val : prev_winner_inputs) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
```

- **作用**：计算来自其他区域的投影输入。对于每个来源区域，遍历其胜者神经元，累加它们对目标区域神经元的输入权重。

##### 计算潜在的新的胜者神经元

```cpp
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
```

- **作用**：
  - 如果目标区域不是显式区域，计算来自来源刺激的投影输入。
  - 计算目标区域剩余的可用神经元数，并检查是否足够进行新的采样。
  - 计算输入的分位点和阈值，生成潜在的新胜者神经元输入。
  - 将潜在的新胜者神经元输入与现有的胜者神经元输入合并。
  - 排序并选择前 `k` 个最大的输入作为新的胜者神经元。

##### 更新显式区域的胜者神经元状态

```cpp
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
```

- **作用**：
  - 更新显式区域的胜者神经元状态。
  - 如果目标区域不是显式区域，确定新的胜者神经元并更新其索引。
  - 更新目标区域的新胜者神经元和新的神经元数量。
  - 输出调试信息（可选）。

##### 处理来自刺激的连接

```cpp
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

    int num_inputs_processed = 0;

    // 处理来自刺激的连接
    for (const auto& stim : from_stimuli) {
        auto& connectomes = connectomes_by_stimulus[

stim];
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

    // 更新未触发的刺激连接
    if (!target_area.explicit_area && num_first_winners_processed > 0) {
        for (auto& [stim_name, connectomes] : connectomes_by_stimulus) {
            if (std::find(from_stimuli.begin(), from_stimuli.end(), stim_name) != from_stimuli.end()) continue;
            connectomes[target_area_name].resize(target_area._new_w);
            std::generate(connectomes[target_area_name].begin() + target_area.w, connectomes[target_area_name].end(), [this] {
                return std::binomial_distribution<>(1, p)(rng);
            });
        }
    }
```

- **作用**：
  - 处理来自刺激的连接。
  - 更新目标区域的连接矩阵，调整突触权重。
  - 如果目标区域不是显式区域，更新未触发的刺激连接。

##### 处理来自其他区域的连接

```cpp
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
```

- **作用**：
  - 处理来自其他区域的连接。
  - 更新连接矩阵，调整突触权重。
  - 如果连接矩阵为空，进行初始化。
  - 更新显式区域和未显式区域之间的连接。

##### 扩展来自其他未触发区域的连接

```cpp
    for (auto& [other_area_name, other_area] : area_by_name) {
        if (std::find(from_areas.begin(), from_areas.end(), other_area_name) == from_areas.end()) {
            auto& the_other_area_connectome = connectomes[other_area_name][target_area_name];
            the_other_area_connectome.resize(other_area->w * target_area._new_w);
            std::generate(the_other_area_connectome.begin() + other_area->w * target_area.w, the_other_area_connectome.end(), [this] {
                return std::binomial_distribution<>(1, p)(rng);
            });
        }

        auto& target_area_connectome = connectomes[target_area_name][other_area_name];
        target_area_connectome.resize(target_area._new_w * other_area->w);
        std::generate(target_area_connectome.begin() + target_area.w * other_area->w, target_area_connectome.end(), [this, other_area_name, target_area_name=target_area.name] {
            return std::binomial_distribution<>(1, p)(rng);
        });

        if (verbose >= 2) {
            std::cout << "Connectome of " << target_area_name << " to " << other_area_name << " is now: ";
            for (const auto& val : target_area_connectome) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
```

- **作用**：
  - 扩展来自其他未触发区域的连接。
  - 初始化和更新连接矩阵。
  - 调整连接矩阵的大小，并生成新的连接。

*总结*

`projectInto` 函数主要用于模拟大脑区域之间的神经元投影和突触更新。其主要步骤包括：

1. 初始化变量。
2. 处理固定组件的投影。
3. 计算来自其他区域的投影输入。
4. 计算潜在的新的胜者神经元。
5. 更新显式区域的胜者神经元状态。
6. 处理来自刺激的连接。
7. 处理来自其他区域的连接。
8. 扩展来自其他未触发区域的连接。

通过这些步骤，函数能够模拟神经元投影过程，更新目标区域的胜者神经元，并处理不同区域和刺激之间的突触连接。这对于理解和模拟大脑的计算模型至关重要。

### `updateConnectomes` 函数

```cpp
void updateConnectomes(Area& target_area, const std::vector<std::string>& from_stimuli, const std::vector<std::string>& from_areas, 
                       const std::vector<int>& new_winner_indices, const std::vector<float>& prev_winner_inputs, int verbose)
```

- **参数**：
  - `target_area`：目标区域对象。
  - `from_stimuli`：来源刺激的名称列表。
  - `from_areas`：来源区域的名称列表。
  - `new_winner_indices`：新的胜者神经元索引。
  - `prev_winner_inputs`：先前的胜者输入向量。
  - `verbose`：用于控制调试信息输出的级别。

```cpp
int num_first_winners_processed = new_winner_indices.size();
int num_inputs_processed = 0;
```

- **作用**：初始化处理的胜者神经元数量和输入处理计数器。

```cpp
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
```

- **作用**：更新来自刺激的突触连接权重。首先调整连接矩阵的大小，然后根据新的胜者神经元更新权重。
  - `connectomes[target_area.name].resize(target_area._new_w)`：调整目标连接矩阵的大小。
  - `target_connectome[i] = 1.0 + target_area.beta_by_stimulus[stim]`：初始化新神经元的权重。
  - `target_connectome[i] *= 1 + target_area.beta_by_stimulus[stim]`：更新现有神经元的权重。

```cpp
if (!target_area.explicit_area && num_first_winners_processed > 0) {
    for (auto& [stim_name, connectomes] : connectomes_by_stimulus) {
        if (std::find(from_stimuli.begin(), from_stimuli.end(), stim_name) != from_stimuli.end()) continue;
        connectomes[target_area.name].resize(target_area._new_w);
        std::generate(connectomes[target_area.name].begin() + target_area.w, connectomes[target_area.name].end(), [this] {
            return std::binomial_distribution<>(1, p)(rng);
        });
    }
}
```

- **作用**：对于未触发的刺激，生成新的突触连接，确保矩阵大小正确。
  - `connectomes[target_area.name].resize(target_area._new_w)`：调整连接矩阵的大小。
  - `std::generate`：使用二项分布生成新的连接。

```cpp
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
```

- **作用**：更新来自其他区域的突触连接权重。对于新的胜者神经元，调整突触权重。
  - `sampleFromSet(from_area->winners, prev_winner_inputs[i])`：从胜者集合中随机采样。
  - `from_area_connectomes[target_area.name][idx * target_area._new_w + i] = 1.0`：初始化新神经元的权重。
  - `from_area_connectomes[target_area.name][j * target_area._new_w + i] *= 1.0 + target_area.beta_by_area[from_area_name]`：更新现有神经元的权重。

```cpp
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
```

- **作用**：对于未触发的其他区域，生成新的突触连接，确保矩阵大小正确。
  - `the_other_area_connectome.resize(other_area->w * target_area._new_w)`：调整连接矩阵的大小。
  - `std::generate`：使用二项分布生成新的连接。

### `project` 函数详细分析

```cpp
void project(const std::unordered_map<std::string, std::vector<std::string>>& areas_by_stim, 
             const std::unordered_map<std::string, std::vector<std::string>>& dst_areas_by_src_area, 
             int verbose = 0)
```

- **参数**：
  - `areas_by_stim`：刺激与区域的映射。
  - `dst_areas_by_src_area`：源区域与目标区域的映射。
  - `verbose`：用于控制调试信息输出的级别。

```cpp
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
```

- **作用**：验证输入的有效性，确保所有指定的区域都在 `area_by_name` 中。收集需要更新的区域名称。

```cpp
for (const auto& area_name : to_update_area_names) {
    auto& area = *area_by_name[area_name];
    std::cout << "Before projection, Area " << area_name << " has winners: ";
    for (const auto& winner : area.winners) {
        std::cout << winner << " ";
    }
    std::cout << std::endl;
    int num_first_winners = projectInto(area, stim_in[area_name], area_in[area_name], verbose);
    area.num_first_winners = num_first_winners;
    if (save_winners) {
        area.saved_winners.push_back(area._new_winners);
        std::cout << "Saved winners in Area " << area_name << ": ";
        for (const auto& winner : area._new_winners) {
            std::cout << winner << " ";
        }
        std::cout << std::endl;
    }
}
```

- **作用**：对每个需要更新的区域执行投影操作，并记录新的胜者神经元。如果需要，保存胜者神经元。

```cpp
for (const auto& area_name : to_update_area_names) {
    auto& area = *area_by_name[area_name];
    area.updateWinners();
    std::cout << "After updating winners, Area " << area_name << " has winners: ";
    for (const auto& winner : area.winners) {
        std::cout << winner << " ";
    }
    std::cout << std::endl;
    if (save_size) {
        area.saved_w.push_back(area.w);
        std::cout << "Saved sizes in Area " << area_name << ": ";
        for (const auto& size : area.saved_w) {
            std::cout << size << " ";
        }
        std::cout << std::endl;
    }
}
```

- **作用**：更新区域的胜者神经元，并保存结果（包括新的胜者神经元和区域大小）。

### 辅助函数

#### `generateBinomialVector`

```cpp
std::vector<float> generateBinomialVector(int size, double p, int n) {
    std::vector<float> result(n);
    std::binomial_distribution<int> binom_dist(size, p);
    for (auto& val : result) {
        val = static_cast<float>(binom_dist(rng));
    }
    return result;
}
```

- **作用**：生成一个包含二项分布值的向量。
- **实现**：使用 `std::binomial_distribution` 生成 `n` 个值，每个值代表在 `size` 次试验中成功的次数，成功概率为 `p`。

#### `generateBinomialMatrix`

```cpp
std::vector<float> generateBinomialMatrix(double p, int rows, int cols) {
    std::vector<float> matrix(rows * cols);
    std::binomial_distribution<int> binom_dist(1, p);
    for (auto& val : matrix) {
        val = static_cast<float>(binom_dist(rng));
    }
    return matrix;
}
```

- **作用**：生成一个包含二项分布值的矩阵。
- **实现**：使用 `std::binomial_distribution` 生成 `rows * cols` 个值，每个值代表在一次试验中成功的次数，成功概率为 `p`。

#### `binom_ppf`

```cpp
double binom_ppf(double quantile, int total_k, double p) {
    std::binomial_distribution<int> binom_dist(total_k, p);
    std::vector<int> results;
    for (int i = 0; i < 10000; ++i) {
        results.push_back(binom_dist(rng));
    }
    std::sort(results.begin(), results.end());
    return results[static_cast<int>(quantile * results.size())];
}
```

- **作用**：模拟 `binom.ppf` 函数，计算分位点。
- **实现**：生成 10000 个二项分布样本，按升序排列后，返回指定分位点对应的值。

#### `truncnorm_rvs`

```cpp
double truncnorm_rvs(double a, double b, double scale, std::mt19937& rng) {
    std::normal_distribution<double> norm_dist(0.0, scale);
    double result;
    do {
        result = norm_dist(rng);
    } while (result < a || result > b);
    return result;
}
```

- **作用**：模拟截断正态分布，生成在区间 `[a, b]` 内的随机数。
- **实现**：使用 `std::normal_distribution` 生成随机数，直到生成的值在 `[a, b]` 区间内。

#### `sampleFromSet`

```cpp
std::vector<int> sampleFromSet(const std::vector<int>& winners, int sample_size) {
    std::vector<int> sampled_indices(sample_size);
    std::sample(winners.begin(), winners.end(), sampled_indices.begin(), sample_size, rng);
    return sampled_indices;
}
```

- **作用**：从胜者集合中随机采样指定数量的元素。
- **实现**：使用 `std::sample` 从 `winners` 集合中随机选择 `sample_size` 个元素。

### 1.3 parserbrain

`parserbrain`继承自`Brain`，主要的实现基于作者开源代码的`parser.py`文件

```cpp
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
```

基于论文，**参数解释：**

- lexeme_dict：词典，这一部分，我们会在报告后面详细讲解
- all_areas：所有的脑区，`std::vector<std::string>`
- recurrent_areas：作者未在论文提及，`{SUBJ, OBJ, VERB, ADJ, ADVERB, PREP, PREP_P}`

- initial_areas：`{LEX, SUBJ, VERB}`

- readout_rules：parse解析后，readout 的方式
- fiber_states：
- area_states：
- activated_fibers：
- area_by_name：数据类型为`std::map<std::string, nemo::Area>`，根据脑区名索引到Area脑区

#### 构造函数

```cpp
ParserBrain(double p, const map<string, Generic>& lexeme_dict = {}, 
            const vector<string>& all_areas = {}, 
            const vector<string>& recurrent_areas = {}, 
            const vector<string>& initial_areas = {}, 
            const map<string, vector<string>>& readout_rules = {})
    : Brain(p), lexeme_dict(lexeme_dict), all_areas(all_areas), 
      recurrent_areas(recurrent_areas), initial_areas(initial_areas), readout_rules(readout_rules) {
    initialize_states();
}
```

- **作用**：构造函数初始化父类 `Brain`，并对自身的成员变量进行初始化，最后调用 `initialize_states` 方法初始化状态。

#### `initialize_states` 方法

```cpp
void initialize_states() {
    for (const auto& from_area : all_areas) {
        fiber_states[from_area] = map<string, set<int>>();
        for (const auto& to_area : all_areas) {
            fiber_states[from_area][to_area].insert(0);
        }
    }

    for (const auto& area : all_areas) {
        area_states[area].insert(0);
    }

    for (const auto& area : initial_areas) {
        area_states[area].erase(0);
    }
}
```

- **作用**：初始化 `fiber_states` 和 `area_states`，设置所有区域和纤维的初始状态。

#### `applyFiberRule` 方法

```cpp
void applyFiberRule(const FiberRule& rule) {
    if (rule.action == INHIBIT) {
        fiber_states[rule.area1][rule.area2].insert(rule.index);
        fiber_states[rule.area2][rule.area1].insert(rule.index);
    } else if (rule.action == DISINHIBIT) {
        fiber_states[rule.area1][rule.area2].erase(rule.index);
        fiber_states[rule.area2][rule.area1].erase(rule.index);
    }
}
```

- **作用**：根据规则更新纤维状态，抑制或去抑制特定的纤维。

#### `applyAreaRule` 方法

```cpp
void applyAreaRule(const AreaRule& rule) {
    if (rule.action == INHIBIT) {
        area_states[rule.area].insert(rule.index);
    } else if (rule.action == DISINHIBIT) {
        area_states[rule.area].erase(rule.index);
    }
}
```

- **作用**：根据规则更新区域状态，抑制或去抑制特定的区域。

#### `applyRule` 方法

```cpp
bool applyRule(const variant<AreaRule, FiberRule>& rule) {
    if (holds_alternative<FiberRule>(rule)) {
        applyFiberRule(get<FiberRule>(rule));
        return true;
    } else if (holds_alternative<AreaRule>(rule)) {
        applyAreaRule(get<AreaRule>(rule));
        return true;
    }
    return false;
}
```

- **作用**：根据规则类型调用相应的方法更新区域或纤维状态。

#### `parse_project` 方法

```cpp
void parse_project() {
    auto project_map = getProjectMap();
    remember_fibers(project_map);

    project({}, project_map);
}
```

- **作用**：获取投影映射，记住被激活的纤维，并执行投影操作。

#### `remember_fibers` 方法

```cpp
void remember_fibers(const std::unordered_map<std::string, std::vector<std::string>>& project_map) {
    for (const auto& [from_area, to_areas] : project_map) {
        for (const auto& to_area : to_areas) {
            if (std::find(activated_fibers[from_area].begin(), activated_fibers[from_area].end(), to_area) == activated_fibers[from_area].end()) {
                activated_fibers[from_area].push_back(to_area);
            }
        }
    }
}
```

- **作用**：记录当前被激活的纤维。

#### `recurrent` 方法

```cpp
bool recurrent(const string& area) {
    return find(recurrent_areas.begin(), recurrent_areas.end(), area) != recurrent_areas.end();
}
```

- **作用**：检查给定的区域是否是递归区域。

#### `getProjectMap` 方法

```cpp
virtual std::unordered_map<std::string, std::vector<std::string>> getProjectMap() {
    std::unordered_map<std::string, std::vector<std::string>> proj_map;
    for (const auto& area1 : all_areas) {
        if (area_states[area1].empty()) {
            for (const auto& area2 : all_areas) {
                if (area1 == LEX && area2 == LEX) continue;
                if (area_states[area2].empty()) {
                    if (fiber_states[area1][area2].empty()) {
                        if (!area_by_name[area1]->winners.empty()) {
                            proj_map[area1].push_back(area2);
                        }
                        if (!area_by_name[area2]->winners.empty()) {
                            proj_map[area2].push_back(area2);
                        }
                    }
                }
            }
        }
    }
    return proj_map;
}
```

- **作用**：生成一个投影映射，描述哪些区域应该投影到哪些其他区域。

#### `activateWord` 方法

```cpp
void activateWord(const string& area_name, const string& word) {
    auto& area = area_by_name[area_name];
    int k = area->k;
    int assembly_start = lexeme_dict[word].index * k;
    area->winners.clear();
    for (int i = 0; i < k; ++i) {
        area->winners.push_back(assembly_start + i);
    }
    area->fixAssembly();
}
```

- **作用**：激活指定区域中的某个词汇，更新该区域的胜者神经元。

#### `activateIndex` 方法

```cpp
void activateIndex(const string& area_name, int index) {
    auto& area = area_by_name[area_name];
    int k = area->k;
    int assembly_start = index * k;
    area->winners.clear();
    for (int i = 0; i < k; ++i) {
        area->winners.push_back(assembly_start + i);
    }
    area->fixAssembly();
}
```

- **作用**：激活指定区域中的某个索引，更新该区域的胜者神经元。

#### `interpretAssemblyAsString` 方法

```cpp
string interpretAssemblyAsString(const string& area_name) {
    return getWord(area_name, 0.7);
}
```

- **作用**：将某个区域中的胜者神经元解释为字符串。

#### `getWord` 方法

```cpp
virtual string getWord(const string& area_name, double min_overlap = 0.7) {
    if (area_by_name[area_name]->winners.empty()) {
        throw runtime_error("Cannot get word because no assembly in " + area_name);
    }
    set<int> winners(area_by_name[area_name]->winners.begin(), area_by_name[area_name]->winners.end());
    int area_k = area_by_name[area_name]->k;
    int threshold = min_overlap * area_k;

    for (const auto& [word, lexeme] : lexeme_dict) {
        int word_index = lexeme.index;
        set<int> word_assembly;
        for (int i = 0; i < area_k; ++i) {
            word_assembly.insert(word_index * area_k + i);
        }
        
        set<int> intersection;
        set_intersection(winners.begin(), winners.end(), word_assembly.begin(), word_assembly.end(),
                         inserter(intersection, intersection.begin()));
        
        if (intersection.size() >= threshold) {
            return word;
        }
    }
    return "";
}
```

- **作用**：根据重叠度阈值，从胜者神经元中获取对应的词汇。

#### `getActivatedFibers` 方法

```cpp
std::unordered_map<std::string, std::vector<std::string>> getActivatedFibers() const {
    std::unordered_map<std::string, std::vector<std::string>> pruned_activated_fibers;
    for (const auto& [from_area, to_areas] : activated_fibers) {


        for (const auto& to_area : to_areas) {
            if (std::find(readout_rules.at(from_area).begin(), readout_rules.at(from_area).end(), to_area) != readout_rules.at(from_area).end()) {
                pruned_activated_fibers[from_area].push_back(to_area);
            }
        }
    }
    return pruned_activated_fibers;
}
```

- **作用**：获取当前激活的纤维，按照读取规则进行修剪。

### 总结

`ParserBrain` 类继承自 `Brain` 类，扩展了其功能，专门用于处理语言解析任务。该类包含了各种用于管理和操作脑区、纤维状态的方法，以及语言解析相关的方法，如激活词汇、解释胜者神经元为字符串等。通过对这些方法的分析，可以看出该类是一个复杂的神经网络模拟器，能够处理语言解析任务并进行相应的投影和状态更新操作。

### 1.3 EnglishParserBrain

`EnglishParserBrain`继承自`ParserBrain`，其主要功能就是在`ParserBrain`类的基础上，可以实现English sentence的解析与readout。

```cpp
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
```

#### 构造函数

```cpp
EnglishParserBrain(double p, int non_LEX_n = 10000, int non_LEX_k = 100, int LEX_k = 20,
                   double default_beta = 0.2, double LEX_beta = 1.0, double recurrent_beta = 0.05, 
                   double interarea_beta = 0.5, bool verbose = false)
    : ParserBrain(p, LEXEME_DICT, AREAS, RECURRENT_AREAS, {LEX, SUBJ, VERB}, ENGLISH_READOUT_RULES), verbose(verbose) {

    int LEX_n = LEX_SIZE * LEX_k;
    addExplicitArea(LEX, LEX_n, LEX_k, default_beta);

    int DET_k = LEX_k;
    addArea(SUBJ, non_LEX_n, non_LEX_k, default_beta);
    addArea(OBJ, non_LEX_n, non_LEX_k, default_beta);
    addArea(VERB, non_LEX_n, non_LEX_k, default_beta);
    addArea(ADJ, non_LEX_n, non_LEX_k, default_beta);
    addArea(PREP, non_LEX_n, non_LEX_k, default_beta);
    addArea(PREP_P, non_LEX_n, non_LEX_k, default_beta);
    addArea(DET, non_LEX_n, DET_k, default_beta);
    addArea(ADVERB, non_LEX_n, non_LEX_k, default_beta);

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
```

- **作用**：初始化 `EnglishParserBrain` 对象，配置特定于英语解析的脑区和连接。

  - `p`：连接概率。
  - `non_LEX_n` 和 `non_LEX_k`：非 LEX 区域的神经元数量和激活神经元数量。
  - `LEX_k`：LEX 区域的激活神经元数量。
  - `default_beta`、`LEX_beta`、`recurrent_beta` 和 `interarea_beta`：可塑性参数。
  - `verbose`：是否输出详细信息。

- **初始化脑区**：
  - 使用 `addExplicitArea` 添加 LEX 区域。
  - 使用 `addArea` 添加其他语法角色对应的区域（例如 SUBJ, OBJ, VERB 等）。

- **设置自定义可塑性**：
  - 对于每个递归区域，设置其与 LEX 以及其他区域的可塑性参数。

#### `getProjectMap` 方法

```cpp
std::unordered_map<std::string, std::vector<std::string>> getProjectMap() override {
    auto proj_map = ParserBrain::getProjectMap();
    if (proj_map.find(LEX) != proj_map.end() && proj_map[LEX].size() > 2) {
        std::string areas;
        for (const auto& area : proj_map[LEX]) {
            areas += area + " ";
        }
        throw std::runtime_error("Got that LEX projecting into many areas: " + areas);
    }
    return proj_map;
}
```

- **作用**：重载 `getProjectMap` 方法，确保 LEX 区域不会投影到超过两个区域。如果超过两个区域则抛出异常。

#### `getWord` 方法

```cpp
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
```

- **作用**：重载 `getWord` 方法，首先尝试使用父类方法获取词汇。如果失败并且区域为 DET，则进一步检查是否为 `<null-det>`，否则返回 `<NON-WORD>`。

#### `join` 方法

```cpp
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
```

- **作用**：将集合中的字符串连接成一个用逗号分隔的字符串。

#### 总结

`EnglishParserBrain` 类专门用于处理英语语言解析任务。它通过添加和配置特定的脑区（如 LEX, SUBJ, OBJ 等）以及设置自定义的可塑性参数，实现了对英语句子的解析。重载的方法确保了 LEX 区域的正确投影，并提供了特定于英语解析的词汇获取逻辑。

## 2. Lexeme Dict

我们实验的`Lexeme Dict`，主要依托于作者的`parser.py`中的实现。阅读论文，我们可以知道每个单词，都有其对应的**action**，例如原论文中：

![](https://github.com/xiao10ma/assemblies/blob/master/word_action.png?raw=true)

我们借鉴作者的实现，针对于每种词性，都设计了其对应的action，例如，verb：

```cpp
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
```

其余详见代码文件的实现，这里不过多赘述。

因此，要是想构建词典。我们只需要明确每个英文单词的词性，并调用其对应的函数，即可得到一个单词的action。

以下是部分词典的实现截图：

```cpp
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
```

## 3. 解析过程

### `parseHelper` 

`parseHelper` 函数是解析器系统的核心功能实现，它负责将输入的句子分解成单词，并通过多轮投影和规则应用来解析句子结构。以下是对该函数的详细分析：

```cpp
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
```

- **参数**：
  - `b`：解析器大脑对象。
  - `sentence`：待解析的句子。
  - `p`：连接概率。
  - `LEX_k`：LEX 区域的激活神经元数量。
  - `project_rounds`：投影轮数。
  - `verbose`：是否输出详细信息。
  - `debug`：是否启用调试模式。
  - `lexeme_dict`：词典，包含单词和其对应的预处理和后处理规则。
  - `all_areas`：所有脑区的列表。
  - `explicit_areas`：显式脑区的列表。
  - `readout_method`：读取方法。
  - `readout_rules`：读取规则。

#### 步骤解析

1. **分解句子成单词**：

```cpp
std::istringstream iss(sentence);
std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
```

- 使用 `istringstream` 和 `istream_iterator` 将句子分解为单词，并存储在 `words` 向量中。

2. **遍历每个单词**：

```cpp
for (const auto& word : words) {
    auto lexeme = lexeme_dict.at(word);
    b.activateWord(LEX, word);
```

- 从词典中获取单词对应的 `lexeme`，然后激活 LEX 区域中的单词。

3. **应用预处理规则**：

```cpp
for (const auto& rule : lexeme.preRules) {
    std::visit([&b](auto&& r) { b.applyRule(r); }, rule);
}
```

- 对每个单词，应用其对应的预处理规则。

4. **获取投影映射并进行多轮投影**：

```cpp
auto proj_map = b.getProjectMap();
for (int i = 0; i < project_rounds; ++i) {
    b.parse_project();
    if (verbose) {
        proj_map = b.getProjectMap();
    }
}
```

- 获取投影映射并进行多轮投影。如果启用了详细模式，则输出投影映射。

5. **应用后处理规则**：

```cpp
for (const auto& rule : lexeme.postRules) {
    std::visit([&b](auto&& r) { b.applyRule(r); }, rule);
}
```

- 对每个单词，应用其对应的后处理规则。

6. **读取操作前的调试信息**：

```cpp
std::cout << "Before readout:" << std::endl;
for (const auto& area : all_areas) {
    std::cout << "Area: " << area << " has winners: ";
    for (const auto& winner : b.area_by_name[area]->winners) {
        std::cout << winner << " ";
    }
    std::cout << std::endl;
}
```

- 输出读取操作前的调试信息，包括每个脑区的获胜神经元。

7. **读取操作**：

```cpp
b.disable_plasticity = true;
for (const auto& area : all_areas) {
    b.area_by_name[area]->unfixAssembly();
}
```

- 禁用可塑性并取消固定每个脑区的组件。

8. **读取依赖关系**：

```cpp
std::vector<std::tuple<std::string, std::string, std::string>> dependencies;
```

- 初始化依赖关系向量，用于存储读取到的依赖关系。

9. **读取函数 `read_out`**：

```cpp
std::function<void(const std::string&, const std::unordered_map<std::string, std::vector<std::string>>&)> read_out = 
[&](const std::string& area, const std::unordered_map<std::string, std::vector<std::string>>& mapping) {
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
```

- 递归读取依赖关系，并存储在 `dependencies` 向量中。

10. **根据读取方法进行读取**：

```cpp
if (readout_method == 0) {
    read_out(VERB, readout_rules);
} else if (readout_method == 1) {
    auto activated_fibers = b.getActivatedFibers();
    read_out(VERB, activated_fibers);
}
```

- 根据读取方法选择不同的读取规则，执行读取操作并输出结果。

### 3.1 流程

<img src="https://github.com/xiao10ma/assemblies/blob/master/output.png?raw=true" style="zoom: 25%;" />

## 4. 实验过程

1. cats chase mice:

```bash
input the sentence:cats chase mice
Dependency Parsing Result:
chase->cats (SUBJ)
chase->mice (OBJ)

Parsing Tree:
chase
    └── cats
    └── mice
```

解析句子“cats chase mice”的结果如下：

- **句法结构**：
  - "chase" 是句子的谓语动词（VERB）。
  - "cats" 是动词 "chase" 的主语（SUBJ）。
  - "mice" 是动词 "chase" 的宾语（OBJ）。

- **依存关系**：
  - **chase->cats (SUBJ)**：表示 "cats" 是动词 "chase" 的主语。
  - **chase->mice (OBJ)**：表示 "mice" 是动词 "chase" 的宾语。

- **解析树**：
  - 根节点是动词 "chase"。
  - 从根节点 "chase" 引出两个子节点："cats" 和 "mice"。
  - "cats" 作为主语与动词 "chase" 连接，标记为 "SUBJ"。
  - "mice" 作为宾语与动词 "chase" 连接，标记为 "OBJ"。

2. the big dog chase the mice quickly

```bash
input the sentence:the big dog chase the mice quickly
Dependency Parsing Result:
chase->dog (SUBJ)
chase->mice (OBJ)
chase->quickly (ADVERB)
mice->the (DET)
dog->the (DET)
dog->big (ADJ)

Parsing Tree:
chase
    └── dog
        └── the
        └── big
    └── mice
        └── the
    └── quickly
```

- **句法结构**：
  - "chase" 是句子的谓语动词（VERB）。
  - "dog" 是动词 "chase" 的主语（SUBJ）。
  - "mice" 是动词 "chase" 的宾语（OBJ）。
  - "quickly" 是修饰动词 "chase" 的副词（ADVERB）。

- **依存关系**：
  - **chase->dog (SUBJ)**：表示 "dog" 是动词 "chase" 的主语。
  - **chase->mice (OBJ)**：表示 "mice" 是动词 "chase" 的宾语。
  - **chase->quickly (ADVERB)**：表示 "quickly" 是修饰动词 "chase" 的副词。
  - **mice->the (DET)**：表示 "the" 是修饰名词 "mice" 的限定词。
  - **dog->the (DET)**：表示 "the" 是修饰名词 "dog" 的限定词。
  - **dog->big (ADJ)**：表示 "big" 是修饰名词 "dog" 的形容词。

- **解析树**：
  - 根节点是动词 "chase"。
  - 从根节点 "chase" 引出三个子节点："dog"、"mice" 和 "quickly"。
    - "dog" 作为主语与动词 "chase" 连接，标记为 "SUBJ"。
      - "dog" 有两个子节点："the" 和 "big"，分别是限定词（DET）和形容词（ADJ）。
    - "mice" 作为宾语与动词 "chase" 连接，标记为 "OBJ"。
      - "mice" 有一个子节点："the"，是限定词（DET）。
    - "quickly" 作为副词与动词 "chase" 连接，标记为 "ADVERB"。

3. people love dog

```bash
input the sentence:people love dog           
Dependency Parsing Result:
love->people (SUBJ)
love->dog (OBJ)

Parsing Tree:
love
    └── people
    └── do
```

- **句法结构**：
  - "love" 是句子的谓语动词（VERB）。
  - "people" 是动词 "love" 的主语（SUBJ）。
  - "dog" 是动词 "love" 的宾语（OBJ）。
- **依存关系**：
  - **love->people (SUBJ)**：表示 "people" 是动词 "love" 的主语。
  - **love->dog (OBJ)**：表示 "dog" 是动词 "love" 的宾语。
- **解析树**：
  - 根节点是动词 "love"。
  - 从根节点 "love" 引出两个子节点："people" 和 "dog"。
    - "people" 作为主语与动词 "love" 连接，标记为 "SUBJ"。
    - "dog" 作为宾语与动词 "love" 连接，标记为 "OBJ"。

4. a man saw a dog bite the woman

```bash
input the sentence:a man saw a dog bite the woman
Dependency Parsing Result:
saw->man (SUBJ)
man->a (DET)
saw->bite (XCOMP)
bite->dog (SUBJ)
dog->a (DET)
bite->woman (OBJ)
woman->the (DET)

Parsing Tree:
saw
    └── man
        └── a
    └── bite
        └── dog
            └── a
        └── woman
            └── the
```

- **句法结构：**
  - "saw" 是句子的主要谓语动词。

  - "man" 是动词 "saw" 的主语。

  - "a" 是限定词，修饰名词 "man"。

  - "dog" 是动词 "bite" 的主语。

  - "bite" 是动词，嵌入在动词 "saw" 的宾语从句中。

  - "the woman" 是动词 "bite" 的宾语。

- **完整的依存关系：**
  - saw->man (SUBJ)：表示 "man" 是动词 "saw" 的主语。
  - man->a (DET)：表示 "a" 是修饰名词 "man" 的限定词。
  - saw->bite (XCOMP)：表示 "bite" 是嵌入动词 "saw" 的宾语从句中的动词。
  - bite->dog (SUBJ)：表示 "dog" 是动词 "bite" 的主语。
  - dog->a (DET)：表示 "a" 是修饰名词 "dog" 的限定词。
  - bite->woman (OBJ)：表示 "woman" 是动词 "bite" 的宾语。
  - woman->the (DET)：表示 "the" 是修饰名词 "woman" 的限定词。


5. people love cats in the big house

```bash
input the sentence:people love cats in the big house
Dependency Parsing Result:
love->people (SUBJ)
love->cats (OBJ)
cats->big (ADJ)
people->big (ADJ)

Parsing Tree:
love
    └── people
        └── big
    └── cats
        └── big
```

解析句子“people love cats in the big house”的结果如下：

1. **句法结构**：
   - "love" 是句子的谓语动词（VERB）。
   - "people" 是动词 "love" 的主语（SUBJ）。
   - "cats" 是动词 "love" 的宾语（OBJ）。
   - "in the big house" 是介词短语，修饰 "love" 的动作。

2. **依存关系**：
   - **love->people (SUBJ)**：表示 "people" 是动词 "love" 的主语。
   - **love->cats (OBJ)**：表示 "cats" 是动词 "love" 的宾语。
   - **cats->the (DET)**：表示 "the" 是修饰名词 "cats" 的限定词。
   - **cats->big (ADJ)**：表示 "big" 是修饰名词 "house" 的形容词。
   - **house->in (PREP)**：表示 "in" 是修饰名词 "house" 的介词短语。

3. **解析树**：
   - 根节点是动词 "love"。
   - 从根节点 "love" 引出两个主要子节点："people" 和 "cats"。
     - "people" 作为主语与动词 "love" 连接，标记为 "SUBJ"。
     - "cats" 作为宾语与动词 "love" 连接，标记为 "OBJ"。
       - "cats" 有一个子节点 "the"，是限定词（DET）。
       - "cats" 有一个子节点 "in the big house"，是介词短语（PREP）。
         - "house" 有子节点 "the"，是限定词（DET）。
         - "house" 有子节点 "big"，是形容词（ADJ）。
