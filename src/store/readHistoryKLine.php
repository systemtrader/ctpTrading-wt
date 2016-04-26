<?php

/**
*
*/
class ReadHistory
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
    }

    public function run()
    {
        $mysql = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");
        $sql = "SELECT * FROM `kline` ORDER BY `id` DESC";
        $st = $mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);

        $rds = new Redis();
        $rds->connect('127.0.0.1', 6379);
        $rds->select($this->rdsdb);

        foreach ($res as $line) {
            unset($line['id']);
            $dataStr = implode('_', $line);
            $res = $rds->lPush("HISTORY_KLINE", $dataStr);
        }
    }
}

$h = new ReadHistory();
$h->run();
