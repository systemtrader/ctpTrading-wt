#include "../libs/Lib.h"
#include "../libs/Socket.h"
#include "../iniReader/iniReader.h"

int main(int argc, char const *argv[])
{
    string fileName = string(argv[1]);

    parseIniFile("../bin/config.ini");
    int          kLineSrvPort = getOptionToInt("k_line_srv_port");
    const char * kLineSrvIp   = getOptionToChar("k_line_srv_ip");
    int frequency = getOptionToInt("history_frequency");

    int cfd = getCSocket(kLineSrvIp, kLineSrvPort);

    std::vector<string> params;
    string line;
    string msg;

    ifstream handle;
    handle.open(fileName.c_str());
    while (!handle.eof()) // To get you all the lines.
    {
        getline(handle, line); // Saves the line in STRING.
        if (line.length() <= 0) continue;
        params = Lib::split(line, ",");
        msg = "4__date_time_" + params[3] + "_" + params[4] + "_" + params[2];
        // cout << msg << endl;
        sendMsg(cfd, msg);
        usleep(frequency*1000);
    }
    close(cfd);
    handle.close();

    return 0;
}
