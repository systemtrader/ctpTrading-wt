<?php
require 'consumer.php';
/**
* 消费K线数据
*/
class KLine
{

    function __construct()
    {
        $this->consumer = new Consumer();
    }

    public function run()
    {
        echo "kLine consumer run" . PHP_EOL;
        while (true) {
            $data = $this->consumer->popViaRds("K_LINE_Q");
            if (count($data) > 1) {
                $index = $data[0];
                $type = $data[1];
                $openTime = $data[2] ? strtotime($data[2] . " " . $data[3]) : time();
                $openMsec = $data[4];
                $open = $data[5];
                $max = $data[6];
                $min = $data[7];
                $close = $data[8];
                $volume = $data[9];
                $closeTime = $data[10] ? strtotime($data[10] . " " . $data[11]) : time();
                $closeMsec = $data[12];
                $instrumnetID = $data[13];
                $sql = "insert into `kline` (`index`, `open_time`, `open_msec`, `close_time`, `close_msec`, `open_price`, `close_price`, `max_price`, `min_price`, `volume`, `type`, `instrumnet_id`) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
                $params = array(
                    $index,
                    date("Y/m/d H:i:s", $openTime), $openMsec,
                    date("Y/m/d H:i:s", $closeTime), $closeMsec,
                    $open, $close, $max, $min, $volume, $type, $instrumnetID,
                );
                echo ".";
                $res = $this->consumer->insertDB($sql, $params);
            } else {
                sleep(1);
            }
        }
    }
}

$kline = new kLine();
$kline->run();
