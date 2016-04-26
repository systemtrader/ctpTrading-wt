<?php

/**
*
*/
class Consumer
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

    public function popViaRds($key)
    {
        $data = $this->rds->rPop($key);
        if (empty($data)) return false;
        $data = explode('_', $data);
        return $data;
    }

    public function insertDB($sql, $data)
    {
        try {
            $st = $this->mysql->prepare($sql);
            $result = $st->execute($data);
            $re = $this->mysql->lastInsertId();
        } catch(Exception $e) {
            var_dump($e);
        }
        return $re;
    }

    public function updateDB($sql, $data)
    {
        try {
            $st = $this->mysql->prepare($sql);
            $re = $st->execute($data);
        } catch (Exception $e) {
            var_dump($e);
        }
        return $re;
    }


}
