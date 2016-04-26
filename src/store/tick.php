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
                $sql = "insert into `tick` (`time`, `msec`, `price`, `bid_price1`, `ask_price1`, `volume`) values (?, ?, ?, ?, ?, ?)";
                $params = array(date("Y/m/d H:i:s", $time), $msec, $price, $bid, $ask, $volume);
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
