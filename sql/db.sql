CREATE TABLE `kline2` (
  `id` int(11) NOT NULL,
  `index` int(11) NOT NULL DEFAULT '0',
  `open_time` datetime NOT NULL,
  `close_time` datetime NOT NULL,
  `open_price` decimal(10,0) NOT NULL DEFAULT '0',
  `close_price` decimal(10,0) NOT NULL DEFAULT '0',
  `max_price` decimal(10,0) NOT NULL DEFAULT '0',
  `min_price` decimal(10,0) NOT NULL DEFAULT '0',
  `volumn` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB CHARSET=utf8;


CREATE TABLE `tick` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL COMMENT '服务端返回时间',
  `price` decimal(10,0) NOT NULL DEFAULT '0',
  `volume` int(11) NOT NULL DEFAULT '0',
  `ctime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `local_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT '本地时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB CHARSET=utf8;
