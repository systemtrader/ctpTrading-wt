/**
 * 回测用，从文件读取历史数据，模拟market服务像k线服务发送tick
 */
#include "../global.h"
#include "../libs/Redis.h"

int main(int argc, char const *argv[])
{
    string fileName = string(argv[1]);

    parseIniFile("../etc/config.ini");
    int sid = getOptionToInt("trade_logic_service_id");
    QClient clt(sid, sizeof(MSG_TO_TRADE_LOGIC));

    MSG_TO_TRADE_LOGIC msg = {0};
    Tick tick = {0};

    std::vector<string> params;
    string line;

    ifstream handle;
    handle.open(fileName.c_str());
    Redis * rds = new Redis("127.0.0.1", 6379, 1);
    string tickStr;
    int i = 0;
    while (!handle.eof()) // To get you all the lines.
    {
        getline(handle, line); // Saves the line in STRING.
        if (line.length() <= 0) continue;
        params = Lib::split(line, "_");

        tick.price = Lib::stod(params[3]);
        tick.volume = Lib::stoi(params[6]);
        tick.bidPrice1 = Lib::stoi(params[3]);
        tick.askPrice1 = Lib::stoi(params[3]);

        tickStr = Lib::tickData2String(tick);
        rds->set("CURRENT_TICK", tickStr); // tick数据，供全局使用

        msg.block.index = i++;
        msg.block.open = Lib::stod(params[2]);
        msg.block.close = Lib::stod(params[3]);
        msg.block.max = Lib::stod(params[4]);
        msg.block.min = Lib::stod(params[5]);
        msg.msgType = MSG_KLINE_OPEN;
        clt.send((void *)&msg);
        msg.msgType = MSG_KLINE_CLOSE;
        clt.send((void *)&msg);
    }
    handle.close();
    // delete rds;
    return 0;
}
