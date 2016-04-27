#!/bin/bash

nohup php /root/source/ctpTrading/src/store/tick.php &
nohup php /root/source/ctpTrading/src/store/order.php &
nohup php /root/source/ctpTrading/src/store/kLine.php &