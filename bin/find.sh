#!/bin/bash
#
#
ps -ef | grep marketSrv | grep -v grep
ps -ef | grep kLineSrv | grep -v grep 
ps -ef | grep tradeLogicSrv | grep -v grep 
ps -ef | grep tradeStrategySrv | grep -v grep 
ps -ef | grep tradeSrv | grep -v grep 