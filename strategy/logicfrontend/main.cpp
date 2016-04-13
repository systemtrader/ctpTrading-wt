#include "TradeLogic.h"

int main(int argc, char const *argv[])
{
    TradeLogic * tl = new TradeLogic(5);
    tl->onKLineOpen();
    return 0;
}
