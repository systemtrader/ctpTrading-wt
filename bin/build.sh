#!/bin/bash

# g++ -o cmdCenter ../src/cmdCenter.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lhiredis
# g++ -o marketSrv ../src/market/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lthostmduserapi -lhiredis
# g++ -o kLineSrv ../src/strategy/kLine/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp ../src/protos/*.cpp -lhiredis
# g++ -o tradeLogicSrv ../src/strategy/logicfrontend/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp ../src/protos/*.cpp -lhiredis
g++ -o tradeStrategySrv ../src/strategy/tradebackend/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp ../src/protos/*.cpp -lhiredis -lrt
#g++ -o tradeSrv ../trader/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lthosttraderapi -lhiredis
