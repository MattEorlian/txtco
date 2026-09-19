# txtco : A Lightweight Text Collector

(Translated by deepseek ai from README_CN.md)

Still struggling to upload multi-file, multi-directory projects to your AI assistant?? Try txtco — a lightweight command-line text collector!

Just download the `.exe` file and add it to PATH (recommended), and you're ready to use it.

txtco supports recursive search, search by extension, search by keyword, excluding a directory, excluding certain files, copying to clipboard, or outputting results to a specified path.

## Quick Start

Recommendation: Put `txtco.exe` in some directory, then add that directory to the system environment variable PATH. This makes it convenient to call txtco from any command-line window.

In a command-line window:

- If you have added it to PATH: type `txtco help` to get help information.
- If you have not added it to PATH: type `[path where you stored txtco.exe]\txtco help` to get help information.

That's all I wanted to say. The following content was drafted by AI.

---

## 1. Specific Commands and Arguments

### Command Overview

```bash
txtco <command> [arguments...]
```

| Command | Description |
| :--- | :--- |
| `txtco` | Collect text files from one or more directories |
| `help` | Show help for all commands or a specific one |

### Arguments for the `txtco` Command

| Flag | Value | Description | Default |
| :--- | :--- | :--- | :--- |
| `-dir` | path(s) | Root directories to scan (multi-value) | `.` (current directory) |
| `-recursive` | `true` / `false` | Recurse into subdirectories | `false` |
| `-exclude_dir` | path(s) | Directories to skip, relative to the first `-dir` (multi-value) | — |
| `-exclude_file` | path(s) | Files to skip, relative to the first `-dir` (multi-value) | — |
| `-format` | extension(s) | File extensions to collect (**required**, multi-value) | — |
| `-keyword` | substring(s) | Only collect files whose name (with extension) contains any of these substrings, **case-insensitive** (multi-value) | — |
| `-o` | path | Output directory | `.` (current directory) |
| `-o_code` | `UTF-8` / `GBK` / `UTF-16` | Output encoding | `UTF-8` |
| `-clipboard` | `true` / `false` | Copy result to clipboard instead of writing to a file | `false` |

**Multi-value arguments**: `-dir`, `-exclude_dir`, `-exclude_file`, `-format`, and `-keyword` accept multiple values separated by spaces. For example: `-format .cpp .h .hpp`.

### Arguments for the `help` Command

| Flag | Value | Description | Default |
| :--- | :--- | :--- | :--- |
| `-which` | command name or `all` | Command to show help for; `all` shows everything | `all` |

### Output Format

Each collected file is written as:

```
[absolute/file/path]
<file content>

[absolute/file/path]
<file content>
```

### Usage Examples

```bash
# Show all commands and their arguments
txtco help

# Show help for the txtco command only
txtco help -which txtco

# Collect all .cpp and .h files from the current directory into the clipboard
txtco txtco -format .cpp .h -clipboard true

# Recursively collect .cpp/.h from D:\project, excluding build and .git
txtco txtco -dir D:\project -recursive true -format .cpp .h -exclude_dir build .git -clipboard true

# Collect from multiple directories at once, with automatic deduplication
txtco txtco -dir src lib ../shared -format .cpp .h -clipboard true

# Only collect files whose name contains "test" or "spec" (case-insensitive)
txtco txtco -dir . -format .cpp -keyword test spec

# Output to D:\out
txtco txtco -dir . -format .md -o D:\out -o_code UTF-8
```

---

## 2. Internal Implementation and Extension Examples

txtco is not just a text collector — it also includes an **extensible command-line framework**. The `txtco` command itself is just a plugin on top of this framework.

### Architecture Overview

