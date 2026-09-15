# txtco

一个轻量级的 C++ 命令行文本收集器。它扫描指定目录下符合扩展名的文本文件，将它们合并为一个输出文件，或直接复制到剪贴板。

---

## 功能特性

- 递归遍历目录（可选）
- 按扩展名过滤文件（支持多值）
- 排除指定目录或文件（支持多值）
- 输出到文件或 Windows 剪贴板
- UTF-8 输出并带 BOM，兼容 Windows 记事本
- 内置 `help` 命令，可查看所有命令或单个命令的参数说明
- 可扩展的命令行框架，内置类型擦除的参数系统

---

## 构建

**环境要求**：CMake ≥ 3.10，C++17 编译器（MinGW-w64 或 MSVC），Windows（剪贴板支持）。

```bash
git clone https://github.com/你的用户名/txtco.git
cd txtco
cmake -S . -B build
cmake --build build
```

生成的可执行文件位于 `app/txtco.exe`。

---

## 用法

```bash
txtco <命令> [参数...]
```

### 命令

| 命令 | 说明 |
| :--- | :--- |
| `txtco` | 从目录中收集文本文件 |
| `help` | 显示所有命令或某个命令的帮助信息 |

### `txtco` 命令的参数

| 参数 | 取值 | 说明 | 默认值 |
| :--- | :--- | :--- | :--- |
| `-dir` | 路径 | 要扫描的根目录 | `.` |
| `-recursive` | `true` / `false` | 是否递归进入子目录 | `false` |
| `-exclude_dir` | 路径（可多个） | 排除的目录（多值） | — |
| `-exclude_file` | 路径（可多个） | 排除的文件（多值） | — |
| `-format` | 扩展名（可多个） | 要收集的文件扩展名（必需，多值） | — |
| `-o` | 路径 | 输出目录 | `.` |
| `-o_code` | `UTF-8` / `GBK` / `UTF-16` | 输出编码 | `UTF-8` |
| `-clipboard` | `true` / `false` | 将结果复制到剪贴板而非写入文件 | `false` |

**注意**：`-exclude_dir` 和 `-exclude_file` 中的相对路径基于 `-dir` 解析；绝对路径直接使用。

### `help` 命令的参数

| 参数 | 取值 | 说明 | 默认值 |
| :--- | :--- | :--- | :--- |
| `-which` | 命令名或 `all` | 要显示帮助的命令名，`all` 表示显示全部 | `all` |

### 示例

```bash
# 显示所有命令和它们的参数
txtco help

# 只显示 txtco 命令的帮助
txtco help -which txtco

# 收集上级目录下所有 .cpp 和 .h 文件到剪贴板
txtco txtco -dir .. -format .cpp .h -clipboard true

# 递归收集 D:\docs 下的 .txt 文件，排除 archive 和 build 目录
txtco txtco -dir D:\docs -recursive true -format .txt -exclude_dir archive build

# 使用 UTF-8 编码将结果写入 D:\out
txtco txtco -dir . -format .md -o D:\out -o_code UTF-8
```

### 输出格式

每个被收集的文件按以下格式写入：

```
[绝对/文件/路径]
<文件内容>
```

文件之间以空行分隔。

---

## 命令行架构

命令行模块由三层组成：

```
┌──────────────────────────────────────────────────────────┐
│  main.cpp                                                │
│  ─ 将 argv[1] 解析为命令名                                │
│  ─ 在 cmd_dict 中查找命令                                 │
│  ─ 调用 injectArgs() 然后调用 operator()                  │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  command（基类）                                          │
│  ─ key / doc：命令元数据                                  │
│  ─ dict：arg_dict（将参数键字符串映射到 argument_base*）   │
│  ─ injectArgs()：解析 argv 并分发到各个参数               │
│  ─ operator()：纯虚函数，由各命令实现                     │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  argument<T>（继承自 argument_base）                      │
│  ─ arg / defaultArg：强类型存储                           │
│  ─ converter：std::function<int(T&, int, char*[])>        │
│  ─ convert()：用剩余参数调用 converter                    │
└──────────────────────────────────────────────────────────┘
```

### 核心类型

