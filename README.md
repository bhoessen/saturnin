 _____       ___   _____   _   _   _____    __   _   _   __   _  
/  ___/     /   | |_   _| | | | | |  _  \  |  \ | | | | |  \ | | 
| |___     / /| |   | |   | | | | | |_| |  |   \| | | | |   \| | 
\___  \   / / | |   | |   | | | | |  _  /  | |\   | | | | |\   | 
 ___| |  / /  | |   | |   | |_| | | | \ \  | | \  | | | | | \  | 
/_____/ /_/   |_|   |_|   \_____/ |_|  \_\ |_|  \_| |_| |_|  \_| 



Description:
------------

Saturnin is a SAT (boolean satisfaction problem) solver engine. It is meant to be fast and easily hackable.

The following options are possible:
 - instance simplification (basic)
 - multi-thread (basic portfolio)
 - db trace using SQLite

Build:
------

In order to compile saturnin, use cmake:

mkdir build
cd build
cmake ..
make


if you want to debug/develop saturnin, use the following commands

mkdir debug
cd debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

Note to windows users: you might want to define the following variables:
CMAKE_BUILD_TYPE:STRING=Debug