#include "../libs/Lib.h"
#include "../libs/Socket.h"

int main(int argc, char const *argv[])
{
    string fileName = string(argv[1]);

    ifstream handle;
    handle.open(fileName.c_str());

    int cfd = getCSocket("127.0.0.1", 5182);

    std::vector<string> params;
    string line;
    string msg;
    while (!handle.eof()) // To get you all the lines.
    {
        getline(handle, line); // Saves the line in STRING.
        if (line.length() <= 0) continue;
        params = Lib::split(line, ",");
        msg = "4__date_time_" + params[3] + "_" + params[4] + "_" + params[2];
        // cout << msg << endl;
        sendMsg(cfd, msg);
        usleep(500*1000);
    }
    close(cfd);
    handle.close();

    return 0;
}
