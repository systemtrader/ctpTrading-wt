#include "Mysql.h"

int main(int argc, char const *argv[])
{
    Mysql * db = new Mysql();
    db->initDB("test03.firstp2plocal.com", "tester", "tester123", "firstp2p_test");
    std::vector< vector<string> > list = db->findBySQL("select * from firstp2p_bonus limit 2;");
    delete db;
    for (int i = 0; i < list.size(); ++i)
    {
        for (int j = 0; j < list[i].size(); ++j)
        {
            /* code */
            cout << list[i][j] << "  ";
        }
        cout << endl;
    }
    return 0;
}
