#!/bin/bash

ps -ef | grep /home/dev/source/ctpTrading/src/store/ | grep -v grep | cut -c 9-15 | xargs kill -9
