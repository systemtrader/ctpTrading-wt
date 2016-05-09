<?php

/**
* 读取历史K线
*/
class InitSys
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
        $this->iIDs = explode('/', $res['instrumnet_id']);
        $this->peroid = $res['peroid'];
    }

    public function run()
    {
        $db = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");
        $rds = new Redis();
        $rds->connect('127.0.0.1', 6379);
        $rds->select($this->rdsdb);

        foreach ($this->iIDs as $iID) {
            // $this->initKLine($iID, $rds, $db);
            // $this->initTradeStatus($iID, $rds);
            $this->initKLineTick($iID, $rds, $db);
        }
    }

    private function initKLine($iID, $rds, $db)
    {
        $key = "CURRENT_BLOCK_STORE_" . $iID;
        $rds->set($key, "");

        // 获取最新K线
        $sql = "SELECT * FROM `kline` WHERE `instrumnet_id` = '{$iID}' ORDER BY `id` DESC LIMIT 1";
        $st = $db->prepare($sql);
        $st->execute([]);
        $res = $st->fetchAll(PDO::FETCH_ASSOC);

        $last = null;
        if (count($res) > 0) $last = $res[0];
        if (!$last) return;

        // 判断K线计算状态
        $tickTime = $last['close_time'];
        $tickMsec = $last['close_msec'];
        $sql = "SELECT * FROM `tick` WHERE `instrumnet_id` = '{$iID}' AND `time` = '{$tickTime}' AND `msec` = {$tickMsec}";
        $st = $db->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $id = $res[0]['id'] + 1;

        $sql = "SELECT * FROM `tick` WHERE `id` = {$id}";
        $st = $db->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $open = $res[0];
        if (!$open) return;

        $sql = "SELECT MAX(`price`) AS `max`, MIN(`price`) AS `min` FROM `tick` WHERE `id` >= {$open['id']} AND `instrumnet_id` = '{$iID}'";
        $st = $db->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $res = $res[0];
        $max = $res['max'];
        $min = $res['min'];

        $openDate = date("Ymd", strtotime($open['time']));
        $openTime = date("H:i:s", strtotime($open['time']));
        $openMsec = $open['msec'];

        $data = array(
            $last['index'] + 1, $open['price'], $openDate, $openTime, $openMsec, $max, $min,
        );
        $dataStr = implode("_", $data);
        $rds->set($key, $dataStr);
    }

    private function initTradeStatus($iID, $rds)
    {
        $rds->set("TRADE_STATUS_" . $iID, 0);
    }

    private function initKLineTick($iID, $rds, $db)
    {
        $key = "MARKOV_HISTORY_KLINE_TICK_" . $iID;
        var_dump($key);
        $rds->set($key, "");
        $cnt = $this->peroid + 1;
        $sql = "SELECT `close_price` FROM `kline` WHERE `instrumnet_id` = '{$iID}' ORDER BY `id` DESC LIMIT {$cnt}";
        $st = $db->prepare($sql);
        $st->execute([]);
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $tick = [];
        foreach ($res as $value) {
            array_unshift($tick, $value['close_price']);
        }
        $dataStr = implode("_", $tick);
        echo $rds->set($key, $dataStr);
    }
}

$h = new InitSys();
$h->run();
