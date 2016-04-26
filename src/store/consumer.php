<?php

/**
*
*/
class Consumer
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

    public function popViaRds($key)
    {
        $rds = new Redis();
        $rds->connect('127.0.0.1', 6379);
        $rds->select($this->rdsdb);

        $data = $rds->rPop($key);
        if (empty($data)) return false;
        $data = explode('_', $data);
        return $data;
    }

    public function insertDB($sql, $data)
    {
        try {
            $mysql = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");
            $st = $mysql->prepare($sql);
            $result = $st->execute($data);
            $re = $mysql->lastInsertId();
        } catch(Exception $e) {
            var_dump($e);
        }
        return $re;
    }

    public function updateDB($sql, $data)
    {
        try {
            $mysql = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");
            $st = $mysql->prepare($sql);
            $re = $st->execute($data);
        } catch (Exception $e) {
            var_dump($e);
        }
        return $re;
    }


}
