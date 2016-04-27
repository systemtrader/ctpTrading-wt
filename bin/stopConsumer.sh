#!/bin/bash

ps -ef | grep /root/source/ctpTrading/src/store/ | grep -v grep | cut -c 9-15 | xargs kill -9