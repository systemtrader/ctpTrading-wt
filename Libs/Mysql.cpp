#include <iostream>
#include <cstdlib>
#include <vector>
#include "Mysql.h"
using namespace std;

Mysql::Mysql()
{
    connection = mysql_init(NULL); // 初始化数据库连接变量
    if(connection == NULL)
    {
        cout << "Error1:" << mysql_error(connection);
        exit(1);
    }
}

Mysql::~Mysql()
{
    cout << "~" << endl;
    if(connection != NULL)  // 关闭数据库连接
    {
        mysql_close(connection);
    }
}

bool Mysql::initDB(string host, string user, string pwd, string db_name)
{
    // 函数mysql_real_connect建立一个数据库连接
    // 成功返回MYSQL*连接句柄，失败返回NULL
    connection = mysql_real_connect(connection, host.c_str(),
            user.c_str(), pwd.c_str(), db_name.c_str(), 0, NULL, 0);
    if(connection == NULL)
    {
        cout << "Error2:" << mysql_error(connection);
        exit(1);
    }
    return true;
}

vector< vector<string> > Mysql::findBySQL(string sql)
{
    std::vector< vector<string> > list;
    vector<string> tmp;
    // mysql_query()执行成功返回0，失败返回非0值。与PHP中不一样
    if(mysql_query(connection, sql.c_str()))
    {
        cout << "Query Error:" << mysql_error(connection);
        exit(1);
    }
    else
    {
        result = mysql_store_result(connection); // 获取结果集

        while (true) {
            // 获取下一行
            row = mysql_fetch_row(result);
            if(row <= 0) {
                break;
            }
            // mysql_num_fields()返回结果集中的字段数
            for(int j=0; j < mysql_num_fields(result); ++j)
            {
                tmp.push_back(row[j] ? string(row[j]) : "NULL");
            }
            list.push_back(tmp);
            tmp.clear();
        }
        // 释放结果集的内存
        mysql_free_result(result);
    }
    return list;
}
