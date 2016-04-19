#!/bin/bash

g++ -o readCsvSendToTick ./*.cpp ../libs/*.cpp ../../include/iniReader/*.cpp -lhiredis