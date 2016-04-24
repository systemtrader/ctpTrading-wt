<?php
require 'consumer.php';
/**
*
*/
class Order
{

    function __construct()
    {
        $this->consumer = new Consumer();
    }

    public function run()
    {
        echo "Order consumer run" . PHP_EOL;
        while (true) {
            $data = $this->consumer->popViaRds("ORDER_LOGS");
            if ($data) {
                $type = $data[0];
                if ($type == "trade") {
                    $kIndex = $data[1];
                    $frontID = $data[2];
                    $sessionID = $data[3];
                    $price = $data[4];
                    $isBuy = $data[5];
                    $isOpen = $data[6];
                    $time = $data[7];
                    list($date, $usec) = explode('.', $time);
                    $sql = "INSERT INTO `order` (`k_index`, `front_id`, `session_id`, `price`, `is_buy`, `is_open`, `start_time`, `start_usec`) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
                    $params = [$kIndex, $frontID, $sessionID, $price, $isBuy, $isOpen, $date, $usec];
                    $this->consumer->insertDB($sql $params);
                }
                if ($type == "traded") {
                    $kIndex = $data[1];
                    $frontID = $data[2];
                    $sessionID = $data[3];
                    $time = $data[4];
                    list($date, $usec) = explode('.', $time);
                    $sql = "UPDATE `order` SET `end_time` = ?, `end_usec` = ?, `status` = 1 WHERE `k_index` = ? AND `front_id` = ? AND `session_id` = ?";
                    $params = [$date, $usec, $kIndex, $frontID, $sessionID];
                    $this->consumer->updateDB($sql, $params);
                }
                if ($type == "orderRtn") {
                    $kIndex = $data[1];
                    $frontID = $data[2];
                    $sessionID = $data[3];
                    $time = $data[4];
                    list($date, $usec) = explode('.', $time);
                    $sql = "UPDATE `order` SET `end_time` = ?, `end_usec` = ? WHERE `k_index` = ? AND `front_id` = ? AND `session_id` = ?";
                    $params = [$date, $usec, $kIndex, $frontID, $sessionID];
                    $this->consumer->updateDB($sql, $params);
                    $sql = "UPDATE `order` SET `first_time` = ?, `first_usec` = ? WHERE `k_index` = ? AND `front_id` = ? AND `session_id` = ? AND `first_time` = 0";
                    $params = [$date, $usec, $kIndex, $frontID, $sessionID];
                    $this->consumer->updateDB($sql, $params);
                }
                echo ".";
            } else {
                sleep(1);
            }
        }
    }
}

$kline = new kLine();
$kline->run();