| 类型 | 作用 |
| :--- | :--- |
| `argument_base` | 为所有参数类型提供类型擦除接口，持有 `doc` 字符串 |
| `argument<T>` | 持有类型 T 值的模板参数 |
| `arg_dict` | `ref_dict<std::string, argument_base>` —— 将参数键映射到参数对象 |
| `cmd_dict` | `ref_dict<std::string, command>` —— 将命令名映射到命令对象 |
| `my::ref_dict<K, V>` | 一个小型字典，存储指向 V 的非拥有指针 |

### 参数转换器协议

每个 `argument<T>` 绑定到一个签名如下的转换器函数：

```cpp
int converter(T& dst, int argc, char* argv[]);
```

- `dst` —— 要写入的值
- `argc` —— 该参数键之后的剩余 token 数量
- `argv` —— 指向该参数键之后第一个 token 的指针

转换器**消费**若干 token 并返回消费的数量。`injectArgs()` 用返回值推进游标。

如果转换器抛出异常，`injectArgs()` 会调用 `dict.restore_default()` 把所有参数恢复为默认值，然后重新抛出。

---

## 扩展

### 添加新命令

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

### 添加新参数类型

1. 在 `converters.h` 中添加转换器：

```cpp
inline int to_int(int& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected an integer");
    dst = std::stoi(argv[0]);
    return 1;
}
```

2. 在命令类中将参数声明为成员：

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

### 转换器约定

| 约定 | 原因 |
| :--- | :--- |
| 单值转换器返回 `1` | 消费一个 token |
| 贪婪转换器在遇到下一个以 `-` 开头的 token 时停止 | 允许 `-format .cpp .h .hpp` |
| 输入非法时抛出 `std::invalid_argument` | 框架捕获、重置、重新抛出 |
| 转换器不得修改 `argv` | 调用方依赖其不可变性 |

---

## 设计说明

- **类型擦除**：`argument_base` 让不同类型的参数能存储在同一个 `arg_dict` 中，模板不会泄漏到 `command` 里。
- **参数自带文档**：`doc` 提升到 `argument_base`，使 `help` 命令能在遍历 `arg_dict` 时统一访问每个参数的说明，无需手写重复的帮助文本。
- **非拥有字典**：`ref_dict` 存储裸指针。命令拥有自己的参数，字典仅引用它们。这完全避免了堆分配。
- **快速失败解析**：任何参数转换器抛异常，整个解析都会中止并恢复默认值，保持命令对象状态一致。
- **内部统一使用 UTF-8**：源文件按字节读取，UTF-8 输出时带 BOM。未来计划支持 GBK/UTF-16 转码。

---

## 已知限制

- 仅支持 Windows 剪贴板（`clipboard.h` 在其他平台返回 `false`）。
- `-o_code` 目前只影响 BOM；尚未实现真正的 GBK/UTF-16 转码。
- 不检测输入文件编码——按字节读取。
- 默认不排除 `build/` 和 `.git/`；用户需显式传入 `-exclude_dir`。
- 参数名拼错时不会给出“你是不是想写……”的建议。

---

## 许可证

MIT License，详见 `LICENSE`。

---

## AI 使用说明

本项目由我本人设计与实现。AI 在其中作为编码助手，而非主要作者。具体分工如下：

**由我（作者）编写：**
- 所有架构决策：`argument_base` / `argument<T>` 的类型擦除设计、`ref_dict` 容器、`command` / `cmd_dict` 框架、`help` 命令与 `-which` 参数。
- `command/` 下的所有头文件及核心框架逻辑。
- `main.cpp`、`CMakeLists.txt` 及整体项目结构。
- 所有调试、集成与迭代，直到工具端到端跑通。
- README 的参数列表、用法示例与架构说明。

**在 AI 辅助下起草：**
- `command_txtco.cpp` —— 首个可用版本由 AI 根据我的规格起草；我随后审阅、修改并集成。
- `clipboard.h` —— Win32 剪贴板代码由 AI 作为参考起草；我进行了适配与验证。
- `converters.h` —— 转换器函数签名由我设计；具体实现结合了 AI 建议进行打磨。
- 本 README —— 结构与内容由我提供，AI 协助润色文字。

**由 AI 编写：**
- 无。本仓库的每个文件都由我审阅、修改并提交。

我认为对现代工具的使用方式保持透明是必要的。AI 帮我提速，但项目是我的。