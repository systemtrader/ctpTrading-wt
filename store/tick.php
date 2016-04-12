<?php
require 'consumer.php';
/**
*
*/
class Tick
{

    function __construct()
    {
        $this->consumer = new Consumer();
    }

    public function run()
    {
        echo "tick consumer run..." . PHP_EOL;
        while (true) {
            $data = $this->consumer->popViaRds("MARKET_TICK_Q");
            if ($data) {
                $localTime = strtotime(str_replace("-", " ", $data[0]));
                $time = strtotime($data[1] . " " . $data[2]);
                $price = $data[3];
                $volume = $data[4];
                $sql = "insert into `tick` (`time`, `price`, `volume`, `local_time`) values (?, ?, ?, ?)";
                $params = [date("Y/m/d H:i:s", $time), $price, $volume, date("Y/m/d H:i:s", $localTime)];
                var_dump($params);
                $this->consumer->insertDB($sql, $params);
            } else {
                sleep(1);
            }
        }
    }
}

(new Tick)->run();
