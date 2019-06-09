# Instructions

## Setup

### Windows

#### GitBash/MinGW

1. Install [GitBash](https://git-scm.com/downloads)
2. Install [MinGw64](https://sourceforge.net/projects/mingw-w64/files/latest/download) with the options `x86_64`  and `posix` and add the directory `C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin` to the user path variable (search for `enironmental variables` and double click the `Path` table entry)
3. Install [CMake](https://cmake.org/download) and add the directory `C:\Program Files\CMake\bin` to the user path variable (search for `enironmental variables` and double click the `Path` table entry)
4. Reboot your computer

#### Visual Studio

1. Install [Visual Studio Community 2019 Preview](https://visualstudio.microsoft.com/vs/preview)
2. Install [CMake](https://cmake.org/download) and add the directory `C:\Program Files\CMake\bin` to the user path variable (search for `enironmental variables` and double click the `Path` table entry)
3. Reboot your computer

##### Info

If you later decide you want to change the `x64 Debug` to `Mingw64-Debug` you need to change the `CMAKE_C_COMPILER`, `CMAKE_CXX_COMPILER` to the ones  in `C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin` and because of some IntelliSense error in the raw JSON `CMakeSettings.json` the key `"generator": "Ninja"` and the key `"intelliSenseMode": "windows-msvc-x64"` to not get errors that `std` headers are not found.

## Run

### Windows

#### GitBash/MinGW

Open `GitBash` and run:

```sh
./build.sh
```

The files are built into the `dist` directory.

```sh
# Execute the program
./dist/main
```

#### Visual Studio

Open `Visual Studio` and click `File/Datei`, `Open/Öffnen`, `CMake` and select the `CMakeLists.txt` file.

Then select either `x64 Debug` or `x64 Release` (much faster, like times 50) and `main.exe`.

Now click the green *Play/Run* triangle.

## Clean

Remove all temporary files

```sh
./clean.sh
```

