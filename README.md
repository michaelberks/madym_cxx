# madym_cxx

New public version of the QBI's labs library for DCE-MRI analysis.

## Installation instructions - building from source

## External dependencies
To build from source, Madym requires the following:

1. A C++ compiler that fully supports C++11
2. CMake to configure and generate project files
3. The VXL vision toolkit library
4. The filesystem and system libraries from Boost
5. (Optional) Qt5 - required to build the Madym-GUI, but not required for the main C++ library or command line tools

## TL;DR instructions
If you're used to configuring and building C++ projects with CMake, Madym should be pretty easy to install. So assuming you have the above pre-requisites built and installed:
+ Go the root directory of the madym_cxx repository you have just cloned, there is a CMakeLists.txt file in this directrory, so this is the head of the source tree. We strongly recommend building your binaries outside of the source tree.
+ Run CMake to confiure and generate project files
    - You will need to set the path to VXL in *VXL_DIR* and the Boost fields if Boost is not automatically found by CMake
    - If you want to build the Madym-GUI, switch *BUILD_QT_GUI* to *ON*. You may then need to set *Qt* fields if they are not found automatically by CMake
    - If you want to build your own documentation with Doxygen, set *BUILD_DOCUMENTATION* to on
 + Build the project. If you selected to build documentation, this will only built for release configurations.
 + Run testing using Ctest.

 Everything should build and all tests pass. If there are or any errors, see section on Testing below. Otherwise, you're good to go... see the [project wiki](https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/home) for instructions on running Madym.

 ## Detailed Instructions

