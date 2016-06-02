<?php

class Order
{
    function __construct($iIDs)
    {
        $res = parse_ini_file(dirname(__FILE__) . '/../../etc/config.ini');
        if ($res['is_dev']) {
            $this->mysqldb = $res['mysql_db_dev'];
            $this->rdsdb = $res['rds_db_dev'];
        } else {
            $this->mysqldb = $res['mysql_db_online'];
            $this->rdsdb = $res['rds_db_online'];
        }
        $this->iIDs = $iIDs;
        $this->dbHost = $res['mysql_host'];
    }

    public function run()
    {
        $db = new PDO("mysql:dbname={$this->mysqldb};host={$this->dbHost}", "root", "Abc518131!");

        $sql = "SELECT * FROM "
    }
}