```
┌──────────────────────────────────────────────────────────┐
│  main.cpp                                                │
│  ─ Parse argv[1] as the command name                     │
│  ─ Look up command in cmd_dict                           │
│  ─ Call injectArgs() then operator()                     │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  command (base class)                                    │
│  ─ key / doc: command metadata                           │
│  ─ dict: arg_dict (flag → argument_base*)                │
│  ─ injectArgs(): parse argv and dispatch to arguments    │
│  ─ operator(): pure virtual, implemented by each command │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  argument<T> (template, inherits from argument_base)     │
│  ─ arg / defaultArg: strongly-typed storage              │
│  ─ converter: std::function<int(T&, int, char*[])>       │
│  ─ convert(): calls the converter with remaining argv    │
└──────────────────────────────────────────────────────────┘
```

### Key Types

| Type | Role |
| :--- | :--- |
| `argument_base` | Type-erased interface; holds the `doc` string |
| `argument<T>` | Templated argument holding a value of type `T` |
| `arg_dict` | `ref_dict<std::string, argument_base>` — maps flag to argument |
| `cmd_dict` | `ref_dict<std::string, command>` — maps command name to command |
| `my::ref_dict<K, V>` | Small non-owning dictionary storing pointers to `V` |

### Argument Converter Protocol

Each `argument<T>` is bound to a converter function:

```cpp
int converter(T& dst, int argc, char* argv[]);
```

- `dst` — the value to write into
- `argc` — number of remaining tokens after this flag
- `argv` — pointer to the first token after this flag

The converter **consumes** some number of tokens and returns that count. `injectArgs()` uses the return value to advance its cursor. If a converter throws, `injectArgs()` resets every argument to its default value, then re-throws.

### Extension: Adding a New Command

1. Create a subclass of `command`:

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

2. Register arguments and implement the action:

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

3. Register it in `main.cpp`:

```cpp
cmd_dict dict;
command_txtco txtco;
command_help help(dict);
command_mycmd mycmd;
dict.add(txtco);
dict.add(help);
dict.add(mycmd);
```

### Extension: Adding a New Argument Type

1. Add a converter to `converters.h`:

```cpp
inline int to_int(int& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected an integer");
    dst = std::stoi(argv[0]);
    return 1;
}
```

2. Declare the member in the command class:

```cpp
argument<int> count;
```

3. Bind it in the constructor, passing a doc string:

```cpp
count(1, conv::to_int, "Number of times to run (default: 1)")
```

4. Register it:

```cpp
dict.pair("-n", count);
```

### Design Notes

- **Type erasure**: `argument_base` allows differently-typed arguments to be stored in a single `arg_dict`; templates do not leak into `command`.
- **Self-documenting arguments**: `doc` is lifted to `argument_base`, enabling the `help` command to iterate over `arg_dict` and print descriptions uniformly.
- **Non-owning dictionary**: `ref_dict` stores raw pointers; commands own their arguments, the dictionary only references them, avoiding heap allocation.
- **Fail-fast parsing**: If any argument converter throws, the entire parse is aborted and defaults are restored.
- **Greedy consumption**: Multi-value converters stop at the next `-`-prefixed token, supporting `-format .cpp .h .hpp`.

---

## 3. AI Collaboration Statement

This project was designed and implemented by me. AI served as a coding assistant, not the primary author.

**Written by me (the author):**

- All architectural decisions: the `argument_base` / `argument<T>` type-erasure design, the `ref_dict` container, the `command` / `cmd_dict` framework, and the `help` command with its `-which` argument.
- All header files and the core framework logic under `command/`.
- `main.cpp`, `CMakeLists.txt`, and the overall project structure.
- All debugging, integration, and iteration until the tool worked end-to-end.
- The README's parameter list, usage examples, and architecture description.

**Drafted with AI assistance:**

- `command_txtco.cpp` — the first working version was drafted by AI based on my specification; I then reviewed, modified, and integrated it (path-resolution base, multi-`-dir` deduplication, `-keyword` case handling, etc. were my changes).
- `clipboard.h` — the Win32 clipboard code was drafted by AI as a reference; I adapted and verified it.
- `converters.h` — the converter function signatures were designed by me; implementations were refined with AI suggestions.
- This README — structure and content were provided by me; AI polished the wording.

I believe transparency about the use of modern tools is important. AI helped me move faster, but the project is mine.

---

## License

MIT License, see `LICENSE` for details.