#### C++ compiler
Madym has mainly been developed on Windows using Visual Studio 2015 (compiler version msvc-14.0). However it has also been built and tested using [GCC 6.30](https://www.gnu.org/software/gcc/gcc-6/) (Linux) and [LLVM/Clang](https://clang.llvm.org/) (MacOs). Any compilers *at least as* modern as those should be fine.

#### CMake
Madym requires [CMake 3.10](https://cmake.org/download/) or above.

#### VXL
VXL is best built from source using the same compiler you use to build madym. Installation and build instructions are available at [https://vxl.github.io/](https://vxl.github.io/). Madym only requires the *vul* utilities libraries, so if you are only installing VXL for Madym, we recommend setting all build options to *OFF* except *VXL_BUILD_CORE_UTILIES* when you configure VXL in CMake. When you have built VXL, make a note of the binary directory you have built it in, as this will be required when you ocnfigure CMake in Madym

#### Boost
On Linux/MacOS you should be able to install Boost simple using *apt-get* or *brew install*. Any version of Boost &ge;1.60 should be ok, however it is important that whatever Boost you link to has been built with at least as modern a compiler as you are using to build Madym. Madym may build against incompatible Boost libraries, but will not work properly at runtime. Any problems should be picked up during initial tests (see notes below).

For Windows (or if for any reason simply installing on Linux/MacOS doesn't work), it is best to build boost from source. To do so, follow the instructions at [https://www.boost.org](https://www.boost.org/doc/libs/1_62_0/more/getting_started/windows.html).

The tricky bit here is making sure you configure the build settings (achieved using boosts own bootstrap.sh script) to use the correct compiler.

+ Open a developer terminal (this ports with VS and is essentially a Windows command terminal, with the environment pre-set for your Visual Studio version – start typing “developer” at the Windows toolbar search box and Developer Command Prompt for Visual Studio should be the first app that appears).
+ CD into the top level boost folder you’ve just downloaded.
+ Run `bootstrap.sh --with-libraries=filesystem,system`
    + The `with-libraries` option isn’t essential, but will save *lots* of time and disk space compared to building all the libraries, which is the default if we don’t set this option. 
+ If this works you should get a message about Building the Boost.Build engine, and then a message saying bootstrapping is done.

Now call `b2` to build the filesystem and system libraries, setting the following options:
1)	`--toolset=msvc-14.0` this tells boost to use correct compiler (this assumes using Visual Studio 2015, see eg [here](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering) for a list of other versions)
2)	`address-model=64` this makes sure we build 64-bit binaries
3)	`--build-type=complete` tells boost to build both debug and release versions (if you know you only want one, you can put Debug or Release instead of complete)

So the final command should look like:
`b2 --toolset=msvc-14.0 address-model=64 --build-type=complete`

If all has been successful the filesystem and system libraries will have been built into **<Boost_DIR>stage\lib**. Make a note of the location of htese files as ou may need to manually add to CMake to configure Madym.

#### Qt
Qt is only required if you would like to build Madym's GUI tool, that acts as a user-friendly front-end to configure and run the main tools. If you just want to build the main Madym C++ library and command line tools, Qt is not required. To download Qt, use the open source installer for Qt-5, available [here](https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5).

## Configuring Madym
Once the external dependencies are installed, Madym should be pretty quick and simple to configure and build, using the following steps:

1. Create a directory for your binary files, we strongly recommend *out-of-source* builds, *ie* create your binary directory outside of the source directory.

### Windows
2. Open the CMake GUI, set *Where is the source code* to the **madym_cxx** repository you have just cloned, and *Where to build the binaries* to the binary directory you have just created.
3. Click configure, and select the compiler (*eg* Visual Studio 14 2015 Win64) from the list of available compilers.
4. After running compiler checks, an error should return as it would be able to find VXL and possibly Boost (if you built Boost from source). 
    * In the option *VXL_DIR* (this may be under Ungrouped Entries if you have the Grouped checkbox ticked), enter the directory where the VXL binaries where built. This is the directory that contains VXLConfig.cmake file.
    * Tick the Advanced checkbox to see the Boost fields. Don't worry about setting *Boost_DIR* but set the paths to the debug/release versions of the filesystem and system libraries and *Boost_INCLUDE_DIR* to the root boost directory (which should contain a directory *boost*, in which the source code headers for each libraries are contained)
5. Click configure again, and this time no errors should be returned. Click Generate to make the project files.

### Linux/MacOS

2. CD into the binary directory you have just created. Call CCMake with the path to your source code to start CMake's interactive configurer (*eg* `ccmake <path_to_source>`), then press `c` to configure. Once the CMake cache has been generated, future calls to Cmake can be made from the binary directory just using `ccmake .` . 

3.  After running compiler checks, an error should return as it would be able to find VXL and possibly Boost (if you built Boost from source). 
    * In the option *VXL_DIR* (this may be under Ungrouped Entries if you have the Grouped checkbox ticked), enter the directory where the VXL binaries where built. This is the directory that contains VXLConfig.cmake file.
    * Press `t` to toggle to advanced mode to see the Boost fields. Don't worry about setting *Boost_DIR* but set the paths to the debug/release versions of the filesystem and system libraries and *Boost_INCLUDE_DIR* to the root boost directory (which should contain a directory *boost*, in which the source code headers for each libraries are contained)
4. Press `c` to configure again, and this time no errors should be returned. Press `g` to generate the project files.

When you have generated CMake project files, check your binary directory. In addition to any project files, there should be two new directories created from the source tree: **calibration_dir** (containing binary files required for testing) and **generated** (containing a version header, that sets various macro definitions required by the project).

#### Additional configuration options
+ To build the GUI, set *BUILD_QT_GUI* to *ON*. You will then need to set the *QtCore_DIR*, *QtGui_DIR* and *QtWidgets_DIR* fields if Qt was not found automatically.
+ To build your own class documentation (requires Doxygen), *BUILD_DOCUMENTATION* to *ON*.

## Building Madym
### Windows
Go to the binary directory in a file explorer, and there should be a Visual Studion solution file *manchester_qbi_madym.sln*. Open this solution in Visual Studio. In the solution explorer, right-click **ALL_BUILD** and select **Build**. Repeat as desired for Debug/Release configurations.

Now right click **RUN_TESTS** and select build. This will run the library checks. All tests should pass. If any tests fail, this will return a build error - ignore the build error, but check the output to note which tests have failed. See more under *Testing* below. Note the tests will be considerably slower in Debug mode because none of the speed-ups used by ALGLIB are optimised in debug mode.

### Linux/MacOS
CD to the binary directory, and run `make all`. Then run `ctest` to run the library checks. All tests should pass.

## It all worked!
Assuming you've got here and everything has built successfully... great! You're good to go and start doing some DCE analysis. Please see the help documentation on the [project wiki](https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/-/wikis/home) for detailed instructions on how to use Madym. 

## Library structure
Madym is organised to build a reusable C++ library, together with a set of command line tools and a GUI to run T1 mapping and DCE analysis of MRI data. In the top level of the repository is a directory **madym**. This contains source code for the main C++ library **mdm**. Within this directory there are sub-directories **opt**, **qt_gui**, **tools** and **tests**. 

**opt** contains a copy of source code from the open source [ALGLIB optimisation library](https://www.alglib.net/), and builds a separate library **mdm_opt**, which **mdm** links to. 

**gt_gui** contains code for building the GUI, and will generate an executable target **madym_gui**. 

**tools** contains code for command line tools and will generate target executables **calculate_T1**, **calculate_T1_lite**, **madym** and **madym_lite**, all of which link to **mdm**.

If testing is enabled, the source code in **tests** and **tools/tests** will generate additional target executables **mdm_test_driver** and **mdm_tools_test_driver**. Finally, if documentation is enabled, a **docs_doxygen** target will generated.


## Testing
Madym uses CMake's test driver program [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html). There are two levels of test implemented, one for the main library, and one for the command line tools.

The tests of the main library check various core functions of the library all work as expected. The tools tests then run sample executions of each the command line tools to check these produce the expected output.

Keep an eye for `test_boost` in the main library tests (currently test `#4`). If for any reason the Boost libraries you have linked to are incompatible with the compiler you've used to build madym, this tests (and all subsequent) should fail. 

The tests use binary files in the **calibration_data** directory copied into the binary directory when CMake generates project files. These files contain pre-generated simulated series of variable flip-angles and associated signal data for testing T1 calculation, and time-series of DCE data for various tracer-kinetic models implemented in Madym. For completeness, source code for generating these files is included in the **tests** directory (make_calibration_data.cxx). However, unless you have very good reason to do so, you ***SHOULD NOT*** need to run this and make new copies of the data. We have externally validated the values in the current data, and they are used to check any subsequent changes to Madym reproduce the same output. For this reason, an option to build this target is not even set in CMakeLists.txt.

If for example, you have implemented a new model, you may wish to look at make_calibration_data.cxx to make your own additional calibration data (and it should be obvious how to create a target executable to do so using CMake). However if you make changes to the existing code-base (perhaps you've spotted we could compute things more efficiently) you want to be able to test your new code reproduces the same output as the old. In this case you **DO NOT WANT** to recompute the calibration data to test your code.

If have any issues with the code please email <michael.berks@mancester.ac.uk>, or drop us a message through GitLab. Otherwise...

## Have fun!



