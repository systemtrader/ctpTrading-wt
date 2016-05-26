<?php
require 'consumer.php';

/**
* 消费订单数据
*/
class Order
{

    function __construct()
    {
        $this->consumer = new Consumer();
    }

    public function run()
    {
        $totalPrice = 0;
        echo "Order consumer run" . PHP_EOL;
        while (true) {
            $data = $this->consumer->popViaRds("ORDER_LOGS");
            if (count($data) > 1) {
                $type = $data[0];
                if ($type == "trade") {
                    $orderID = $data[1];
                    $frontID = $data[2];
                    $sessionID = $data[3];
                    $orderRef = $data[4];
                    $price = $data[5];
                    $isBuy = $data[6];
                    $isOpen = $data[7];
                    $time = str_replace('-', ' ', $data[8]);
                    list($date, $usec) = explode('.', $time);
                    $iID = $data[9];
                    $sql = "INSERT INTO `order` (`order_id`, `front_id`, `session_id`, `order_ref`, `price`, `is_buy`, `is_open`, `start_time`, `start_usec`, `instrumnet_id`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
                    $params = array($orderID, $frontID, $sessionID, $orderRef, $price, $isBuy, $isOpen, $date, $usec, $iID);
                    $this->consumer->insertDB($sql, $params);
                    
                    if ($isOpen) {
                        $oncePrice = $isBuy ? 0 - $price : $price;
		    } else {
                        $oncePrice = $isBuy ? $oncePrice - $price : $oncePrice + $price;
                        $totalPrice += $oncePrice;
                    }
                    file_put_contents('/tmp/totalPrice', $totalPrice, FILE_APPEND);
                }
                if ($type == "traded") {
                    $orderRef = $data[1];
                    $frontID = $data[2];
                    $sessionID = $data[3];
                    $srvTime = $data[4] ? strtotime($data[4] . " " . $data[5]) : time();
                    $srvTime = date("Y/m/d H:i:s", $srvTime);
                    $time = str_replace('-', ' ', $data[6]);
                    list($date, $usec) = explode('.', $time);
                    $realPrice = $data[7];
                    $sql = "UPDATE `order` SET `end_time` = ?, `end_usec` = ?, `srv_traded_time` = ?, `status` = 1, `real_price` = ? WHERE `order_ref` = ? AND `front_id` = ? AND `session_id` = ?";
                    $params = array($date, $usec, $srvTime, $realPrice, $orderRef, $frontID, $sessionID);
                    $this->consumer->updateDB($sql, $params);
                }
                if ($type == "orderRtn") {
                    $orderRef = $data[1];
                    $frontID = $data[2];
                    $sessionID = $data[3];
                    $srvTime = $data[4] ? strtotime($data[4] . " " . $data[5]) : time();
                    $srvTime = date("Y/m/d H:i:s", $srvTime);
                    $time = str_replace('-', ' ', $data[6]);
                    $status = $data[7] == 5 ? 2 : 0;
                    list($date, $usec) = explode('.', $time);
                    $sql = "UPDATE `order` SET `end_time` = ?, `end_usec` = ?, `srv_insert_time` = ?, `status` = ? WHERE `order_ref` = ? AND `front_id` = ? AND `session_id` = ?";
                    $params = array($date, $usec, $srvTime, $status, $orderRef, $frontID, $sessionID);
                    $this->consumer->updateDB($sql, $params);
                    $sql = "UPDATE `order` SET `first_time` = ?, `first_usec` = ? WHERE `order_ref` = ? AND `front_id` = ? AND `session_id` = ? AND `first_time` = 0";
                    $params = array($date, $usec, $orderRef, $frontID, $sessionID);
                    $this->consumer->updateDB($sql, $params);
                }
                if ($type == "klineorder") {
                    $kIndex = $data[1];
                    $orderID = $data[2];
                    $iID = $data[3];
                    $sql = "INSERT INTO `markov_kline_order` (`order_id`, `kindex`, `instrumnet_id`) VALUES (?, ?, ?)";
                    $params = [$orderID, $kIndex, $iID];
                    $this->consumer->insertDB($sql, $params);
                }
                echo ".";
            } else {
                sleep(1);
            }
        }
    }
}

$order = new Order();
$order->run();
