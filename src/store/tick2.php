<?php

/**
* 读取历史K线
*/
class Refresh
{

    function __construct($i, $id)
    {
        $this->i = $i;
        $this->id = $id;
    }

    public function run()
    {
        // $this->tick1DB = new PDO("mysql:dbname=tick1;host=127.0.0.1", "root", "Abc518131!");
        $this->tickDB = new PDO("mysql:dbname=tick;host=127.0.0.1", "root", "Abc518131!");

        // 删除旧数据
        $iID = "{$this->i}{$this->id}";
        $sql = "DELETE FROM `tick` WHERE `instrumnet_id` = '{$iID}'";
        $st = $this->tickDB->prepare($sql);
        $st->execute([]);

        for ($i=601; $i < 625; $i++) { 
            echo $i . PHP_EOL;
            $fileName = "/" . strtoupper($this->i) . "201606/{$this->i}{$this->id}_20160{$i}.csv";
            $this->refresh($fileName);
        }
    }

    private function refresh($fileName)
    {
        $sTime = time();
        $tickDB = $this->tickDB;
        $iID = "{$this->i}{$this->id}";
        
        // 导入新数据
        $fileBase = "/home/dev/source/ctpDev/data";
        $file = $fileBase . $fileName;

        if (!is_file($file)) return;
        $content = file_get_contents($file);
        $content = explode(PHP_EOL, $content);
        array_shift($content);
        $total = count($content);
        $one = ceil($total / 100);
        foreach ($content as $key => $item) {
            if (($key + 1) % $one == 0) {
                $percent = intval($key / $one);
                echo "[";
                for ($j=0; $j < $percent; $j++) { 
                    echo ">";
                }
                for ($j=0; $j < 100 - $percent; $j++) { 
                    echo " ";
                }
                echo "]\r";
            }

            $line = explode(',', $item);
            if (count($line) < 3) continue;
            $iID = $line[1];
            list($time, $msec) = explode('.', $line[2]);
            $price = $line[3];
            $volume = $line[4];
            $bid_price1 = $line[12];
            $bid_volume1 = $line[14];
            $ask_price1 = $line[13];
            $ask_volume1 = $line[15];
            $params = [$iID, $time, $msec, $price, $volume, $bid_price1, $bid_volume1, $ask_price1, $ask_volume1];
            $sql = "INSERT INTO `tick` (`instrumnet_id`, `time`, `msec`, `price`, `volume`, `bid_price1`, `bid_volume1`, `ask_price1`, `ask_volume1`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
            $st = $tickDB->prepare($sql);
            $st->execute($params);
        }
        $t = time() - $sTime;
        echo PHP_EOL . "花费{$t}s" . PHP_EOL . PHP_EOL; 
    }

}


$h = new Refresh($argv[1], $argv[2]);
$h->run();
