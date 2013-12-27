#创建数据库
create database if not exists `quality_test`;
#创建当前数据信息表，用于server重启后获取信息
use `quality_test`;
drop table if exists `cur_data_info`;
create table if not exists `cur_data_info` (
    `bid` int(11) UNSIGNED NOT NULL,
    `ip` varchar(255) NOT NULL,
    `port` SMALLINT UNSIGNED not null,
    `aid` int(11) unsigned not null,
    `pid` int(11) unsigned not null,
    `active` int(11) not null default 0,
    `update_time` DATETIME not null,
    `via_time` datetime not null,
    `counter` int(11) unsigned not null default 0,
    `delay` int(11) not null default - 1,
	`protocol_type` SMALLINT UNSIGNED not null default 0,
    PRIMARY KEY (`bid` , `ip`)
)  ENGINE=InnoDB DEFAULT CHARSET=utf8;
#创建测试历史信息记录表，数据分析，导出，报表
drop table if exists `history_data_info`;
create table if not exists `history_data_info` (
	`time_stamp` datetime not null,
	`bid` int(11) UNSIGNED NOT NULL,
	`ip` varchar(255) NOT NULL,
	`port` SMALLINT UNSIGNED not null,
	`aid` int(11) unsigned not null,
	`pid` int(11) unsigned not null,
	`active` int(11) not null default 0,
	`update_time` DATETIME not null,
	`via_time` datetime not null,
	`counter` int(11) unsigned not null default 0,
	`delay` int(11) not null default - 1,
	`protocol_type` SMALLINT UNSIGNED not null default 0,
	PRIMARY KEY (`time_stamp`,`bid` , `ip`,`port`)	
) ENGINE=InnoDB DEFAULT CHARSET=utf8 partition by HASH( day(time_stamp));

