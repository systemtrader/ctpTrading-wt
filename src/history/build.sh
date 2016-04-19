#!/bin/bash

g++ -o readCsvSendToTick ./*.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis