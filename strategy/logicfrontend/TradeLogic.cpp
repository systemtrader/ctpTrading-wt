#include "TradeLogic.h"
#include <vector>

TradeLogic::TradeLogic(int count)
{
    _kLineCount = count;
    _max = _min = _mean = 0;
    _store = new Redis("127.0.0.1", 6379, 1);
    _openIndex = -1;
}

TradeLogic::~TradeLogic()
{
    delete _store;
}

void TradeLogic::init()
{
    string res;
    KLineBlock tmp;
    while(1) {
        res = _store->pop("HISTORY_KLINE");
        tmp = KLineBlock::make(res);
        _bList.push_front(tmp);
    }
}

void TradeLogic::onKLineOpen()
{
    int status = _getStatus();
    switch (status) {
        // 开仓
        case TRADE_STATUS_NOTHING:
        case TRADE_STATUS_BUYOPENING:
        case TRADE_STATUS_SELLOPENING:
            _closeAction = CLOSE_ACTION_OPEN;
            _calculateOpen();
            break;

        case TRADE_STATUS_BUYOPENED:
        case TRADE_STATUS_SELLCLOSING:
            _closeAction = CLOSE_ACTION_SELLCLOSE;
            _calculateSellClose();
            break;

        case TRADE_STATUS_SELLOPENED:
        case TRADE_STATUS_BUYCLOSING:
            _closeAction = CLOSE_ACTION_BUYCLOSE;
            _calculateBuyClose();
            break;

        default:
            break;
    }
}

void TradeLogic::onKLineClose(KLineBlock block)
{
    _bList.push_front(block);
    switch (_closeAction) {

        case CLOSE_ACTION_OPEN:
            if (block.getClosePrice() > _max && block.getClosePrice() > _mean) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_BUYOPENED));
            }
            if (block.getClosePrice() < _min && block.getClosePrice() < _mean) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_SELLOPENED));
            }
            _openIndex = block.getIndex();
            break;

        case CLOSE_ACTION_SELLCLOSE:
            break;
        case CLOSE_ACTION_BUYCLOSE:
            break;
        default:
            break;
    }
}

int TradeLogic::_getStatus()
{
    string status = _store->get("TRADE_STATUS");
    return Lib::stoi(status);
}

void TradeLogic::_calculateOpen()
{
    double * maxArr, * minArr;
    int count = min(_kLineCount, (int)_bList.size());
    if (count == 0) return;

    maxArr = (double*) malloc(count * sizeof(double));
    minArr = (double*) malloc(count * sizeof(double));

    list<KLineBlock>::iterator item = _bList.begin();
    int cnt = count, i = 0;
    while (1) {
        *(maxArr + i) = (*item).getMaxPrice();
        *(minArr + i) = (*item).getMinPrice();
        if (--cnt == 0) break;
        item++;
        i++;
    }

    _max  = Lib::max(maxArr, count);
    _min  = Lib::min(minArr, count);
    _mean = Lib::mean(minArr, count);

    free(maxArr);
    free(minArr);
}

void TradeLogic::_calculateBuyClose()
{

}

void TradeLogic::_calculateSellClose()
{

}

