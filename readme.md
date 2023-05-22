# POC_Env_Sensor

Fifth semeseter project created by Antoni Marcinek

Target board - ESP32

Build process is aimed for Ubuntu/Debian based Linux distributions.
 

# 0. Prerequisites
To compile you need to get the following packages:
```
sudo apt-get install git wget flex bison gperf python python-pip python-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util
```
Add the user to `dialout` group:
```
sudo usermod -a -G dialout $USER
```

# 1. Development
In this README, the following variables are assumed to be defined: `$PROJECT_DIR`, `$BUILD_DIR`, `BUILD_TEST_DIR`.
E.g.: 
```
PROJECT_DIR=`pwd`  # source root - directory of this README
BUILD_DIR=$(realpath $PROJECT_DIR/../build-<project_name>)
BUILD_TEST_DIR=$(realpath $PROJECT_DIR/../build-<project_name>-test)
```

## 1.1. Build environment preparation
Before first build, some one-time preparation is required

### 1.1.1. Don't forget to update the submodules first
```
git submodule init && git submodule update
git submodule update --init --recursive
```
To update the submodules to the tracked branch (set in `.gitmodules` file):
```
git submodule update --remote  
```
It will update the submodules in local repo. To commit them, call `git add path/to/submodule` and commit.

### 1.1.2. Apply patches
```
cd $PROJECT_DIR/externals/esp-idf/components/bt/host/nimble/nimble
git stash
git apply ../../../../../../../ble-nimble-patch-1.patch
```

### 1.1.3. Build the esp compiler
```
cd $PROJECT_DIR
cd build
./install_esp32_compiler.sh 
```

## 1.2. Activate the environment in the terminal
Environment needs to be activated for all commands related to building, flashing and monitoring firmware. 
Needs to be run in every terminal.
```
. $PROJECT_DIR/externals/esp-idf/export.sh  
```

## 1.3. Building
To build the firmware, issue the following commands:
```
mkdir $BUILD_DIR
cd $BUILD_DIR
idf.py -B . -C $PROJECT_DIR/ build  
```

## 1.4. Flashing
To flash: 

(if not in `$BUILD_DIR` change `./` to appropriate folder) 
```
idf.py -B ./ -C $PROJECT_DIR/ flash 
```

One can also start the terminal monitor afterwards:
```
idf.py -B ./ -C $PROJECT_DIR/ flash monitor
```
Other commands to be combined: `erase-flash`, e.g.:
```
idf.py -B ./ -C ../pockethernet-esp32/ erase-flash build flash monitor
```

If the board and serial port converter doesn't support automatic control of RESET and BOOT pins control, one may need to drive them manually, e.g. with buttons.

## 1.5. Menuconfig
To configure the ESP32 build - run:
```
# Remember to activate the environment
cd $BUILD_DIR
idf.py -B ./ -C $PROJECT_DIR/ menuconfig
```
## 1.6. Library for DHT22 sensor
Clone the repository:
```
cd $PROJECT_DIR/externals
git clone https://github.com/UncleRus/esp-idf-lib.git
```
Add path to components in CMakeLists.txt:
```
cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS /home/user/myprojects/esp/esp-idf-lib/components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my-esp-project)
```