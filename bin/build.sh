#!/bin/bash

# g++ -o cmdCenter ../src/cmdCenter.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lhiredis
# g++ -o marketSrv ../src/market/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lthostmduserapi -lhiredis
# g++ -o kLineSrv ../src/strategy/kLine/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp ../src/protos/*.cpp -lhiredis
# g++ -o tradeLogicSrv ../src/strategy/logicfrontend/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp ../src/protos/*.cpp -lhiredis
# g++ -o tradeStrategySrv ../src/strategy/tradebackend/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp ../src/protos/*.cpp -lhiredis -lrt
g++ -o tradeSrv ../src/trade/*.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lthosttraderapi -lhiredis

#历史回测模块
# g++ -o readCsvSendToTick ../src/history/ReadCsvSendTick.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lhiredis
# g++ -o tradeStrategySuccess ../src/history/tradeStrategySuccess.cpp ../src/libs/*.cpp ../include/iniReader/*.cpp -lhiredis

# 测试模块
g++ -o tradeSrvBefore ../src/test/tradeSrvBefore.cpp ../include/iniReader/*.cpp ../src/libs/*.cpp -lhiredis
# g++ -o tradeStrategyBefore ../src/test/tradeStrategyBefore.cpp ../include/iniReader/*.cpp ../src/libs/*.cpp -lhiredis
# g++ -o tradeStrategyAfter ../src/test/tradeStrategyAfter.cpp ../include/iniReader/*.cpp ../src/libs/*.cpp -lhiredis

