<?php

/**
* 读取历史K线
*/
class Refresh
{

    function __construct()
    {
        $res = parse_ini_file(dirname(__FILE__) . '/../../etc/config.ini');
        if ($res['is_dev']) {
            $this->mysqldb = $res['mysql_db_dev'];
        } else {
            $this->mysqldb = $res['mysql_db_online'];
        }
        $this->iIDs = explode('/', $res['instrumnet_id']);
        $this->kRanges = explode('/', $res['k_range']);
        $this->dbHost = $res['mysql_host'];
    }

    public function run()
    {
        $db = new PDO("mysql:dbname={$this->mysqldb};host={$this->dbHost}", "root", "Abc518131!");
        $tickDB = new PDO("mysql:dbname=tick;host={$this->dbHost}", "root", "Abc518131!");
        $i = 0;
        foreach ($this->iIDs as $iID) {
            $this->refresh($iID, $this->kRanges[$i++], $db, $tickDB);
        }
    }

    private function refresh($iID, $range, $db, $tickDB)
    {
        // 清空原有数据
        $sql = "DELETE FROM `kline` WHERE `instrumnet_id` = '{$iID}'";
        $st = $db->prepare($sql);
        $st->execute([]);

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

        $sql = "INSERT INTO `kline` (`index`, `open_time`, `open_msec`, `close_time`, `close_msec`, `open_price`, `close_price`, `max_price`, `min_price`, `volume`, `type`, `instrumnet_id`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        $params = array($index, $open['time'], $open['msec'], $close['time'], $close['msec'], $open['price'], $close['price'], $max, $min, $close['volume'], $type, $iID);
        $st = $db->prepare($sql);
        $st->execute($params);

        return $close['id'];
    }

}


$h = new Refresh();
$h->run();
