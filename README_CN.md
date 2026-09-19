# txtco : 轻量级文本收集器

还在为向你的 AI 助手上传多文件、多层路径项目而苦恼吗？？试试 txtco —— 一款轻量级命令行文本收集器！

仅需下载 `.exe` 文件并添加到 PATH（建议）即可使用。

txtco 支持递归搜索、按后缀名搜索、按关键字搜索、排除某个文件夹、排除某些文件、复制到剪贴板或者输出结果到指定路径。

## 快速上手

建议：把 `txtco.exe` 放在某个路径下后，将该路径添加到系统环境变量的 PATH 中。这样方便你在任何命令行窗口中都可以调用 txtco。

在命令行窗口中：

- 如果你已添加到 PATH：输入 `txtco help` 获取帮助信息。
- 如果还未添加到 PATH：输入 `[你存放 txtco.exe 的路径]\txtco help` 获取帮助信息。


好了我想说的就这么多，以下内容由ai起草。

---

## 1. 具体命令及参数

### 命令总览

```bash
txtco <命令> [参数...]
```

| 命令 | 说明 |
| :--- | :--- |
| `txtco` | 从一个或多个目录中收集文本文件 |
| `help` | 显示所有命令或某个命令的帮助信息 |

### `txtco` 命令的参数

| 参数 | 取值 | 说明 | 默认值 |
| :--- | :--- | :--- | :--- |
| `-dir` | 路径（可多个） | 要扫描的根目录（多值） | `.`（当前目录） |
| `-recursive` | `true` / `false` | 是否递归进入子目录 | `false` |
| `-exclude_dir` | 路径（可多个） | 排除的目录，相对第一个 `-dir` 解析（多值） | — |
| `-exclude_file` | 路径（可多个） | 排除的文件，相对第一个 `-dir` 解析（多值） | — |
| `-format` | 扩展名（可多个） | 要收集的文件扩展名（**必需**，多值） | — |
| `-keyword` | 子串（可多个） | 只收集文件名（含后缀）包含任一子串的文件，**不区分大小写**（多值） | — |
| `-o` | 路径 | 输出目录 | `.`（当前目录） |
| `-o_code` | `UTF-8` / `GBK` / `UTF-16` | 输出编码 | `UTF-8` |
| `-clipboard` | `true` / `false` | 将结果复制到剪贴板而非写入文件 | `false` |

**多值参数**：`-dir`、`-exclude_dir`、`-exclude_file`、`-format`、`-keyword` 支持一次传多个值，用空格隔开即可。例如 `-format .cpp .h .hpp`。

### `help` 命令的参数

| 参数 | 取值 | 说明 | 默认值 |
| :--- | :--- | :--- | :--- |
| `-which` | 命令名或 `all` | 要显示帮助的命令名，`all` 表示显示全部 | `all` |

### 输出格式

每个被收集的文件按以下格式写入：

```
[绝对/文件/路径]
<文件内容>

[绝对/文件/路径]
<文件内容>
```

### 使用示例

```bash
# 查看所有命令及其参数
txtco help

# 只查看 txtco 命令的帮助
txtco help -which txtco

# 收集当前目录下的所有 .cpp 和 .h 文件到剪贴板
txtco txtco -format .cpp .h -clipboard true

# 递归收集 D:\project 下的 .cpp/.h，排除 build 和 .git 目录
txtco txtco -dir D:\project -recursive true -format .cpp .h -exclude_dir build .git -clipboard true

# 一次收集多个目录，重叠部分自动去重
txtco txtco -dir src lib ../shared -format .cpp .h -clipboard true

# 只收集文件名含 "test" 或 "spec" 的文件（不区分大小写）
txtco txtco -dir . -format .cpp -keyword test spec

# 输出到 D:\out
txtco txtco -dir . -format .md -o D:\out -o_code UTF-8
```

---

## 2. 内部实现及拓展示例

txtco 不只是一个文本收集器，它还内置了一个**可扩展的命令行框架**。`txtco` 命令本身只是这个框架上的一个插件。

### 架构总览

```
┌──────────────────────────────────────────────────────────┐
│  main.cpp                                                │
│  ─ 解析 argv[1] 作为命令名                                |
│  ─ 在 cmd_dict 中查找命令                                 │
│  ─ 调用 injectArgs() 然后调用 operator()                  │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  command（命令基类）                                      │
│  ─ key / doc：命令元数据                                  │
│  ─ dict：arg_dict（参数键 → argument_base*）              │
│  ─ injectArgs()：解析 argv 并分发给各参数                  │
│  ─ operator()：纯虚函数，由各命令实现                      │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  argument<T>（参数模板，继承自 argument_base）             │
│  ─ arg / defaultArg：强类型存储                           │
│  ─ converter：std::function<int(T&, int, char*[])>       │
│  ─ convert()：用剩余 argv 调用转换器                       │
└──────────────────────────────────────────────────────────┘
```

