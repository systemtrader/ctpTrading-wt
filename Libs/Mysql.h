#ifndef MYBD_H__
#include <iostream>
#include <string>
#include <vector>
#include <mysql/mysql.h>

using namespace std;

class Mysql
{
public:

    Mysql();
    ~Mysql();
    bool initDB(string host, string user, string pwd, string db_name);
    vector< vector<string> > findBySQL(string sql);

private:

    MYSQL *connection;
    MYSQL_RES *result;
    MYSQL_ROW row;

};

#endif
