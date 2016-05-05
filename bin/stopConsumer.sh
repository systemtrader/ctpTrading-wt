#!/bin/bash

ps -ef | grep tools | grep php | grep -v grep | cut -c 9-15 | xargs kill -9
