# Portable LCE

---

This project is a heavily modified version of the Minecraft Console Legacy Edition codebase, aimed at porting old Minecraft (TU19/1.6.1) to different platforms and refactoring the codebase to improve organization and use modern C++ features.

## Status

| - | - | - | - |
| **app** | `desktop` | `desktop` | `desktop` |
| **ui** | `java`, `shiggy`[^2] | `java`, `shiggy`[^2] | `java` |
| **fs** | `std` | `std` | `std` |
| **renderer** | `gl` | `gl` | `gl` |
| **sound** | `miniaudio` | `miniaudio` | `miniaudio` |
| **input** | `sdl2` | `sdl2` | `sdl2` |
| **thread** | `std` | `std` | `std` |
| **game** | `stub` | `stub` | `stub` |
| **network** | `stub` | `stub` | `stub` |
| **storage** | `stub` | `stub` | `stub` |
| **profile** | `stub` | `stub` | `stub` |
| **leaderboard** | `stub` | `stub` | `stub` |

[^2]: `-Dui_backend=shiggy` supports the x86-64 architecture with AVX2 extensions only.


## Building (Linux)

### Prerequisites

#### System Libraries

Debian/Ubuntu:
```bash
sudo apt-get install -y build-essential libsdl2-dev libgl-dev libglu1-mesa-dev libpthread-stubs0-dev
```

Arch/Manjaro:
```bash
sudo pacman -S base-devel pkgconf sdl2-compat mesa glu
```

Fedora/Red Hat/Nobara:
```bash
sudo dnf install gcc gcc-c++ make SDL2-devel mesa-libGL-devel mesa-libGLU-devel openssl-devel
```

#### Toolchain

This project requires a C++23 compiler with full standard library support.

**If your distro ships GCC 15+**, you're good - just use the system compiler:

```bash
meson setup build
```

**If your distro ships an older GCC:** install LLVM with libc++ and use the provided toolchain file:

```bash
# Debian/Ubuntu
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20
sudo apt install libc++-20-dev libc++abi-20-dev
```

```bash
# Fedora/RHEL (if needed)
sudo dnf install clang lld libcxx-devel libcxxabi-devel
```

Then configure with the LLVM native file (see Configure & Build below).

#### Meson + Ninja

Install [Meson](https://mesonbuild.com/) and [Ninja](https://ninja-build.org/):

```bash
pip install meson ninja
```

Or follow the [Meson quickstart guide](https://mesonbuild.com/Quick-guide.html).


### Configure & Build

```bash
# If using system GCC 15+
meson setup build

meson configure build -Dui_backend=java -Drenderer=gles

# If using LLVM/libc++
meson setup --native-file ./scripts/llvm_native.txt build

# Compile
meson compile -C build
```

The binary is output to:

```
./build/targets/app/Minecraft.Client
```

#### Clean

To perform a clean compilation:

```bash
meson compile --clean -C build
```

...or to reconfigure an existing build directory:

```bash
meson setup --native-file ./scripts/llvm_native.txt build --reconfigure
```

...or to hard reset the build directory:

```bash
rm -r ./build
meson setup --native-file ./scripts/llvm_native.txt build
```

---

## Running

Game assets are automatically copied to the build output directory during compilation. Run from that directory:

```sh
./build/targets/app/Minecraft.Client
```

