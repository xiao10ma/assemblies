# 自然语言依赖解析

| 姓名   | 学号     | Contribution |
| ------ | -------- | ------------ |
| 卢敦杰 | 21312028 | 40%          |
| 马梓培 | 21311525 | 40%          |
| 李涛   | 21310330 | 20%          |

代码详见附件或[github](https://github.com/xiao10ma/assemblies)

## 1. 数据对象

### 1.1 Brain

作者基于**AC**来实现，神经元和突触，有4个属性：

1. 脑区内的神经元随机连接
2. 神经元线性输入
3. 每个区域内的抑制机制，使得最活跃的前k个神经元触发
4. Hebbian可塑性的简单模型，即当神经元触发时，突触的强度会增加。

我们的实验中，Brain（突触Synapse，纤维Fiber，脑区Area）基本是基于作者开源代码。

#### 1.1.1 Synapse

`Synapse` 结构体用于表示神经网络中的一个突触。突触是神经元之间传递信号的连接点。在这个结构体中，有两个主要成员变量：`neuron` 和 `weight`。以下是对这个结构体的详细介绍：

**结构体定义**

```cpp
struct Synapse {
    uint32_t neuron; // 目标神经元的标识符
    float weight;    // 突触的权重
};
```

1. **neuron**:
   - 类型：`uint32_t`
   - 解释：表示与当前突触连接的目标神经元的标识符或索引。

2. **weight**:
   - 类型：`float`
   - 解释：表示这个突触的权重。用来表示信号在这个突触上传递时的强度或影响。权重值通常用于计算信号从一个神经元传递到另一个神经元时的调制效果。通过论文中Hebbian plasticity来改变

*使用场景*

`Synapse` 结构体通常用于模拟神经网络中的突触连接。在神经网络的实现中，每个神经元可以通过一个或多个突触连接到其他神经元。每个突触都有一个权重，用于调节信号传递的强度。

#### 1.1.2 Area

`Area`表示一个脑区。

```cpp
struct Area {
    // 默认构造函数
    Area() : index(0), n(0), k(0), support(0), is_fixed(false) {}

    // 参数化构造函数
    Area(uint32_t index, uint32_t n, uint32_t k) 
        : index(index), n(n), k(k), support(0), is_fixed(false) {}

    const uint32_t index;
    const uint32_t n;
    const uint32_t k;
    uint32_t support;
    bool is_fixed;
    std::vector<uint32_t> activated;
};
```

基于论文，参数的解释：

1. `index`：脑区编号
2. `n`：当前脑区的神经元数
3. `k`：当前脑区assembly的神经元数
4. `support`：⼀个脑区的⽀持集定义为在所有时间步中曾经活跃过的神经元集的并集。因此，support表示曾经活跃过的神经元总数
5. `is_fixed`：bool flag，标定一个脑区的assembly是否固定

6. `activated`：`vector<int>`类型，表明激活的神经元编号。方便用于计算，一个脑区中的cap(We call the set of k neurons in an area firing at time t the cap of that area.)

#### 1.1.3 Fiber

Fiber 在论文中的定义是用于连接两个脑区的，它也是和神经元一样，可以inhibit或者disinhibit.

```cpp
struct Fiber
{
    Fiber(uint32_t from, uint32_t to) : from_area(from), to_area(to) {}

    const uint32_t from_area;
    const uint32_t to_area;
    bool is_active = true;
    std::vector<std::vector<Synapse>> outgoing_synapses;
};
```

基于论文，参数的解释：

1. `from_area`: 源脑区
2. `to_area`：目标脑区
3. `is_activate`：当前是否激活
4. `outgoing_synapses`：从源脑区到目标脑区之间，神经元的突触连接

#### 1.1.4 Brain

```cpp
class Brain
{
public:
    Brain(float p, float beta, float max_weight, uint32_t seed);

    Area &AddArea(const std::string &name, uint32_t n, uint32_t k,
                  bool recurrent = true, bool is_explicit = false);
    void AddStimulus(const std::string &name, uint32_t k);
    void AddFiber(const std::string &from, const std::string &to,
                  bool bidirectional = false);

    Area &GetArea(const std::string &name);
    const Area &GetArea(const std::string &name) const;
    Fiber &GetFiber(const std::string &from, const std::string &to);
    const Fiber &GetFiber(const std::string &from, const std::string &to) const;

    void InhibitAll();
    void InhibitFiber(const std::string &from, const std::string &to);
    void ActivateFiber(const std::string &from, const std::string &to);
    void InitProjection(const ProjectMap &graph);

    void ActivateArea(const std::string &name, uint32_t assembly_index);

    void SimulateOneStep(bool update_plasticity = true);
    void Project(const ProjectMap &graph, uint32_t num_steps,
                 bool update_plasticity = true);

    void ReadAssembly(const std::string &name, size_t &index, size_t &overlap);

    void SetLogLevel(int log_level) { log_level_ = log_level; }
    void LogGraphStats();
    void LogActivated(const std::string &area_name);

private:
    void ComputeKnownActivations(const Area &to_area,
                                 std::vector<Synapse> &activations);
    void GenerateNewCandidates(const Area &to_area, uint32_t total_k,
                               std::vector<Synapse> &activations);
    void ConnectNewNeuron(Area &area,
                          uint32_t num_synapses_from_activated,
                          uint32_t &total_synapses_from_non_activated);
    void ChooseSynapsesFromActivated(const Area &area,
                                     uint32_t num_synapses);
    void ChooseSynapsesFromNonActivated(const Area &area,
                                        uint32_t &total_synapses);
    void ChooseOutgoingSynapses(const Area &area);
    void UpdatePlasticity(Area &to_area,
                          const std::vector<uint32_t> &new_activated);

protected:
    std::mt19937 rng_;
    int log_level_ = 0;

    const float p_;
    const float beta_;
    const float learn_rate_;
    const float max_weight_;
    std::vector<Area> areas_;
    std::vector<Fiber> fibers_;
    std::vector<std::vector<uint32_t>> incoming_fibers_;
    std::vector<std::vector<uint32_t>> outgoing_fibers_;
    std::map<std::string, uint32_t> area_by_name_;
    std::vector<std::string> area_name_;
    uint32_t step_ = 0;
};
```

基于论文，**参数解释**：

- `rng_`：随机种子
- `log_level_`：用于log记录
- `p_`：随机图，边连接概率
- `beta_`：Hebbian plasticity，如果存在突触连接的神经元，源节点在时间步t兴奋(fired),⽬标节点在时间步t + 1兴奋，则⼆者之间的权重增加为原来的1 + β
- `learn_rate_`：1+beta_
- `max_weight_`：即突触最大权值，源论文没有细说，我在初始化时将其设为无穷大
- `areas_`：Brain的脑区，数据类型为`std::vector<Area>`
- `fibers_`：脑区间的fiber，`std::vector<Fiber>`
- `incoming_fibers_`：fiber是有向的，表示输入的脑区
- `outgoing_fibers_`：fiber是有向的，表示输出的脑区
- `area_by_name_`：数据类型为`std::map<std::string, uint32_t>`，哈希表，脑区名（例如，LEX）映射到脑区的编号
- `area_name_`：脑区名，例如LEX
- `step_`：Project\*的step

### 1.2 parserbrain

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

