# WBOT02 - Qt WOTBOT 

WOTBOT actor

## Sections

[Download, build and run main executable](#main)

[Discussion](#Discussion)

<a name="main"></a>

## Download, build and run main executable

This repository is based on the 6.2.4 installation of [Qt](https://www.qt.io) on Windows 11. [Download](https://www.qt.io/download) the installer `C:\Qt\MaintenanceTool.exe`, and run it. Install Qt Creator and the `Desktop Qt 6.2.4 MSVC2019 64bit` kit. (Note that the default `Desktop Qt6.2.4 MinGW 64-bit` kit, which is GCC for Windows, may not have camera or screen grab functionality.) Check the various examples build and run ok.

Now install the [AlignmentActive repository](https://github.com/caiks/AlignmentActive), the [AlignmentRepaC repository](https://github.com/caiks/AlignmentRepaC) and the underlying repositories. Create a directory,

```
mkdir C:\caiks

```
Then use use [Github Desktop](https://desktop.github.com/), for example, to clone the following repositories to the `C:\caiks` directory -
```
https://github.com/Tencent/rapidjson.git
https://github.com/caiks/AlignmentC.git
https://github.com/caiks/AlignmentRepaC.git
https://github.com/caiks/AlignmentActive.git
```
Then clone the [WBOT02 repository](https://github.com/caiks/WBOT02) -
```
https://github.com/caiks/WBOT02.git
```
Then clone the [WBOT02 workspace repository](https://github.com/caiks/WBOT02_ws) -
```
https://github.com/caiks/WBOT02_ws.git
```
The WBOT02 executable can be built and run in Qt Creator by opening the `WBOT02` project in `C:\caiks\WBOT02` and then building the Release configuration. Then in Qt Creator go to 'Projects/WBOT02/Desktop_Qt_6_2_4_MSVC2019_64bit/Run' and set the 'Command line arguments' to `hello`. Then run and check that `hello` appears in the 'Application Output' window. To test `actor001` can grab the screen, set the 'Command line arguments' to `actor001`. The `actor001` window should open on the left hand side of the screen and capture a rectangle on the right hand side of the screen.

The build can also be done from the command line, but in version 6.2.4 of Qt we must first set the necessary environment variables from Qt Creator. In Qt Creator go to 'Projects/WBOT02/Desktop_Qt_6_2_4_MSVC2019_64bit/Build/Details' and hit 'Open Terminal' to create a shell, then run `cmake` - 
```
"C:\Qt\Tools\CMake_64\bin\cmake.exe" -DCMAKE_PREFIX_PATH=C:\Qt\6.2.4\msvc2019_64 -S C:/caiks/WBOT02 -B C:/caiks/build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release

```
Then the executable can be built -
```
cd /d C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release
"C:\Qt\Tools\CMake_64\bin\cmake.exe" --build C:/caiks/build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release --target all

```
Finally the executable can be tested -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" hello > WBOT02.txt

```
Note that standard output does not echo to the shell but must be redirected to a file. Check that `C:\caiks\WBOT02_ws\WBOT02.txt` contains the word 'hello'.

To test `actor001` can grab the screen, run the following -
```
cd /d C:\caiks\WBOT02_ws
"C:\caiks\build-WBOT02-Desktop_Qt_6_2_4_MSVC2019_64bit-Release\WBOT02.exe" actor001

```
The `actor001` window should open on the left hand side of the screen and capture a rectangle on the right hand side of the screen.

<a name = "Discussion"></a>

## Discussion

Now let us investigate various wotbot *slice* topologies and goals. 

[Actor node](#Actor)

[Conclusion](#Conclusion)


<a name = "Actor"></a>

### Actor node

model|scales|frame position|events|fuds|fuds/sz/thrshld|median diagonal|max diagonal|lagging fuds
---|---|---|---|---|---|---|---|---
model001|1.0, 0.5, 0.25, 0.125|centred|720,000|2823|0.784167|31.5714|41.1581|11
model002|0.25|1 centred, 4 offset|720,000|2799|0.7775|31.2526|41.5411|489
model003|0.5|1 centred, 4 offset|720,000|2735|0.759722|31.4093|41.8025|599
model006|0.5|1 centred|180,000|697|0.775099|31.6855|41.1768|0
model007|0.25|1 centred|180,000|698|0.776233|31.458|40.8568|0

<a name = "Conclusion"></a>

### Conclusion
