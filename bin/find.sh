#!/bin/bash
#
#
ps aux | grep marketSrv | grep -v grep
ps aux | grep kLineSrv | grep -v grep
ps aux | grep tradeLogicSrv | grep -v grep
ps aux | grep tradeStrategySrv | grep -v grep
ps aux | grep tradeSrv | grep -v grep

ps aux | grep tradeStrategyAfter | grep -v grep

ps aux | grep tick.php | grep -v grep
ps aux | grep readHistoryKLine.php | grep -v grep
ps aux | grep order.php | grep -v grep
ps aux | grep kLine.php | grep -v grep
