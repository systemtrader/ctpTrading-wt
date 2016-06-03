/**
 * 回测用，从文件读取历史数据，模拟market服务像k线服务发送tick
 */
#include "../global.h"
#include "../libs/Redis.h"

int main(int argc, char const *argv[])
{
    string fileName = string(argv[1]);

    int pIdx = Lib::stoi(string(argv[2]));
    int vIdx = Lib::stoi(string(argv[3]));
    int bIdx = Lib::stoi(string(argv[4]));
    int aIdx = Lib::stoi(string(argv[5]));

    parseIniFile("../etc/config.ini");
    int kLineSrvID  = getOptionToInt("k_line_service_id");
    int db = getOptionToInt("rds_db_dev");
    QClient klineClient(kLineSrvID, sizeof(MSG_TO_KLINE));

    MSG_TO_KLINE msg = {0};
    msg.msgType = MSG_TICK;

    std::vector<string> params;
    string line;

    ifstream handle;
    handle.open(fileName.c_str());

    Redis * rds = new Redis("127.0.0.1", 6379, db);
    string tickStr;
    while (!handle.eof()) // To get you all the lines.
    {
        getline(handle, line); // Saves the line in STRING.
        if (line.length() <= 0) continue;
        params = Lib::split(line, ",");

        msg.tick.price = Lib::stod(params[pIdx]);
        msg.tick.volume = Lib::stoi(params[vIdx]);
        msg.tick.bidPrice1 = Lib::stoi(params[bIdx]);
        msg.tick.askPrice1 = Lib::stoi(params[aIdx]);
        strcpy(msg.tick.instrumnetID, argv[6]);

        tickStr = Lib::tickData2String(msg.tick);
        rds->set("CURRENT_TICK", tickStr); // tick数据，供全局使用

        klineClient.send((void *)&msg);
    }
    handle.close();
    // delete rds;
    return 0;
}
