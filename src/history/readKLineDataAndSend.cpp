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
    int db = getOptionToInt("rds_db_dev");
    string logPath = getOptionToString("log_path");
    QClient clt(sid, sizeof(MSG_TO_TRADE_LOGIC));

    MSG_TO_TRADE_LOGIC msg = {0};
    TickData tick = {0};

    std::vector<string> params;
    string line;

    ifstream handle;
    handle.open(fileName.c_str());
    Redis * rds = new Redis("127.0.0.1", 6379, db);
    string tickStr;
    int i = 0;
    while (!handle.eof()) // To get you all the lines.
    {
        getline(handle, line); // Saves the line in STRING.
        if (line.length() <= 0) continue;
        params = Lib::split(line, ",");

        tick.price = Lib::stod(params[3]);
        tick.volume = Lib::stoi(params[6]);
        tick.bidPrice1 = Lib::stoi(params[3]);
        tick.askPrice1 = Lib::stoi(params[3]);
        strcpy(tick.instrumnetID, argv[2]);

        tickStr = Lib::tickData2String(tick);
        rds->set("CURRENT_TICK", tickStr); // tick数据，供全局使用

        msg.block.index = i++;
        msg.block.open = Lib::stod(params[2]);
        msg.block.close = Lib::stod(params[3]);
        msg.block.max = Lib::stod(params[4]);
        msg.block.min = Lib::stod(params[5]);
        strcpy(msg.block.instrumnetID, argv[2]);
        msg.tick = tick;
        msg.msgType = MSG_KLINE_OPEN;
        clt.send((void *)&msg);
        // usleep(50*1000);

        while(true)
        {
            string statusStr = rds->get("TRADE_STATUS_" + string(argv[2]));
            int status = Lib::stoi(statusStr);
            if (status > 2) {
                usleep(1000);
            } else {
                break;
            }
        }

        ofstream info;
        Lib::initInfoLogHandle(logPath, info);
        info << "KLineSrv[close]";
        info << "|index|" << msg.block.index;
        info << "|open|" << msg.block.open;
        info << "|close|" << msg.block.close;
        info << endl;
        info.close();

        msg.msgType = MSG_KLINE_CLOSE;
        clt.send((void *)&msg);
    }
    handle.close();
    // delete rds;
    return 0;
}
