// #include "Mysql.h"
#include "Redis.h"

int main(int argc, char const *argv[])
{
    // Mysql * db = new Mysql();
    // db->initDB("test03.firstp2plocal.com", "tester", "tester123", "firstp2p_test");
    // std::vector< vector<string> > list = db->findBySQL("select * from firstp2p_bonus limit 2;");
    // delete db;
    // for (int i = 0; i < list.size(); ++i)
    // {
    //     for (int j = 0; j < list[i].size(); ++j)
    //     {
    //         /* code */
    //         cout << list[i][j] << "  ";
    //     }
    //     cout << endl;
    // }
    Redis * r = new Redis("127.0.0.1", 6379, 1);
    string res = r->execCmd("get 1f9bdfb48be31742add208695da89c47");
    cout << res << endl;

    return 0;
}
