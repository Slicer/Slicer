#!/usr/bin/csh
./Slicer3 --clear-module-cache --verbose-module-discovery 2>&1 | tee listOfModules.txt
grep "A module named" listOfModules.txt | grep -v already > listOfUniqueModules.txt

