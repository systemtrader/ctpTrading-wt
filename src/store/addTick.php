<?php
require 'consumer.php';

/**
* 消费tick数据
*/
class Tick
{
    public function run()
    {
        $db = new PDO("mysql:dbname=ctp;host=127.0.0.1", "root", "Abc518131!");
        $dbTick = new PDO("mysql:dbname=tick;host=127.0.0.1", "root", "Abc518131!");

        $start = 1000 * 180;
        while (true) {
            $sql = "SELECT * FROM `tick` LIMIT {$start}, 1000";
            $st = $db->prepare($sql);
            $st->execute(array());
            $res = $st->fetchAll(PDO::FETCH_ASSOC);

            if (empty($res)) return;
            $start += 1000;

            foreach ($res as $line) {
                try {
                    $sql = "insert into `tick` (`instrumnet_id`, `time`, `msec`, `price`, `bid_price1`, `ask_price1`, `volume`) values (?, ?, ?, ?, ?, ?, ?)";
                    $params = array($line['instrumnet_id'], $line['time'], $line['msec'], $line['price'], $line['bid_price1'], $line['ask_price1'], $line['volume']);
                    $st = $dbTick->prepare($sql);
                    $st->execute($params);
                    // echo ".";
                } catch (Exception $e) {
                    echo PHP_EOL . "重复记录：{$line['instrumnet_id']}, {$line['time']}, {$line['msec']}" . PHP_EOL;
                }

            }
        }
    }
}

$tick = new Tick();
$tick->run();
