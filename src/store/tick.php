<?php
require 'consumer.php';

/**
* 消费tick数据
*/
class Tick
{

    function __construct()
    {
        $this->consumer = new Consumer();
    }

    public function run()
    {
        echo "tick consumer run" . PHP_EOL;
        while (true) {
            $data = $this->consumer->popViaRds("MARKET_TICK_Q");
            if (count($data) > 1) {
                $time = $data[4] ? strtotime($data[4] . " " . $data[5]) : time();
                $msec = $data[6];
                $price = $data[0];
                $volume = $data[1];
                $bid = $data[2];
                $ask = $data[3];
                $askVolume1 = $data[8];
                $bidVolume1 = $data[9];
                $instrumnetID = $data[7];
                $sql = "insert into `tick` (`time`, `msec`, `price`, `bid_price1`, `ask_price1`, `volume`, `instrumnet_id`, `ask_volume1`, `bid_volume1`) values (?, ?, ?, ?, ?, ?, ?, ?, ?)";
                $params = array(date("Y/m/d H:i:s", $time), $msec, $price, $bid, $ask, $volume, $instrumnetID, $askVolume1, $bidVolume1);
                echo ".";
                $this->consumer->insertDB($sql, $params);
            } else {
                sleep(1);
            }
        }
    }
}

$tick = new Tick();
$tick->run();
