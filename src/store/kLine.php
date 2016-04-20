<?php
require 'consumer.php';
/**
*
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
            if ($data) {
                $index = $data[0];
                $type = $data[1];
                $openTime = strtotime($data[2] . " " . $data[3]);
                $openMsec = $data[4];
                $open = $data[5];
                $max = $data[6];
                $min = $data[7];
                $close = $data[8];
                $volume = $data[9];
                $closeTime = strtotime($data[10] . " " . $data[11]);
                $closeMsec = $data[12];
                $sql = "insert into `kline` (`index`, `open_time`, `open_msec`, `close_time`, `close_msec`, `open_price`, `close_price`, `max_price`, `min_price`, `volume`, `type`) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
                $params = array(
                    $index,
                    date("Y/m/d H:i:s", $openTime), $openMsec,
                    date("Y/m/d H:i:s", $closeTime), $closeMsec,
                    $open, $close, $max, $min, $volume, $type,
                );
                echo ".";
                $this->consumer->insertDB($sql, $params);
            } else {
                sleep(1);
            }
        }
    }
}

$kline = new kLine();
$kline->run();
