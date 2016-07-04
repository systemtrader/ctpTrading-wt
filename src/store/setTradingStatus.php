<?php

/**
* 读取历史K线
*/
class Status
{

    function __construct($iID, $status)
    {
        $res = parse_ini_file(dirname(__FILE__) . '/../../etc/config.ini');
        if ($res['is_dev']) {
            $this->rdsdb = $res['rds_db_dev'];
        } else {
            $this->rdsdb = $res['rds_db_online'];
        }
        $this->iID = $iID;
        if (!in_array($status, ['stop', 'restart'])) exit("参数错误");
        if ($status == 'stop') $this->status = 2;
        if ($status == 'restart') $this->status = 3;
    }

    public function run()
    {
        $rds = new Redis();
        $rds->connect('127.0.0.1', 6379);
        $rds->select($this->rdsdb);

        $key = "TRADE_TAG_" . $this->iID;
        $rds->set($key, $this->status);
    }

}

$h = new Status($argv[1], $argv[2]);
$h->run();
