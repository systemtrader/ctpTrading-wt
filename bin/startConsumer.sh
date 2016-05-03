#!/bin/bash

nohup php /home/dev/source/ctpTrading/src/store/tick.php &
nohup php /home/dev/source/ctpTrading/src/store/order.php &
nohup php /home/dev/source/ctpTrading/src/store/kLine.php &
