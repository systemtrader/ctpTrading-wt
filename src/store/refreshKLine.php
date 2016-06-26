<?php

/**
* 读取历史K线
*/
class Refresh
{

    function __construct($iID, $range)
    {
        $this->iID = $iID;
        $this->kRange = $range;
    }

    public function run()
    {
        $db = new PDO("mysql:dbname=ctp_1;host=127.0.0.1", "root", "Abc518131!");
        $tickDB = new PDO("mysql:dbname=tick;host=127.0.0.1", "root", "Abc518131!");
        $this->refresh($this->iID, $this->kRange, $db, $tickDB);
    }

    private function refresh($iID, $range, $db, $tickDB)
    {
        // 清空原有数据
        $sql = "DELETE FROM `kline` WHERE `instrumnet_id` = '{$iID}'";
        $st = $db->prepare($sql);
        $st->execute([]);
        // 获取最新K线
        // $sql = "SELECT"

        $id = 0;
        $index = 0;
        while (true) {
            $id = $this->makeBlock($id, $iID, $range, $index, $db, $tickDB);
            $index++;
            if (!$id) break;
        }
    }

    public function makeBlock($id, $iID, $range, $index, $db, $tickDB)
    {
        if (empty($id)) {
            $sql = "SELECT * FROM `tick` WHERE `instrumnet_id` = '{$iID}' ORDER BY `id` LIMIT 1";
        } else {
            $sql = "SELECT * FROM `tick` WHERE `id` = {$id}";
        }
        $st = $tickDB->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $open = $res[0];
        if (!$open) return false;

        $startPrice = $open['price'];
        $highPrice = $startPrice + $range;
        $lowPrice = $startPrice - $range;
        $sql = "SELECT * FROM `tick` WHERE (`price` >= {$highPrice} or `price` <= {$lowPrice}) AND `id` > {$id} AND `instrumnet_id` = '{$iID}' ORDER BY `id` LIMIT 1";
        $st = $tickDB->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $close = $res[0];
        if (!$close) return false;

        $openID = $open['id'];
        $closeID = $open['id'];
        $sql = "SELECT MAX(`price`) AS `max`, MIN(`price`) AS `min` FROM `tick` WHERE `id` >= {$openID} AND `id` <= {$closeID} AND `instrumnet_id` = '{$iID}'";
        $st = $tickDB->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $res = $res[0];
        $max = $res['max'];
        $min = $res['min'];

        $type = 2;
        if ($open['price'] > $close['price']) $type = 1;

        $sql = "INSERT INTO `kline` (`index`, `open_time`, `open_msec`, `close_time`, `close_msec`, `open_price`, `close_price`, `max_price`, `min_price`, `volume`, `type`, `instrumnet_id`, `range`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        $params = array($index, $open['time'], $open['msec'], $close['time'], $close['msec'], $open['price'], $close['price'], $max, $min, $close['volume'], $type, $iID, $range);
        $st = $db->prepare($sql);
        $st->execute($params);

        return $close['id'];
    }

}


$h = new Refresh($argv[1], $argv[2]);
$h->run();
