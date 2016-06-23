<?php

/**
* 读取历史K线
*/
class Report
{

    private $title = ['序列号', '订单号', '系统单号', '合约', 'K线索引', '买卖', '开平', '订单类型', '报单时间', '最后成交时间/撤单时间', '报单价格', '成交价格', '报单手数', '未成交手数', '盈亏', '手续费', '系统响应耗时', '订单成交耗时', '详细状态'];
    private $infoTitle = ['合约', '盈亏', '下单总数', '成交', '平开', '预测单', '双开预测', '平开预测', '方向对的平开预测', '双开成功', '平开成功', '双开成功率', '平开成功率', '实时单', '实时成功', '实时平仓', '实时追价'];
    private $commission = [
        'sn1609' => 3.9,
        'hc1610' => 5.12,
    ];

    private $priceRadio = [
        'sn1609' => 1,
        'hc1610' => 10,
    ];

    function __construct($start, $end)
    {
        $this->mysqldb = 'ctp_1';

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
        $this->file = "/home/dev/ctpOrder/order_" . $this->file;
        $this->start = $start;
        $this->end = $end;
    }

    public function run()
    {
        $this->db = new PDO("mysql:dbname={$this->mysqldb};host=127.0.0.1", "root", "Abc518131!");
        $this->dbTick = new PDO("mysql:dbname=tick;host=127.0.0.1", "root", "Abc518131!");
        $sql = "SELECT
            m.order_id, m.instrumnet_id, m.kindex, o.is_buy, o.is_open, m.is_forecast, m.is_zhuijia, o.srv_insert_time, o.srv_traded_time, o.start_time, o.start_usec, o.first_time, o.first_usec, o.end_time, o.end_usec, o.price, o.real_price, m.cancel_type, o.status, o.session_id, o.front_id, o.order_ref
        FROM
            markov_kline_order as m,
            `order` as o
        WHERE
            m.order_id = o.order_id
            and o.start_time > '{$this->start}'
            and o.start_time < '{$this->end}'
            and o.status in (1, 2);";

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
            $tmp[] = "{$line['front_id']}:{$line['session_id']}:{$line['order_ref']}";
            $tmp[] = $line['instrumnet_id'];
            $tmp[] = $line['kindex'];
            $tmp[] = $line['is_buy'] ? 'buy' : 'sell';
            $tmp[] = $line['is_open'] ? 'kai' : 'ping';
            $tmp[] = $line['is_forecast'] ? ($line['kindex'] == -1 ? '强平单' : '预测单') : ($line['is_zhuijia'] ? '追价单' : '实时单');
            $tmp[] = $line['start_time'];
            $tmp[] = $line['end_time'];
            $tmp[] = $line['price'];
            $tmp[] = $line['real_price'];
            $tmp[] = $line['status'] == 1 ? 1 : 0;
            $tmp[] = $line['status'] != 1 ? 1 : 0;
            if ($line['is_open'] && $line['status'] == 1) {
                $openPrice[$line['instrumnet_id']] = $line['real_price'];
            }
            if (!$line['is_open'] && $line['status'] == 1) {
                $p = $line['real_price'] - $openPrice[$line['instrumnet_id']];
                if ($line['is_buy']) $p *= -1;
                $p = $p * $this->priceRadio[$line['instrumnet_id']];
                $p = $p - $this->commission[$line['instrumnet_id']];
                $tmp[] = $p;
                $totalPrice[$line['instrumnet_id']] = isset($totalPrice[$line['instrumnet_id']]) ? $totalPrice[$line['instrumnet_id']] + $p : $p;
                $tmp[] = $this->commission[$line['instrumnet_id']];
                $openPrice[$line['instrumnet_id']] = 0;
            } else {
                $tmp[] = 0;
                $tmp[] = 0;
            }

            $startTime = strtotime($line['start_time']) * 1000000 + $line['start_usec'];
            $firstTime = strtotime($line['first_time']) * 1000000 + $line['first_usec'];
            $endTime = strtotime($line['end_time']) * 1000000 + $line['end_usec'];

            $tmp[] = ($firstTime - $startTime)/1000;
            $tmp[] = ($endTime - $startTime)/1000;

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
                $sql = "SELECT * FROM `tick` WHERE `time` = '{$lastTime}' AND `instrumnet_id` = '{$line['instrumnet_id']}'";
                $st = $this->dbTick->prepare($sql);
                $st->execute([]);
                $res2 = $st->fetchAll(PDO::FETCH_ASSOC);
                $tickPrice = [];
                foreach ($res2 as $item) {
                    $tickPrice[] = $item['price'];
                }
                $report[$key][11] = implode(',', array_unique($tickPrice));
            }
        }

        // 计算统计信息
        foreach ($res as $line) {
            // 下单总数
            $total[$line['instrumnet_id']] = isset($total[$line['instrumnet_id']]) ? $total[$line['instrumnet_id']] + 1 : 1;
            // 成交单数
            if ($line['status'] == 1) {
                $traded[$line['instrumnet_id']] = isset($traded[$line['instrumnet_id']]) ? $traded[$line['instrumnet_id']] + 1 : 1;
            }
            // 预测单
            if ($line['is_forecast'] && $line['kindex'] >= 0) {
                $forecast[$line['instrumnet_id']] = isset($forecast[$line['instrumnet_id']]) ? $forecast[$line['instrumnet_id']] + 1 : 1;
                $forecastKline[$line['instrumnet_id']][$line['kindex']][] = $line['is_open'];
                $forecastKline[$line['instrumnet_id']][$line['kindex']][] = $line['status'];
            } else {
                $real[$line['instrumnet_id']] = isset($real[$line['instrumnet_id']]) ? $real[$line['instrumnet_id']] + 1 : 1;
                if ($line['status'] == 1) {
                    $realOK[$line['instrumnet_id']] = isset($realOK[$line['instrumnet_id']]) ? $realOK[$line['instrumnet_id']] + 1 : 1;
                    if (!$line['is_open']) {
                        if ($line['is_zhuijia']) $realZhuiOK[$line['instrumnet_id']] = isset($realZhuiOK[$line['instrumnet_id']]) ? $realZhuiOK[$line['instrumnet_id']] + 1 : 1;
                        else  $realCloseOK[$line['instrumnet_id']] = isset($realCloseOK[$line['instrumnet_id']]) ? $realCloseOK[$line['instrumnet_id']] + 1 : 1;
                    }

                }
            }
            if (!$line['is_open'] && $line['kindex'] >= 0) {
                $tmpType = 0;
                if ($line['is_forecast']) $tmpType = 1; // 预测
                else if ($line['is_zhuijia']) $tmpType = 3; // 追价
                else $tmpType = 2; // 实时
                $closeLine[$line['instrumnet_id']][$line['kindex']][] = $tmpType;
                $closeLine[$line['instrumnet_id']][$line['kindex']][] = intval($line['status']);
            }
        }

        foreach ($forecastKline as $iid => $kline) {
            foreach ($kline as $key => $item) {
                if ($item[0] == 1 && $item[2] == 1) {
                    $openopen[$iid] = isset($openopen[$iid]) ? $openopen[$iid] + 1 : 1;
                    if ($item[1] == 1 || $item[3] == 1) {
                        $openopenOK[$iid] = isset($openopenOK[$iid]) ? $openopenOK[$iid] + 1 : 1;
                    }
                } else {
                    $openclose[$iid] = isset($openclose[$iid]) ? $openclose[$iid] + 1 : 1;
                }
            }

        }

        foreach ($closeLine as $iid => $klineList) {
            foreach ($klineList as $key => $kline) {
                if (count($kline) > 2) {
                    $opencloseRight[$iid] = isset($opencloseRight[$iid]) ? $opencloseRight[$iid] + 1 : 1;
                } else {
                    if ($kline[1] == 1) {
                        $opencloseRight[$iid] = isset($opencloseRight[$iid]) ? $opencloseRight[$iid] + 1 : 1;
                        $opencloseOK[$iid] = isset($opencloseOK[$iid]) ? $opencloseOK[$iid] + 1 : 1;
                    }
                }
            }
        }

        $info[] = $this->infoTitle;
        foreach ($total as $iid => $item) {
            $tmp = [];
            $tmp[] = $iid;
            $tmp[] = $totalPrice[$iid] ?: 0;
            $tmp[] = $total[$iid] ?: 0;
            $tmp[] = $traded[$iid] ?: 0;
            $tmp[] = $traded[$iid] / 2 ?: 0;
            $tmp[] = $forecast[$iid] ?: 0;
            $tmp[] = $openopen[$iid] ?: 0;
            $tmp[] = $openclose[$iid] ?: 0;
            $tmp[] = $opencloseRight[$iid] ?: 0;
            $tmp[] = $openopenOK[$iid] ?: 0;
            $tmp[] = $opencloseOK[$iid] ?: 0;
            $tmp[] = $openopenOK[$iid] / $openopen[$iid] ?: 0;
            $tmp[] = $opencloseOK[$iid] / $opencloseRight[$iid] ?: 0;

            $tmp[] = $real[$iid] ?: 0;
            $tmp[] = $realOK[$iid] ?: 0;
            $tmp[] = $realCloseOK[$iid] ?: 0;
            $tmp[] = $realZhuiOK[$iid]  ?: 0;
            $info[] = $tmp;
        }

        array_unshift($report, $this->title);
        array_walk_recursive($report, function(&$item) {
            $item = iconv('utf8', 'gbk', $item);
        });
        // csv
        $fp = fopen($this->file . ".csv", 'w');
        foreach ($report as $fields) {
            fputcsv($fp, $fields);
        }
        fputcsv($fp, []);
        array_walk_recursive($info, function(&$item) {
            $item = iconv('utf8', 'gbk', $item);
        });
        foreach ($info as $fields) {
            fputcsv($fp, $fields);
        }
        fclose($fp);
    }

}

$start = isset($argv[1]) ? $argv[1] : false;
$end = isset($argv[2]) ? $argv[2] : false;

$h = new Report($start, $end);
$h->run();
