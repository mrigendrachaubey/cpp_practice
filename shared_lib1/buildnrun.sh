#!/bin/bash
#helper script

clear
make clean
echo -e "\e[32mBuilding...\e[0m"
make
echo -e "\e[32mBuilding done.\e[0m"
echo -e "\e[32mexporting loadable library path..\e[0m"

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`
export LD_LIBRARY_PATH
echo -e "\e[32mrun test app.\e[0m"
./main
unset LD_LIBRARY_PATH