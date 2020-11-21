[![Teeworlds Nodes](https://nodes.teeworlds.dev/logo_github.png)](https://nodes.teeworlds.dev) ![GitHub Actions](https://github.com/teeworlds/teeworlds/workflows/Build/badge.svg)
=========

Teeworlds Nodes 0.7 Remake
---------------------------

Teeworlds Nodes is a remake of the 0.5 Nodes mod by inherited & Mo(2).  
This modification is based on Teeworlds.

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software. See license.txt for full license
text including copyright information.

Please visit https://nodes.teeworlds.dev/ for more information.

---

Building on Linux or macOS (CMake)
==========================

Installing dependencies
-----------------------

    # Debian/Ubuntu
    sudo apt install build-essential cmake git libfreetype6-dev libsdl2-dev libpnglite-dev libwavpack-dev python3 libcurl4-openssl-dev libssl-dev


Downloading repository
----------------------

    git clone https://github.com/teeworldsnetwork/nodes --recurse-submodules
    cd nodes

    # If you already cloned the repository before, use:
    # git submodule update --init


Building
--------

    mkdir -p build
    cd build
    cmake ..
    make

On subsequent builds, you only have to repeat the `make` step.

You can then run the client with `./twnodes` and the server with
`./twnodes_srv`.


Build options
-------------

The following options can be passed to the `cmake ..` command line (between the
`cmake` and `..`) in the "Building" step above.

`-GNinja`: Use the Ninja build system instead of Make. This automatically
parallelizes the build and is generally **faster**. (Needs `sudo apt install
ninja-build` on Debian, `sudo dnf install ninja-build` on Fedora, and `sudo
pacman -S --needed ninja` on Arch Linux.)

`-DDEV=ON`: Enable debug mode and disable some release mechanics. This leads to
**faster** builds.

`-DCLIENT=OFF`: Disable generation of the client target. Can be useful on
headless servers which don't have graphics libraries like SDL2 installed.


Building on Windows with Visual Studio & CMake
======================

Download and install some version of [Microsoft Visual
Studio](https://www.visualstudio.com/) (as of writing, MSVS Community 2019)
with the following components:

* Desktop development with C++ (on the main page)
* Python development (on the main page)
* Git for Windows (in Individual Components → Code tools)

Run Visual Studio. Open the Team Explorer (View → Team Explorer, Ctrl+^,
Ctrl+M). Click Clone (in the Team Explorer, Connect → Local Git Repositories).
Enter `https://github.com/teeworldsnetwork/nodes` into the first input box. Wait
for the download to complete (terminals might pop up).

Wait until the CMake configuration is done (watch the Output windows at the
bottom).

Select `twnodes.exe` in the Select Startup Item… combobox next to the green
arrow. Wait for the compilation to finish.

For subsequent builds you only have to click the button with the green arrow
again.


Building on Windows with MinGW & CMake
======================

Download and install MinGW with at least the following components:

- mingw-developer-toolkit-bin
- mingw32-base-bin
- mingw32-gcc-g++-bin
- msys-base-bin

Also install [Git](https://git-scm.com/downloads) (for downloading the source
code), [Python](https://www.python.org/downloads/) and
[CMake](https://cmake.org/download/).

Open CMake ("CMake (cmake-gui)" in the start menu). Click "Browse Source"
(first line) and select the directory with the Teeworlds source code. Next,
click "Browse Build" and create a subdirectory for the build (e.g. called
"build"). Then click "Configure". Select "MinGW Makefiles" as the generator and
click "Finish". Wait a bit (until the progress bar is full). Then click
"Generate".

You can now build Teeworlds by executing `mingw32-make` in the build directory.
