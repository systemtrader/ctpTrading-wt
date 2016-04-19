#!/bin/bash

# g++ -o cmdCenter ../src/cmdCenter.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lhiredis
g++ -o marketSrv ../src/market/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lthostmduserapi -lhiredis
#g++ -o tradeSrv ../trader/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lthosttraderapi -lhiredis
# g++ -o kLineSrv ../strategy/kLine/*.cpp ../strategy/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis
# g++ -o tradeLogicFrontend ../strategy/logicfrontend/*.cpp ../strategy/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis
# g++ -o tradeStrategyBackend ../strategy/tradebackend/*.cpp ../strategy/*.cpp ../libs/*.cpp ../iniReader/*.cpp -lhiredis -lrt
