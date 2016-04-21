<?php

/**
*
*/
class ReadHistory
{

    function __construct()
    {
        $this->mysql = new PDO("mysql:dbname=ctp;host=127.0.0.1", "root", "Abc518131!");
        $this->rds = new Redis();
        $this->rds->connect('127.0.0.1', 6379);
        $this->rds->select(1);
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
