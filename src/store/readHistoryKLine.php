<?php

/**
*
*/
class ReadHistory
{

    function __construct()
    {
        $res = parse_ini_file('../../etc/config.ini');
        if ($res['is_dev']) {
            $mysqldb = $res['mysql_db_dev'];
            $rdsdb = $res['rds_db_dev'];
        } else {
            $mysqldb = $res['mysql_db_online'];
            $rdsdb = $res['rds_db_online'];
        }
        $this->mysql = new PDO("mysql:dbname={$mysqldb};host=127.0.0.1", "root", "Abc518131!");
        $this->rds = new Redis();
        $this->rds->connect('127.0.0.1', 6379);
        $this->rds->select($rdsdb);
    }

    public function run()
    {
        $sql = "SELECT * FROM `kline` ORDER BY `id` DESC";
        $st = $this->mysql->prepare($sql);
        $st->execute(array());
        $res = $st->fetchAll(PDO::FETCH_ASSOC);
        foreach ($res as $line) {
            unset($line['id']);
            $dataStr = implode('_', $line);
            $res = $this->rds->lPush("HISTORY_KLINE", $dataStr);
        }
    }
}

$h = new ReadHistory();
$h->run();
