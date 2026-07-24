# Compile Instructions

## Visual Studio

1. Clone the repo, including submodules.
    - If you don't, the build will fail. `git clone --recurse-submodules https://github.com/mclemp/MCLEClient.git`
2. Open the repo folder in Visual Studio 2022+.
3. Wait for cmake to configure the project and load all assets (this may take a few minutes on the first run).
4. Right click a folder in the solution explorer and switch to the 'CMake Targets View'
5. Select platform and configuration from the dropdown. EG: `Windows64 - Release`
6. Pick the startup project `Minecraft.Client.exe`

## CMake (Windows x64)

Configure (use your VS Community instance explicitly):

Open `Developer PowerShell for VS` and run:

```powershell
cmake --preset windows64
```

Build Release:

```powershell
cmake --build --preset windows64-release --target Minecraft.Client
```

Run executable:

```powershell
cd .\build\windows64\Minecraft.Client\Debug
.\Minecraft.Client.exe
```

Notes:
- Post-build asset copy is automatic for `Minecraft.Client` in CMake (Debug and Release variants).
- The game relies on relative paths (for example `Common\Media\...`), so launching from the output directory is required.

## CMake (Linux x64 Cross-Compile with Clang)

Cross-compile Windows x64 binaries on Linux using LLVM/Clang and the Windows SDK obtained via xwin.

### Prerequisites

Install the following packages (example for Ubuntu):

```bash
sudo apt install clang lld llvm cmake ninja-build rsync cargo
```

Install xwin for downloading the Windows SDK:

```bash
cargo install xwin
```

### Download Windows SDK

Download and extract the Windows SDK and CRT:

```bash
xwin --accept-license splat --output ~/.cache/xwin/splat
```

Create symlinks to account for Linux filesystems being case sensitive:

```bash
WINSDK=~/.cache/xwin/splat
ln -sf $WINSDK/sdk/include/shared/sdkddkver.h $WINSDK/sdk/include/shared/SDKDDKVer.h
ln -sf $WINSDK/sdk/lib/um/x86_64/xinput9_1_0.lib $WINSDK/sdk/lib/um/x86_64/XInput9_1_0.lib
ln -sf $WINSDK/sdk/lib/um/x86_64/ws2_32.lib $WINSDK/sdk/lib/um/x86_64/Ws2_32.lib
```

### Configure

Set environment variables and configure CMake:

```bash
export WINSDK=~/.cache/xwin/splat
export INCLUDE="$WINSDK/crt/include;$WINSDK/sdk/include/um;$WINSDK/sdk/include/ucrt;$WINSDK/sdk/include/shared"
export LIB="$WINSDK/crt/lib/x86_64;$WINSDK/sdk/lib/um/x86_64;$WINSDK/sdk/lib/ucrt/x86_64"

cmake -S . -B build/windows64-clang \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang-cl \
  -DCMAKE_CXX_COMPILER=clang-cl \
  -DCMAKE_LINKER=lld-link \
  -DCMAKE_RC_COMPILER=llvm-rc \
  -DCMAKE_MT=llvm-mt \
  -DPLATFORM_DEFINES="_WINDOWS64" \
  -DPLATFORM_NAME="Windows64" \
  -DIGGY_LIBS="iggy_w64.lib;iggyperfmon_w64.lib;iggyexpruntime_w64.lib" \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded \
  -DCMAKE_C_FLAGS="/MT -fms-compatibility -fms-extensions --target=x86_64-pc-windows-msvc -imsvc $WINSDK/crt/include -imsvc $WINSDK/sdk/include/ucrt -imsvc $WINSDK/sdk/include/um -imsvc $WINSDK/sdk/include/shared" \
  -DCMAKE_CXX_FLAGS="/MT -fms-compatibility -fms-extensions --target=x86_64-pc-windows-msvc -imsvc $WINSDK/crt/include -imsvc $WINSDK/sdk/include/ucrt -imsvc $WINSDK/sdk/include/um -imsvc $WINSDK/sdk/include/shared" \
  -DCMAKE_ASM_MASM_FLAGS="-m64" \
  -DCMAKE_EXE_LINKER_FLAGS="-libpath:$WINSDK/crt/lib/x86_64 -libpath:$WINSDK/sdk/lib/um/x86_64 -libpath:$WINSDK/sdk/lib/ucrt/x86_64"
```

### Build

Build Release:

```bash
cmake --build build/windows64-clang --config Release
```

Build specific target:

```bash
cmake --build build/windows64-clang --config Release --target Minecraft.Client
```

### Run with Wine

Run executable:

```bash
cd build/windows64-clang/Minecraft.Client
wine ./Minecraft.Client.exe
```

Notes:
- Requires LLVM 15+ with clang-cl, lld-link, llvm-rc, and llvm-mt.
- The xwin tool downloads ~1GB of SDK files on first run.
- Wine is required to run the compiled Windows executables on Linux.
