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
        $this->knum = $res['open_max_k_count'];
    }

    public function run()
    {
        $mysql = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");
        $rds = new Redis();
        $rds->connect('127.0.0.1', 6379);
        $rds->select($this->rdsdb);


        // 获取历史K线数据
        $sql = "SELECT * FROM `kline` ORDER BY `id` DESC LIMIT 1";
        $st = $mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);

        $last = null;
        if (count($res) > 0) $last = $res[0];

        // 判断K线计算状态
        $this->checkKlineStatus($rds, $mysql, $last);

        // 读取本地交易记录，设定交易状态
        $rds->set("TRADE_STATUS", 0);
    }

    public function checkKlineStatus($rds, $mysql, $last)
    {
        $rds->set("CURRENT_BLOCK_STORE", "");
        if (!$last) return;

        $tickTime = $last['close_time'];
        $tickMsec = $last['close_msec'];
        $sql = "SELECT * FROM `tick` WHERE `time` = '{$tickTime}' AND `msec` = {$tickMsec}";
        $st = $mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $id = $res[0]['id'] + 1;

        $sql = "SELECT * FROM `tick` WHERE `id` = {$id}";
        $st = $mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        $open = $res[0];
        if (!$open) return;

        $sql = "SELECT MAX(`price`) AS `max`, MIN(`price`) AS `min` FROM `tick` WHERE `id` >= {$open['id']}";
        $st = $mysql->prepare($sql);
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
        $rds->set("CURRENT_BLOCK_STORE", $dataStr);

    }
}

$h = new InitSys();
$h->run();
