# 0.4 Squirrel

**VC:MP 0.4 plugin for scripting in Squirrel.**

0.4 Squirrel is a scripting plugin for **Vice City Multiplayer (VC:MP) 0.4**, providing integration between the VC:MP server and the **Squirrel scripting language**.

This project was originally created by **Stormeus** and released as part of the VC:MP 0.4 ecosystem:

https://bitbucket.org/stormeus/0.4-squirrel/

---

## Updated Version

This version includes improvements, maintenance updates, and support for the latest **VC:MP 0.4.7.1** features.

The original Squirrel module was not updated for several newer server features introduced in later VC:MP releases, leaving some functionality unavailable for Squirrel developers. This version updates the plugin to expose these improvements while maintaining compatibility with existing Squirrel scripts.

## Improvements

- Added compatibility with release 0.4.7.1.
- Updated support for newer plugin SDK features.

# Building

The project supports both **Windows** and **Linux**, with **x86 (32-bit)** and **x64 (64-bit)** targets.

---

# Windows

## Requirements

- Visual Studio 2026
- Desktop Development with C++
- Premake5

## Generate the Visual Studio solution

Run:

```bat
premake\win-build.bat
```

This generates the Visual Studio solution in the `build` directory.

## Build

1. Open:

```
build/SquirrelPlugin.sln
```

2. Select the desired configuration:

- **Debug** or **Release**
- **x86** or **x64**

3. Build the solution.

## Output files

Generated files are located in:

```
build/bin/x86/Debug/
build/bin/x86/Release/

build/bin/x64/Debug/
build/bin/x64/Release/
```

Plugin output names:

| Configuration | Output |
|--------------|--------|
| Debug x86 | `squirrel04dbg32.dll` |
| Release x86 | `squirrel04rel32.dll` |
| Debug x64 | `squirrel04dbg64.dll` |
| Release x64 | `squirrel04rel64.dll` |

---

# Linux

## Requirements

- Premake5
- GNU Make
- GCC or Clang with C++17 support

## Generate Makefiles

```bash
premake5 gmake
```

## Enter the build directory

```bash
cd build
```

## Build

### Release x64

```bash
make config=release_x64
```

### Debug x64

```bash
make config=debug_x64
```

### Release x86

```bash
make config=release_x86
```

### Debug x86

```bash
make config=debug_x86
```

## Output files

Generated files are located in:

```
build/bin/x86/Debug/
build/bin/x86/Release/

build/bin/x64/Debug/
build/bin/x64/Release/
```

Plugin output names:

| Configuration | Output |
|--------------|--------|
| Debug x86 | `squirrel04dbg32.so` |
| Release x86 | `squirrel04rel32.so` |
| Debug x64 | `squirrel04dbg64.so` |
| Release x64 | `squirrel04rel64.so` |

---

# Build Directory Structure

```
build/
├── bin/
│   ├── x86/
│   │   ├── Debug/
│   │   └── Release/
│   └── x64/
│       ├── Debug/
│       └── Release/
│
├── obj/
│   ├── squirrel/
│   ├── sqstdlib/
│   └── SquirrelPlugin/
│
├── SquirrelPlugin.sln      # Visual Studio
├── Makefile                # Linux
└── ...
```

## Notes

- All projects are compiled using **C++17**.
- `squirrel` and `sqstdlib` are built as static libraries and linked automatically into `SquirrelPlugin`.
- Windows builds link against `ws2_32`.
- Linux builds link against `pthread`, `dl`, and `m`.
- Linux shared libraries are generated without the `lib` prefix (e.g. `squirrel04rel64.so`).
- Windows Debug builds use the **Unicode** character set, while Release builds use **MBCS**, matching the original project configuration.
