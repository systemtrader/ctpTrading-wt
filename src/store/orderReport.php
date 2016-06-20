<?php

/**
* 读取历史K线
*/
class Report
{

    private $title = ['序列号', '订单号', '合约', 'K线索引', '买卖', '开平', '订单类型', '报单时间', '毫秒', '最后成交时间/撤单时间', '毫秒', '报单价格', '成交价格', '报单手数', '未成交手数', '盈亏', '手续费', '系统响应耗时', '订单成交耗时', '详细状态'];

    function __construct($start, $end)
    {
        $res = parse_ini_file(dirname(__FILE__) . '/../../etc/config.ini');
        $this->mysqldb = $res['mysql_db_online'];

        $today = date('Y-m-d', time());
        $yestoday = date('Y-m-d', strtotime('-1 day'));
        $this->file = $start . "_" . $end;
        if (empty($start)) {
            $time = intval(date('Hi', time()));
            if ($time >= 0 && $time <= 230) {
                $start = $yestoday . " 13:29";
                $end = $yestoday . " 15:01";
                $this->file = $yestoday . "_1330";
            }
            if ($time > 230 && $time <= 1130) {
                $start = $yestoday . " 20:59";
                $end = $today . " 02:31";
                $this->file = $yestoday . "_2100";
            }
            if ($time > 1130 && $time <= 1500) {
                $start = $today . " 08:59";
                $end = $today . " 11:31";
                $this->file = $today . "_0900";
            }
            if ($time > 1500 && $time <= 2359) {
                $start = $today . " 13:29";
                $end = $today . " 15:01";
                $this->file = $today . "_1330";
            }
        }
        $this->file = str_replace(" ", "_", $this->file);
        $this->file = "order_" . $this->file;
        $this->start = $start;
        $this->end = $end;
    }

    public function run()
    {
        $this->db = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");
        $sql = "SELECT
            m.order_id, m.instrumnet_id, m.kindex, o.is_buy, o.is_open, m.is_forecast, m.is_zhuijia, o.srv_insert_time, o.srv_traded_time, o.start_time, o.start_usec, o.first_time, o.first_usec, o.end_time, o.end_usec, o.price, o.real_price, m.cancel_type, o.status
        FROM
            markov_kline_order as m,
            `order` as o
        WHERE
            m.order_id = o.order_id
            and o.start_time > '{$this->start}'
            and o.start_time > '{$this->end}';";

        $st = $this->db->prepare($sql);
        $st->execute([]);
        $res = $st->fetchAll(PDO::FETCH_ASSOC);

        $report = [];
        $no = 1;
        // 初步处理
        foreach ($res as $line) {

            $tmp = [];
            $tmp[] = $no++;
            $tmp[] = $line['order_id'];
            $tmp[] = $line['instrumnet_id'];
            $tmp[] = $line['kindex'];
            $tmp[] = $line['is_buy'] ? 'buy' : 'sell';
            $tmp[] = $line['is_open'] ? 'kai' : 'ping';
            $tmp[] = $line['is_forecast'] ? '预测单' : $line['is_zhuijia'] ? '追价单' : '实时单';
            $tmp[] = $line['start_time'];
            $tmp[] = $line['end_time'];
            $tmp[] = $line['price'];
            $tmp[] = $line['real_price'];
            $tmp[] = $line['status'] == 1 ? 1 : 0;
            $tmp[] = $line['status'] != 1 ? 1 : 0;
            if ($line['is_open'] && $line['status'] == 1) {
                $openPrice = $line['real_price'];
            }
            if (!$line['is_open'] && $line['status'] == 1) {
                $p = $line['real_price'] - $openPrice;
                if (!$line['is_buy']) $p *= -1;
                $tmp[] = $p;
                $tmp[] = $price0;
                $openPrice = 0;
            } else {
                $tmp[] = 0;
                $tmp[] = 0;
            }

            $startTime = strtotime($line['start_time']) * 1000000 + $line['start_usec'];
            $firstTime = strtotime($line['first_time']) * 1000000 + $line['first_usec'];
            $endTime = strtotime($line['end_time']) * 1000000 + $line['end_usec'];

            $tmp[] = $firstTime - $startTime;
            $tmp[] = $endTime - $startTime;

            switch ($line['status']) {
                case 1:
                    $tmp[] = '全部成交';
                    break;

                case 2:
                    $tmp[] = '撤单';
                    break;

                default:
                    $tmp[] = '未知';
                    break;
            }
            $report[] = $tmp;
        }

        // 更新撤单tick价
        foreach ($res as $key => $line) {
            if ($line['real_price'] == 0) {
                $lastTime = $line['srv_insert_time'];
                if ($lastTime == '0000-00-00 00:00:00') continue;
                $sql = "SELECT * FROM `tick` WHERE `time` = '{$lastTime}'";
                $st = $this->db->prepare($sql);
                $st->execute([]);
                $res2 = $st->fetchAll(PDO::FETCH_ASSOC);
                $tickPrice = [];
                foreach ($res2 as $item) {
                    $tickPrice[] = $item['price'];
                }
                $report[$key][10] = $tickPrice;
            }
        }

        // csv
        $fp = fopen($this->file . ".csv", 'w');
        fputcsv($fp, $this->title);
        foreach ($report as $fields) {
            fputcsv($fp, $fields);
        }
        fclose($fp);
    }

}

$start = isset($argv[1]) ? $argv[1] : false;
$end = isset($argv[2]) ? $argv[2] : false;

$h = new Report($start, $end);
$h->run();
