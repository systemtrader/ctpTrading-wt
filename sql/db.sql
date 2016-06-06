CREATE TABLE `kline` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instrumnet_id` varchar(50) NOT NULL DEFAULT '',
  `index` int(11) NOT NULL DEFAULT '0',
  `open_time` datetime NOT NULL,
  `open_msec` int(11) NOT NULL DEFAULT 0,
  `close_time` datetime NOT NULL,
  `close_msec` int(11) NOT NULL DEFAULT 0,
  `open_price` decimal(10,0) NOT NULL DEFAULT '0',
  `close_price` decimal(10,0) NOT NULL DEFAULT '0',
  `max_price` decimal(10,0) NOT NULL DEFAULT '0',
  `min_price` decimal(10,0) NOT NULL DEFAULT '0',
  `volume` int(11) NOT NULL DEFAULT '0',
  `type` int(11) NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB CHARSET=utf8;

CREATE TABLE `markov_kline_order` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instrumnet_id` varchar(50) NOT NULL DEFAULT '',
  `order_id` int(11) NOT NULL DEFAULT 0 COMMENT '下单模块生成的自增id',
  `kindex` int(11) NOT NULL DEFAULT 0 COMMENT 'K线索引',
  `is_forecast` int(11) NOT NULL DEFAULT '0' COMMENT '是否是预测单',
  `is_main` int(11) NOT NULL DEFAULT '0' COMMENT '是否是主线单',
  `cancel_type` int(11) NOT NULL DEFAULT '0' COMMENT '撤单类型 1：超时撤单 2：预判撤单',
  `is_zhuijia` int(11) NOT NULL DEFAULT '0' COMMENT '是否是追加单',
  `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB CHARSET=utf8;

alter table `markov_kline_order`
  add `is_forecast` int(11) NOT NULL DEFAULT '0' COMMENT '是否是预测单',
  add `is_main` int(11) NOT NULL DEFAULT '0' COMMENT '是否是主线单',
  add `cancel_type` int(11) NOT NULL DEFAULT '0' COMMENT '撤单类型 1：超时撤单 2：预判撤单',
  add `is_zhuijia` int(11) NOT NULL DEFAULT '0' COMMENT '是否是追加单',
  add `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP;


CREATE TABLE `tick` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instrumnet_id` varchar(50) NOT NULL DEFAULT '',
  `time` datetime NOT NULL COMMENT '服务端返回时间',
  `msec` int(11) NOT NULL DEFAULT '0',
  `price` decimal(10,2) NOT NULL DEFAULT '0',
  `volume` int(11) NOT NULL DEFAULT '0',
  `bid_price1` decimal(10,2) NOT NULL DEFAULT '0',
  `bid_volume1` int(11) NOT NULL DEFAULT '0',
  `ask_price1` decimal(10,2) NOT NULL DEFAULT '0',
  `ask_volume1` int(11) NOT NULL DEFAULT '0',
  `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB CHARSET=utf8;


CREATE TABLE `order` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instrumnet_id` varchar(50) NOT NULL DEFAULT '',
  `order_id` int(11) NOT NULL DEFAULT 0,
  `front_id` int(11) NOT NULL DEFAULT 0,
  `session_id` int(11) NOT NULL DEFAULT 0,
  `order_ref` int(11) NOT NULL DEFAULT 0,
  `price` decimal(10,2) NOT NULL DEFAULT '0',
  `real_price` decimal(10,2) NOT NULL DEFAULT '0',
  `is_buy` int(1) NOT NULL DEFAULT 0,
  `is_open` int(11) NOT NULL DEFAULT '0',
  `srv_insert_time` datetime NOT NULL COMMENT '服务器返回的insert时间',
  `srv_traded_time` datetime NOT NULL COMMENT '服务器返回的成交',
  `start_time` datetime NOT NULL COMMENT '发出交易指令时间',
  `start_usec` int(11) NOT NULL DEFAULT 0,
  `first_time` datetime NOT NULL COMMENT '首次得到交易回馈时间',
  `first_usec` int(11) NOT NULL DEFAULT 0,
  `end_time` datetime NOT NULL COMMENT '首次得到交易回馈时间',
  `end_usec` int(11) NOT NULL DEFAULT 0,
  `status` int(11) NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_front_session_ref` (`front_id`, `session_id`, `order_ref`)
) ENGINE=InnoDB CHARSET=utf8;


CREATE TABLE `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` varchar(20) NOT NULL DEFAULT '' COMMENT '用户ID',
  `password` varchar(32) NOT NULL DEFAULT '',
  `type` int(1) NOT NULL DEFAULT 0 COMMENT '0:admin, 1:实盘, 2:仿真',
  `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB CHARSET=utf8;



SELECT
    sum(p)
FROM
    (
        SELECT
            CASE
        WHEN is_buy = 1 THEN
            (0 - real_price)
        WHEN is_buy = 0 THEN
            real_price
        END AS p
        FROM
            `order`
        WHERE
            id > 98
        AND STATUS = 1
    ) AS tmp;




SELECT
    CASE
WHEN is_open = 0 THEN
    @l
WHEN is_open = 1 THEN
    @l :=@l + 1
END AS l,
 CASE
WHEN is_buy = 1 THEN
    (0 - real_price)
WHEN is_buy = 0 THEN
    real_price
END AS p
FROM
    `order` o,
    (SELECT @l := 0) tmp
WHERE
    id > 98
AND STATUS = 1;





SELECT
    SUM(p)
FROM
    (
        SELECT
            CASE
        WHEN is_open = 0 THEN
            @l
        WHEN is_open = 1 THEN
            @l :=@l + 1
        END AS l,
        CASE
    WHEN is_buy = 1 THEN
        (0 - real_price)
    WHEN is_buy = 0 THEN
        real_price
    END AS p
    FROM
        `order` o,
        (SELECT @l := 0) tmp
    WHERE
        id > 98
    AND STATUS = 1
    ) tmp2
GROUP BY
    l;


SELECT
    sum(g)
FROM
    (
        SELECT
            SUM(p) AS g
        FROM
            (
                SELECT
                    CASE
                WHEN is_open = 0 THEN
                    @l
                WHEN is_open = 1 THEN
                    @l :=@l + 1
                END AS l,
                CASE
            WHEN is_buy = 1 THEN
                (0 - real_price)
            WHEN is_buy = 0 THEN
                real_price
            END AS p
            FROM
                `order` o,
                (SELECT @l := 0) tmp
            WHERE
                id > 98
            AND STATUS = 1
            ) tmp2
        GROUP BY
            l
    ) tmp3;

SELECT
  m.order_id, m.is_forecast, m.is_main, m.cancel_type, m.is_zhuijia,
  o.price, o.real_price, o.is_buy, o.is_open, o.status, o.mtime
FROM
  markov_kline_order as m,
  `order` as o
WHERE
  m.order_id = o.order_id
  and m.id > 41769
  and o.id > 4260;
