# ZControl

Filesystem RPC C++ Library for Unix systems

## Architecture
- **Commander** - Library that communicates with a single **Worker** and issues commands to it
- **Worker** - Library that communicates with a **Commander** and handles commands
- **Server** - Accepts incoming **Worker** connections and manages **Commanders** for them
- **ZCFS** - FUSE module that allows access to remote **Workers** filesystems
- **ZCWorkerd** - Executable wrapper for a **Worker**

## Building
To build you must use CMake 3.9+ and compiler supporting C++17. Enter the source directory and run the following commands:
```
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
```
To diasble building of unneeded components, override the options specified under the main `CMakeList.txt`

## ZCFS Usage
- Mount ZCFS under /mnt/z with default host and port parameters (0.0.0.0:4444)
```
./zcfs/zcfs /mnt/z
```
Mount ZCFS under /mnt/z with specified host and port parameters
```
./zcfs/zcfs /mnt/z -o host=127.0.0.1,port=4422
````
To unmount, use `unmount` as in other file systems

## ZCWorkerd Usage
```
./zcworkerd/zcworkerd [HOST] [PORT]
```
