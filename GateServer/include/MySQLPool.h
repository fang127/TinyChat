#pragma once

#include "Const.h"

#include <cppconn/exception.h> // 定义了数据库操作的异常类​（如 sql::SQLException）。捕获和处理数据库操作中的错误（如连接失败、SQL语法错误、权限不足等）
#include <cppconn/prepared_statement.h> // 预编译语句执行器
#include <cppconn/resultset.h> // 查询结果集 接收SELECT语句的执行结果，提供遍历、获取字段值的方法
#include <cppconn/statement.h> // 普通SQL语句执行器
#include <mysql_connection.h>  // 数据库连接对象 管理连接，执行SQL语句
#include <mysql_driver.h> // 定义了MySQL驱动的核心接口（sql::mysql::MySQL_Driver类），是连接MySQL的“入口”
#include <thread>

class MySQLPool
{
};