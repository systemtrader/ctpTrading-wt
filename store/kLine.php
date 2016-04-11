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
        echo "kLine consumer run..." . PHP_EOL;
        while (true) {
            $data = $this->consumer->popViaRds("K_LINE_Q");
            if ($data) {
                $localTime = strtotime(str_replace("-", " ", $data[0]));
                $index = $data[1];
                $type = $data[2];
                $openTime = strtotime($data[3] . " " . $data[4]);
                $open = $data[5];
                $max = $data[6];
                $min = $data[7];
                $close = $data[8];
                $volume = $data[9];
                $closeTime = strtotime($data[10] . " " . $data[11]);
                $sql = "insert into `tick` (`index`, `open_time`, `close_time`, `open_price`, `close_price`, `max_price`, `min_price`, `volume`) values (?, ?, ?, ?, ?, ?, ?, ?)";
                $params = [
                    $index,
                    date("Y/m/d H:i:s", $openTime),
                    date("Y/m/d H:i:s", $closeTime),
                    $open, $close, $max, $min, $volume,
                ];
                $this->consumer->insertDB($sql, $params);
            } else {
                sleep(1);
            }
        }
    }
}

(new Tick)->run();