### 核心类型

| 类型 | 作用 |
| :--- | :--- |
| `argument_base` | 类型擦除接口，持有 `doc` 字符串 |
| `argument<T>` | 持有类型 `T` 的参数模板 |
| `arg_dict` | `ref_dict<std::string, argument_base>`，将参数键映射到参数对象 |
| `cmd_dict` | `ref_dict<std::string, command>`，将命令名映射到命令对象 |
| `my::ref_dict<K, V>` | 小型非拥有字典，存储指向 `V` 的指针 |

### 参数转换器协议

每个 `argument<T>` 绑定到一个转换器函数：

```cpp
int converter(T& dst, int argc, char* argv[]);
```

- `dst` —— 要写入的值
- `argc` —— 该参数键之后的剩余 token 数量
- `argv` —— 指向该参数键之后第一个 token 的指针

转换器**消费**若干 token 并返回消费数量。`injectArgs()` 用返回值推进游标。如果转换器抛异常，`injectArgs()` 会把所有参数恢复默认值，然后重新抛出。

### 扩展：添加新命令

1. 创建 `command` 的子类：

```cpp
// command_mycmd.h
#pragma once
#include "command.h"
#include "converters.h"

class command_mycmd : public command {
public:
    argument<int> count;    // -n

    command_mycmd();
    void operator()() override;
};
```

2. 注册参数并实现动作：

```cpp
// command_mycmd.cpp
#include "command_mycmd.h"

command_mycmd::command_mycmd()
    : command("mycmd", "My new command"),
      count(1, conv::to_int, "Number of times to run (default: 1)")
{
    dict.pair("-n", count);
}

void command_mycmd::operator()() {
    for (int i = 0; i < count.arg; ++i)
        std::cout << "Hello\n";
}
```

3. 在 `main.cpp` 中注册：

```cpp
cmd_dict dict;
command_txtco txtco;
command_help help(dict);
command_mycmd mycmd;
dict.add(txtco);
dict.add(help);
dict.add(mycmd);
```

### 扩展：添加新参数类型

1. 在 `converters.h` 中添加转换器：

```cpp
inline int to_int(int& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected an integer");
    dst = std::stoi(argv[0]);
    return 1;
}
```

2. 在命令类中声明成员：

```cpp
argument<int> count;
```

3. 在构造函数中绑定，并传入 doc 字符串：

```cpp
count(1, conv::to_int, "Number of times to run (default: 1)")
```

4. 注册：

```cpp
dict.pair("-n", count);
```

### 设计要点

- **类型擦除**：`argument_base` 让不同类型的参数能存储在同一个 `arg_dict` 中，模板不会泄漏到 `command`。
- **参数自带文档**：`doc` 提升到 `argument_base`，使 `help` 命令能遍历 `arg_dict` 统一打印说明。
- **非拥有字典**：`ref_dict` 存裸指针，命令拥有参数，字典仅引用，避免堆分配。
- **快速失败解析**：任何参数转换器抛异常，整个解析中止并恢复默认值。
- **贪婪消费**：多值转换器读到下一个以 `-` 开头的 token 停止，支持 `-format .cpp .h .hpp`。

---

## 3. AI 合作声明

本项目由我本人设计与实现。AI 在其中作为编码助手，而非主要作者。

**由我（作者）编写：**

- 所有架构决策：`argument_base` / `argument<T>` 类型擦除设计、`ref_dict` 容器、`command` / `cmd_dict` 框架、`help` 命令与 `-which` 参数。
- `command/` 下的所有头文件及核心框架逻辑。
- `main.cpp`、`CMakeLists.txt` 及整体项目结构。
- 所有调试、集成与迭代，直到工具端到端跑通。
- README 的参数列表、用法示例与架构说明。

**在 AI 辅助下起草：**

- `command_txtco.cpp` —— 首个可用版本由 AI 根据我的规格起草；我随后审阅、修改并集成（路径解析基准、多 `-dir` 去重、`-keyword` 大小写处理等均为我所改）。
- `clipboard.h` —— Win32 剪贴板代码由 AI 作为参考起草；我进行了适配与验证。
- `converters.h` —— 转换器函数签名由我设计；具体实现结合 AI 建议打磨。
- 本 README —— 结构与内容由我提供，AI 协助润色文字。

我认为对现代工具的使用方式保持透明是必要的。AI 帮我提速，但项目是我的。

---

## 许可证

MIT License，详见 `LICENSE`。