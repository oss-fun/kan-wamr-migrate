
Build WAMR vmcore (iwasm)
=========================
It is recommended to use the [WAMR SDK](../wamr-sdk) tools to build a project that integrates the WAMR. This document introduces how to build the WAMR minimal product which is vmcore only (no app-framework and app-mgr) for multiple platforms.

## WAMR vmcore cmake building configurations

By including the script `runtime_lib.cmake` under folder [build-scripts](../build-scripts) in CMakeList.txt, it is easy to build minimal product with cmake.

```cmake
# add this into your CMakeList.txt
include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})
```

The script `runtime_lib.cmake` defines a number of variables for configuring the WAMR runtime features. You can set these variables in your CMakeList.txt or pass the configurations from cmake command line.

#### **Configure platform and architecture**

- **WAMR_BUILD_PLATFORM**:  set the target platform. It can be set to any platform name (folder name) under folder [core/shared/platform](../core/shared/platform).

- **WAMR_BUILD_TARGET**: set the target CPU architecture. Current supported targets are:  X86_64, X86_32, AARCH64, ARM, THUMB, XTENSA, RISCV64 and MIPS.
  - For ARM and THUMB, the format is \<arch>\[\<sub-arch>]\[_VFP], where \<sub-arch> is the ARM sub-architecture and the "_VFP" suffix means using VFP coprocessor registers s0-s15 (d0-d7) for passing arguments or returning results in standard procedure-call. Both \<sub-arch> and "_VFP" are optional, e.g. ARMV7, ARMV7_VFP, THUMBV7, THUMBV7_VFP and so on.
  - For AARCH64, the format is\<arch>[\<sub-arch>], VFP is enabled by default. \<sub-arch> is optional, e.g. AARCH64, AARCH64V8, AARCH64V8.1 and so on.
  - For RISCV64, the format is \<arch\>[_abi], where "_abi" is optional, currently the supported formats are RISCV64, RISCV64_LP64D and RISCV64_LP64: RISCV64 and RISCV64_LP64D are identical, using [LP64D](https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#-named-abis) as abi (LP64 with hardware floating-point calling convention for FLEN=64). And RISCV64_LP64 uses [LP64](https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#-named-abis) as abi (Integer calling-convention only, and hardware floating-point calling convention is not used).
  - For RISCV32, the format is \<arch\>[_abi], where "_abi" is optional, currently the supported formats are RISCV32, RISCV32_ILP32D and RISCV32_ILP32: RISCV32 and RISCV32_ILP32D are identical, using [ILP32D](https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#-named-abis) as abi (ILP32 with hardware floating-point calling convention for FLEN=64). And RISCV32_ILP32 uses [ILP32](https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#-named-abis) as abi (Integer calling-convention only, and hardware floating-point calling convention is not used).

```bash
cmake -DWAMR_BUILD_PLATFORM=linux -DWAMR_BUILD_TARGET=ARM
```

#### **Configure interpreter**

- **WAMR_BUILD_INTERP**=1/0: enable or disable WASM interpreter

- **WAMR_BUILD_FAST_INTERP**=1/0: build fast (default) or classic WASM interpreter.

  NOTE: the fast interpreter runs ~2X faster than classic interpreter, but consumes about 2X memory to hold the WASM bytecode code.

#### **Configure AoT and JIT**

- **WAMR_BUILD_AOT**=1/0, default to enable if not set
- **WAMR_BUILD_JIT**=1/0, default to disable if not set
- **WAMR_BUILD_LAZY_JIT**=1/0, default to disable if not set

#### **Configure LIBC**

- **WAMR_BUILD_LIBC_BUILTIN**=1/0, build the built-in libc subset for WASM app, default to enable if not set

- **WAMR_BUILD_LIBC_WASI**=1/0, build the [WASI](https://github.com/WebAssembly/WASI) libc subset for WASM app, default to enable if not set

- **WAMR_BUILD_LIBC_UVWASI**=1/0 (Experiment), build the [WASI](https://github.com/WebAssembly/WASI) libc subset for WASM app based on [uvwasi](https://github.com/nodejs/uvwasi) implementation, default to disable if not set

> Note: for platform which doesn't support **WAMR_BUILD_LIBC_WASI**, e.g. Windows, developer can try using **WAMR_BUILD_LIBC_UVWASI**. And the uvwasi source code must be cloned under core/deps:
>
> ```bash
> cd <WAMR-ROOT>/core/deps
> git clone https://github.com/nodejs/uvwasi.git
> ```

#### **Enable Multi-Module feature**

- **WAMR_BUILD_MULTI_MODULE**=1/0, default to disable if not set

#### **Enable WASM mini loader**

- **WAMR_BUILD_MINI_LOADER**=1/0, default to disable if not set

> Note: the mini loader doesn't check the integrity of the WASM binary file, developer must ensure that the WASM file is well-formed.

#### **Enable shared memory feature**
- **WAMR_BUILD_SHARED_MEMORY**=1/0, default to disable if not set

#### **Enable thread manager**
- **WAMR_BUILD_THREAD_MGR**=1/0, default to disable if not set

#### **Enable lib-pthread**
- **WAMR_BUILD_LIB_PTHREAD**=1/0, default to disable if not set
> Note: The dependent feature of lib pthread such as the `shared memory` and `thread manager` will be enabled automatically.

#### **Disable boundary check with hardware trap in AOT or JIT mode**
- **WAMR_DISABLE_HW_BOUND_CHECK**=1/0, default to enable if not set and supported by platform
> Note: by default only platform linux/darwin/android/vxworks 64-bit will enable boundary check with hardware trap in AOT or JIT mode, and the wamrc tool will generate AOT code without boundary check instructions in all 64-bit targets except SGX to improve performance.

#### **Enable tail call feature**
- **WAMR_BUILD_TAIL_CALL**=1/0, default to disable if not set

#### **Enable 128-bit SIMD feature**
- **WAMR_BUILD_SIMD**=1/0, default to enable if not set
> Note: only supported in AOT mode x86-64 target.

#### **Configure Debug**

- **WAMR_BUILD_CUSTOM_NAME_SECTION**=1/0, load the function name from custom name section, default to disable if not set

#### **Enable dump call stack feature**
- **WAMR_BUILD_DUMP_CALL_STACK**=1/0, default to disable if not set

> Note: if it is enabled, the call stack will be dumped when exception occurs.

> - For interpreter mode, the function names are firstly extracted from *custom name section*, if this section doesn't exist or the feature is not enabled, then the name will be extracted from the import/export sections
> - For AoT/JIT mode, the function names are extracted from import/export section, please export as many functions as possible (for `wasi-sdk` you can use `-Wl,--export-all`) when compiling wasm module, and add `--enable-dump-call-stack` option to wamrc during compiling AoT module.

#### **Enable memory profiling (Experiment)**
- **WAMR_BUILD_MEMORY_PROFILING**=1/0, default to disable if not set
> Note: if it is enabled, developer can use API `void wasm_runtime_dump_mem_consumption(wasm_exec_env_t exec_env)` to dump the memory consumption info.
Currently we only profile the memory consumption of module, module_instance and exec_env, the memory consumed by other components such as `wasi-ctx`, `multi-module` and `thread-manager` are not included.

#### **Enable performance profiling (Experiment)**
- **WAMR_BUILD_PERF_PROFILING**=1/0, default to disable if not set
> Note: if it is enabled, developer can use API `void wasm_runtime_dump_perf_profiling(wasm_module_inst_t module_inst)` to dump the performance consumption info. Currently we only profile the performance consumption of each WASM function.

> The function name searching sequence is the same with dump call stack feature.

#### **Set maximum app thread stack size**
- **WAMR_APP_THREAD_STACK_SIZE_MAX**=n, default to 8 MB (8388608) if not set
> Note: the AOT boundary check with hardware trap mechanism might consume large stack since the OS may lazily grow the stack mapping as a guard page is hit, we may use this configuration to reduce the total stack usage, e.g. -DWAMR_APP_THREAD_STACK_SIZE_MAX=131072 (128 KB).

#### **WAMR_BH_VPRINTF**=<vprintf_callback>, default to disable if not set
> Note: if the vprintf_callback function is provided by developer, the os_printf() and os_vprintf() in Linux, Darwin, Windows and VxWorks platforms, besides WASI Libc output will call the callback function instead of libc vprintf() function to redirect the stdout output. For example, developer can define the callback function like below outside runtime lib:
>
> ```C
> int my_vprintf(const char *format, va_list ap)
> {
>     /* output to pre-opened file stream */
>     FILE *my_file = ...;
>     return vfprintf(my_file, format, ap);
>     /* or output to pre-opened file descriptor */
>     int my_fd = ...;
>     return vdprintf(my_fd, format, ap);
>     /* or output to string buffer and print the string */
>     char buf[128];
>     vsnprintf(buf, sizeof(buf), format, ap);
>     return my_printf("%s", buf);
> }
> ```
>
> and then use `cmake -DWAMR_BH_VPRINTF=my_vprintf ..` to pass the callback function, or add `BH_VPRINTF=my_vprintf` macro for the compiler, e.g. add line `add_defintions(-DBH_VPRINTF=my_vprintf)` in CMakeListst.txt.

#### **Enable reference types feature**
- **WAMR_BUILD_REF_TYPES**=1/0, default to disable if not set

#### **Exclude WAMR application entry functions**
- **WAMR_DISABLE_APP_ENTRY**=1/0, default to disable if not set

> Note: The WAMR application entry (`core/iwasm/common/wasm_application.c`) encapsulate some common process to instantiate, execute the wasm functions and print the results. Some platform related APIs are used in these functions, so you can enable this flag to exclude this file if your platform doesn't support those APIs.
> *Don't enable this flag if you are building `product-mini`*

**Combination of configurations:**

We can combine the configurations. For example, if we want to disable interpreter, enable AOT and WASI, we can run command:

``` Bash
cmake .. -DWAMR_BUILD_INTERP=0 -DWAMR_BUILD_AOT=1 -DWAMR_BUILD_LIBC_WASI=0 -DWAMR_BUILD_PLATFORM=linux
```

Or if we want to enable interpreter, disable AOT and WASI, and build as X86_32, we can run command:

``` Bash
cmake .. -DWAMR_BUILD_INTERP=1 -DWAMR_BUILD_AOT=0 -DWAMR_BUILD_LIBC_WASI=0 -DWAMR_BUILD_TARGET=X86_32
```

## Cross compilation

If you are building for ARM architecture on a X86 development machine, you can use the `CMAKE_TOOLCHAIN_FILE`  to set the toolchain file for cross compling.

```
cmake .. -DCMAKE_TOOLCHAIN_FILE=$TOOL_CHAIN_FILE  \
         -DWAMR_BUILD_PLATFORM=linux    \
         -DWAMR_BUILD_TARGET=ARM
```

Refer to toolchain sample file [`samples/simple/profiles/arm-interp/toolchain.cmake`](../samples/simple/profiles/arm-interp/toolchain.cmake) for how to build mini product for ARM target architecture.

Linux
-------------------------
First of all please install the dependent packages.
Run command below in Ubuntu-18.04:

``` Bash
sudo apt install build-essential cmake g++-multilib libgcc-8-dev lib32gcc-8-dev
```
Or in Ubuntu-16.04:
``` Bash
sudo apt install build-essential cmake g++-multilib libgcc-5-dev lib32gcc-5-dev
```
Or in Fedora:
``` Bash
sudo dnf install glibc-devel.i686
```

After installing dependencies, build the source code:
``` Bash
cd product-mini/platforms/linux/
mkdir build
cd build
cmake ..
make
```


By default in Linux, the interpreter, AOT and WASI are enabled, and JIT and LazyJIT are disabled.
And the build target is set to X86_64 or X86_32 depending on the platform's bitwidth.

To enable WASM JIT, firstly we should build LLVM:

``` Bash
cd product-mini/platforms/linux/
./build_llvm.sh     (The llvm source code is cloned under <wamr_root_dir>/core/deps/llvm and auto built)
```

Then pass argument `-DWAMR_BUILD_JIT=1` to cmake to enable WASM JIT:

``` Bash
mkdir build
cd build
cmake .. -DWAMR_BUILD_JIT=1
make
```

Moreover, pass arguments `-DWAMR_BUILD_JIT=1` and `-DWAMR_BUILD_LAZY_JIT=1` together to cmake to enable WASM Lazy JIT.
If Lazy JIT is enabled, then jit function bodies in the module will not be compiled until they are first called,
so compile time reduces significantly.


Linux SGX (Intel Software Guard Extension)
-------------------------

Please see [Build and Port WAMR vmcore for Linux SGX](./linux_sgx.md) for the details.

MacOS
-------------------------

Make sure to install Xcode from App Store firstly, and install cmake.

If you use Homebrew, install cmake from the command line:
``` Bash
brew install cmake
```

Then build the source codes:
``` Bash
cd product-mini/platforms/darwin/
mkdir build
cd build
cmake ..
make
```
Note:
WAMR provides some features which can be easily configured by passing options to cmake, please see [WAMR vmcore cmake building configurations](./build_wamr.md#wamr-vmcore-cmake-building-configurations) for details. Currently in MacOS, interpreter, AoT, and builtin libc are enabled by default.

Windows
-------------------------

Make sure `MSVC` and `cmake` are installed and available in the command line environment

Then build the source codes:
``` Bash
cd core/deps/
git clone https://github.com/nodejs/uvwasi.git

cd product-mini/platforms/windows/
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
The executable file is `build/Release/iwasm.exe`

VxWorks
-------------------------
VxWorks 7 SR0620 release is validated.

First you need to build a VSB. Make sure *UTILS_UNIX* layer is added in the VSB.
After the VSB is built, export the VxWorks toolchain path by:
```bash
export <vsb_dir_path>/host/vx-compiler/bin:$PATH
```
Now switch to iwasm source tree to build the source code:
```bash
cd product-mini/platforms/vxworks/
mkdir build
cd build
cmake ..
make
```
Create a VIP based on the VSB. Make sure the following components are added:
* INCLUDE_POSIX_PTHREADS
* INCLUDE_POSIX_PTHREAD_SCHEDULER
* INCLUDE_SHARED_DATA
* INCLUDE_SHL

Copy the generated iwasm executable, the test WASM binary as well as the needed
shared libraries (libc.so.1, libllvm.so.1 or libgnu.so.1 depending on the VSB,
libunix.so.1) to a supported file system (eg: romfs).

Note:
WAMR provides some features which can be easily configured by passing options to cmake, please see [WAMR vmcore cmake building configurations](./build_wamr.md#wamr-vmcore-cmake-building-configurations) for details. Currently in VxWorks, interpreter and builtin libc are enabled by default.

Zephyr
-------------------------
You need to download the Zephyr source code first and embed WAMR into it.
``` Bash
git clone https://github.com/zephyrproject-rtos/zephyr.git
cd zephyr/samples/
cp -a <wamr_root_dir>/product-mini/platforms/zephyr/simple .
cd simple
ln -s <wamr_root_dir> wamr
source ../../zephyr-env.sh
# Execute the ./build_and_run.sh script with board name as parameter. Here take x86 as example:
./build_and_run.sh x86

```

Note:
WAMR provides some features which can be easily configured by passing options to cmake, please see [WAMR vmcore cmake building configurations](./build_wamr.md#wamr-vmcore-cmake-building-configurations) for details. Currently in Zephyr, interpreter, AoT and builtin libc are enabled by default.


AliOS-Things
-------------------------
1. a developerkit board id needed for testing
2. download the AliOS-Things code
   ``` Bash
   git clone https://github.com/alibaba/AliOS-Things.git
   ```
3. copy <wamr_root_dir>/product-mini/platforms/alios-things directory to AliOS-Things/middleware, and rename it as iwasm
   ``` Bash
   cp -a <wamr_root_dir>/product-mini/platforms/alios-things middleware/iwasm
   ```
4. create a link to <wamr_root_dir> in middleware/iwasm/ and rename it to wamr
   ``` Bash
   ln -s <wamr_root_dir> middleware/iwasm/wamr
   ```
5. modify file app/example/helloworld/helloworld.c, patch as:
   ``` C
   #include <stdbool.h>
   #include <aos/kernel.h>
   extern bool iwasm_init();
   int application_start(int argc, char *argv[])
   {
        int count = 0;
        iwasm_init();
       ...
   }
   ```
6. modify file app/example/helloworld/aos.mk
   ``` C
      $(NAME)_COMPONENTS := osal_aos iwasm
   ```
7. build source code and run
   For linux host:

   ``` Bash
   aos make helloworld@linuxhost -c config
   aos make
   ./out/helloworld@linuxhost/binary/helloworld@linuxhost.elf
   ```

   For developerkit:
   Modify file middleware/iwasm/aos.mk, patch as:

   ``` C
   WAMR_BUILD_TARGET := THUMBV7M
   ```

   ``` Bash
   aos make helloworld@developerkit -c config
   aos make
   ```
   download the binary to developerkit board, check the output from serial port

RT-Thread
-------------------------

1. Get rt-thread [system codes](https://github.com/RT-Thread/rt-thread).

2. Enable WAMR software package with menuconfig tool which provided by RT-Thread.

   * Environment in Linux, run command below:

   ```bash
   scons --menuconfig
   ```

   * Environment in Windows ConEmu, run command below:

   ```bash
   menuconfig
   ```

   Select and enable `WAMR` in:

   * RT-Thread online packages
     * tools packages
       * WebAssembly Micro Runtime (WAMR)

3. Configure `WAMR` with menuconfig tool.

   you can choice features of iwasm below:

   * Enable testing parameters of iwasm
   * Enable interpreter Mode / Fast interpreter Mode
   * Use built-libc
   * Enable AOT

4. Exit menuconfig tool and save configure, update and download package.

   ```bash
   pkgs --update
   ```

5. build project and download the binary to boards.

   ```bash
   scons
   ```

   or build project with 8-thread by using command below:

   ```bash
   scons -j8
   ```

   after project building, you can got an binary file named `rtthread.bin`, then you can download this file to the MCU board.

Android
-------------------------
able to generate a shared library support Android platform.
- need an [android SDK](https://developer.android.com/studio). Go and get the "Command line tools only"
- look for a command named *sdkmanager* and download below components. version numbers might need to check and pick others
   - "build-tools;29.0.3"
   - "cmake;3.10.2.4988404"
   - "ndk;21.0.6113669"
   - "patcher;v4"
   - "platform-tools"
   - "platforms;android-29"
- add bin/ of the downloaded cmake to $PATH
- export ANDROID_SDK_HOME=/the/path/of/downloaded/sdk/
- export ANDROID_NDK_HOME=/the/path/of/downloaded/sdk/ndk/
- ready to go

Use such commands, you are able to compile with default configurations. Any compiling requirement should be satisfied by modifying product-mini/platforms/android/CMakeList.txt. For example, chaning ${WAMR_BUILD_TARGET} in CMakeList could get different libraries support different ABIs.

``` shell
$ cd product-mini/platforms/android/
$ mkdir build
$ cd build
$ cmake ..
$ make
$ # check output in distribution/wasm
$ # include/ includes all necesary head files
$ # lib includes libiwasm.so
```

NuttX
-------------------------
WAMR is intergrated with NuttX, just enable the WAMR in Kconfig option (Application Configuration/Interpreters).

Docker
-------------------------
[Docker](https://www.docker.com/) will download all the dependencies and build WAMR Core on your behalf.

Make sure you have Docker installed on your machine: [macOS](https://docs.docker.com/docker-for-mac/install/), [Windows](https://docs.docker.com/docker-for-windows/install/) or [Linux](https://docs.docker.com/install/linux/docker-ce/ubuntu/).

Build *iwasm* with the Docker image:

``` Bash
$ cd ci
$ ./build_wamr.sh
$ ls ../build_out/
```

*build_wamr.sh* will generate *linux* compatible libraries ( libiwasm.so and
libvmlib.a ) and an executable binary (*iwasm*) and copy *iwasm* to
*build_out*. All original generated files are still under
*product-mini/platforms/linux/build*.
