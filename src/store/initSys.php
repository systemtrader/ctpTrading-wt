<?php

/**
* 读取历史K线
*/
class InitSys
{

    function __construct()
    {
        $res = parse_ini_file('/root/source/ctpTrading/etc/config.ini');
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

        // 获取历史K线数据
        $sql = "SELECT * FROM `kline` ORDER BY `id` DESC LIMIT {$this->knum}";
        $st = $mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);

        $rds = new Redis();
        $rds->connect('127.0.0.1', 6379);
        $rds->select($this->rdsdb);

        $last = null;
        foreach ($res as $line) {
            if (empty($last)) $last = $line;
            unset($line['id']);
            $dataStr = implode('_', $line);
            $res = $rds->lPush("HISTORY_KLINE", $dataStr);
        }

        // 读取本地交易记录
        $rds->set("TRADE_STATUS", 0);
        if ($last && $last['is_open'] == 1) { // 交易未关闭
            if ($last['is_buy'] == 1) {
                $rds->set("TRADE_STATUS", 1);
            } else {
                $rds->set("TRADE_STATUS", 2);
            }
        }
    }
}

$h = new InitSys();
$h->run();
