# txtco

A lightweight C++ command-line text collector. It scans a directory for text files matching specified extensions, concatenates them into a single output file, or copies the result straight to the clipboard.

---

## Features

- Recursively traverse a directory (optional)
- Filter files by extension (multi-value)
- Exclude specific directories or files (multi-value)
- Output to a file or the Windows clipboard
- UTF-8 output with BOM for Windows Notepad compatibility
- Built-in `help` command for browsing commands and their arguments
- Extensible command-line framework with a type-erased argument system

---

## Building

**Requirements**: CMake ≥ 3.10, a C++17 compiler (MinGW-w64 or MSVC), Windows (clipboard support).

```bash
git clone https://github.com/your-username/txtco.git
cd txtco
cmake -S . -B build
cmake --build build
```

The executable is generated at `app/txtco.exe`.

---

## Usage

```bash
txtco <command> [arguments...]
```

### Commands

| Command | Description |
| :--- | :--- |
| `txtco` | Collect text files from a directory |
| `help` | Show help for all commands or a specific one |

### `txtco` Arguments

| Flag | Value | Description | Default |
| :--- | :--- | :--- | :--- |
| `-dir` | path | Root directory to scan | `.` |
| `-recursive` | `true` / `false` | Recurse into subdirectories | `false` |
| `-exclude_dir` | path(s) | Directories to skip (multi-value) | — |
| `-exclude_file` | path(s) | Files to skip (multi-value) | — |
| `-format` | extension(s) | File extensions to collect (required, multi-value) | — |
| `-o` | path | Output directory | `.` |
| `-o_code` | `UTF-8` / `GBK` / `UTF-16` | Output encoding | `UTF-8` |
| `-clipboard` | `true` / `false` | Copy result to clipboard instead of writing a file | `false` |

**Note**: Relative paths passed to `-exclude_dir` and `-exclude_file` are resolved against `-dir`. Absolute paths are used as-is.

### `help` Arguments

| Flag | Value | Description | Default |
| :--- | :--- | :--- | :--- |
| `-which` | command name or `all` | Command to show help for; `all` shows everything | `all` |

### Examples

```bash
# Show every command and its arguments
txtco help

# Show help for the txtco command only
txtco help -which txtco

# Collect all .cpp and .h files from the parent directory into the clipboard
txtco txtco -dir .. -format .cpp .h -clipboard true

# Recursively collect .txt files from D:\docs, excluding archive and build
txtco txtco -dir D:\docs -recursive true -format .txt -exclude_dir archive build

# Write the result to D:\out with UTF-8 encoding
txtco txtco -dir . -format .md -o D:\out -o_code UTF-8
```

### Output Format

Each collected file is written as:

```
[absolute/file/path]
<file content>
```

with a blank line separating files.

---

## Command-Line Architecture

The command-line module is built on three layers:

```
┌──────────────────────────────────────────────────────────┐
│  main.cpp                                                │
│  ─ Parses argv[1] as the command name                    │
│  ─ Looks up the command in cmd_dict                      │
│  ─ Calls injectArgs() then operator()                    │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  command  (base class)                                   │
│  ─ key / doc: command metadata                           │
│  ─ dict: arg_dict (maps flag strings → argument_base*)   │
│  ─ injectArgs(): parses argv and dispatches to arguments │
│  ─ operator(): pure virtual, implemented by each command │
└──────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────┐
│  argument<T>  (derived from argument_base)               │
│  ─ arg / defaultArg: strongly-typed storage              │
│  ─ converter: std::function<int(T&, int, char*[])>       │
│  ─ convert(): calls the converter with remaining args    │
└──────────────────────────────────────────────────────────┘
```

### Key Types

| Type | Role |
| :--- | :--- |
| `argument_base` | Type-erased interface for all argument types; also holds the `doc` string |
| `argument<T>` | Templated argument holding a value of type `T` |
| `arg_dict` | `ref_dict<std::string, argument_base>` — maps flag → argument |
| `cmd_dict` | `ref_dict<std::string, command>` — maps command name → command |
| `my::ref_dict<K, V>` | A small dictionary that stores non-owning pointers to `V` |

### Argument Converter Protocol

Each `argument<T>` is bound to a converter function with the signature:

```cpp
int converter(T& dst, int argc, char* argv[]);
```

- `dst` — the value to write into
- `argc` — number of remaining tokens after this flag
- `argv` — pointer to the first token after this flag

The converter **consumes** some number of tokens and returns that count. `injectArgs()` uses the return value to advance its cursor.

If a converter throws, `injectArgs()` calls `dict.restore_default()` to reset every argument to its default value, then re-throws.

---

## Extending

### Adding a New Command

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

### Adding a New Argument Type

1. Add a converter to `converters.h`:

```cpp
inline int to_int(int& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected an integer");
    dst = std::stoi(argv[0]);
    return 1;
}
```

2. Declare the argument as a member of your command:

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

### Converter Conventions

| Convention | Rationale |
| :--- | :--- |
| Single-value converters return `1` | Consumes one token |
| Greedy converters stop at the next `-`-prefixed token | Allows `-format .cpp .h .hpp` |
| Throw `std::invalid_argument` on bad input | Framework catches, resets, re-throws |
| Converters must not modify `argv` | Callers rely on it |

---

## Design Notes

- **Type erasure**: `argument_base` allows differently-typed arguments to be stored in a single `arg_dict`. No templates leak into `command`.
- **Self-documenting arguments**: `doc` lives on `argument_base`, so the `help` command can walk any `arg_dict` and print each argument's description without maintaining a separate help text.
- **Non-owning dictionary**: `ref_dict` stores raw pointers. The commands own their arguments; the dict merely references them. This avoids heap allocation entirely.
- **Fail-fast parsing**: If any argument converter throws, the entire parse is aborted and defaults are restored. This keeps the command object in a consistent state.
- **UTF-8 everywhere internally**: Source file bytes are read as-is and written with a BOM for UTF-8 output. Future support for GBK/UTF-16 conversion is planned.

---

## Known Limitations

- Only supports Windows clipboard (`clipboard.h` returns `false` on other platforms).
- `-o_code` currently only affects the BOM; actual transcoding to GBK/UTF-16 is not yet implemented.
- Input file encoding is not detected — files are read byte-for-byte.
- `build/` and `.git/` are not excluded by default; users must pass `-exclude_dir` explicitly.
- Misspelled flags do not yet produce "did you mean…?" suggestions.

---

## License

MIT License. See `LICENSE` for details.

---

## AI Assistance

This project is the result of my own design and implementation. AI was used as a coding assistant, not as the primary author. The breakdown is as follows:

**Written by me (the author):**
- All architectural decisions: the `argument_base` / `argument<T>` type-erasure design, the `ref_dict` container, the `command` / `cmd_dict` framework, and the `help` command with its `-which` argument.
- All header files and the core framework logic in `command/`.
- `main.cpp`, `CMakeLists.txt`, and the overall project structure.
- Debugging, integration, and iteration on the tool until it worked end-to-end.
- The README's parameter list, usage examples, and architecture description.

**Drafted with AI assistance:**
- `command_txtco.cpp` — the first working version was drafted by AI based on my specification; I then reviewed, modified, and integrated it.
- `clipboard.h` — the Win32 clipboard code was drafted by AI as a reference; I adapted and verified it.
- `converters.h` — the converter function signatures were designed by me; the specific implementations were refined with AI suggestions.
- This README — I provided the structure and content; AI polished the wording.

**Written by AI:**
- Nothing. Every file in this repository was reviewed, edited, and committed by me.

I believe in transparency about how modern tools are used. AI helped me move faster, but the project is mine.