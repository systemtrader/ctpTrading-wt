<?php

class kLine
{
    function __construct()
    {
        $res = parse_ini_file(dirname(__FILE__) . '/../../etc/config.ini');
        if ($res['is_dev']) {
            $this->mysqldb = $res['mysql_db_dev'];
            $this->rdsdb = $res['rds_db_dev'];
        } else {
            $this->mysqldb = $res['mysql_db_online'];
            $this->rdsdb = $res['rds_db_online'];
        }
        $this->mysql = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");

        $this->index = 0;
        $this->range = $res['k_range'];

        $sql = "TRUNCATE `kline`";
        $st = $this->mysql->prepare($sql);
        $st->execute(array());
    }


    public function makeBlock($id)
    {
        if (empty($id)) {
            $sql = "SELECT * FROM `tick` ORDER BY `id` LIMIT 1";
        } else {
            $sql = "SELECT * FROM `tick` WHERE `id` = {$id}";
        }

        $st = $this->mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $open = $res[0];
        if (!$open) return false;

        $startPrice = $open['price'];
        $highPrice = $startPrice + $this->range;
        $lowPrice = $startPrice - $this->range;
        $sql = "SELECT * FROM `tick` WHERE (`price` >= {$highPrice} or `price` <= {$lowPrice}) AND `id` > {$id} ORDER BY `id` LIMIT 1";
        $st = $this->mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $close = $res[0];
        if (!$close) return false;

        $openID = $open['id'];
        $closeID = $open['id'];
        $sql = "SELECT MAX(`price`) AS `max`, MIN(`price`) AS `min` FROM `tick` WHERE `id` >= {$openID} AND `id` <= {$closeID}";
        $st = $this->mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $res = $res[0];
        $max = $res['max'];
        $min = $res['min'];

        $type = 2;
        if ($open > $close) $type = 1;

        $sql = "INSERT INTO `kline` (`index`, `open_time`, `open_msec`, `close_time`, `close_msec`, `open_price`, `close_price`, `max_price`, `min_price`, `volume`, `type`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        $params = array($this->index++, $open['time'], $open['msec'], $close['time'], $close['msec'], $open['price'], $close['price'], $max, $min, $close['volume'], $type);
        $st = $this->mysql->prepare($sql);
        $st->execute($params);
        return $close['id'] + 1;

    }
}

$kLine = new kLine();
$id = 0;
while (true) {
    $id = $kLine->makeBlock($id);
    if (!$id) break;
}
