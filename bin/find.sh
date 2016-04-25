#!/bin/bash
#
#
ps -ef | grep marketSrv | grep -v grep
ps -ef | grep kLineSrv | grep -v grep
ps -ef | grep tradeLogicSrv | grep -v grep
ps -ef | grep tradeStrategySrv | grep -v grep
ps -ef | grep tradeSrv | grep -v grep

ps -ef | grep tradeStrategyAfter | grep -v grep

ps -ef | grep tick.php | grep -v grep
ps -ef | grep readHistoryKLine.php | grep -v grep
ps -ef | grep order.php | grep -v grep
ps -ef | grep kLine.php | grep -v grep
