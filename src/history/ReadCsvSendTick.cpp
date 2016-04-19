#include "../global.h"

int main(int argc, char const *argv[])
{
    string fileName = string(argv[1]);

    parseIniFile("../../etc/config.ini");
    int kLineSrvID  = getOptionToInt("k_line_service_id");
    QClient klineClient(kLineSrvID, sizeof(MSG_TO_KLINE));

    MSG_TO_KLINE msg = {0};
    msg.msgType = MSG_TICK;

    std::vector<string> params;
    string line;

    ifstream handle;
    handle.open(fileName.c_str());
    while (!handle.eof()) // To get you all the lines.
    {
        getline(handle, line); // Saves the line in STRING.
        if (line.length() <= 0) continue;
        params = Lib::split(line, ",");

        msg.tick.price = Lib::stod(params[3]);
        msg.tick.volume = Lib::stoi(params[4]);
        klineClient.send((void *)&msg);
    }
    handle.close();

    return 0;
}
