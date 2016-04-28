<?php
/**
 * 手动强平后，执行该脚本更新数据库状态
 */
require "consumer.php";


if (count($argv) != 4) {
    exit("参数错误 price isbuy isopen" . PHP_EOL);
}

$consumer = new Consumer();

$sql = "INSERT INTO `order` (`front_id`, `session_id`, `order_ref`, `price`, `is_buy`, `is_open`, `status`) VALUES (?, ?, ?, ?, ?, ?, ?)";
$params = array(-1, time(), time(), $argv[1], $argv[2], $argv[3], 1);
$consumer->insertDB($sql, $params);
