#include <iostream>
#include <thread>
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcMdApi.h"

#include "mdHandler.h"
#include "tradeHandler.h"

using namespace std;

int main(int argc, void **argv) {

	//account information
	const char* brokerId = "9999";
	const char* userId = "110522";
	const char* password = "yh52yh";
	char marketFront[] = "tcp://218.202.237.33:10012";
	char tradeFront[] = "tcp://218.202.237.33:10002";

	//Marketdata handler
	/*1. 使用函数 CreateFtdcMdApi 创建 CThostFtdcMdApi 的实例。其中第一个参数是本地流文件生成的目录。流文
	件是行情接口或交易接口在本地生成的流文件，后缀名为.con。流文件中记录着客户端收到的所有的数据流的数量。第
	二个参数描述是否使用 UDP 传输模式， true 表示使用 UDP 模式， false 表示使用 TCP 模式。*/
	CThostFtdcMdApi* mdApi = CThostFtdcMdApi::CreateFtdcMdApi("", true);

	/*2. 创建 SPI 实例。 MdHandler 是自己创建的行情类，继承了 CThostFtdcMdSpi。*/
	MdHandler md_handler(mdApi, brokerId, userId, password);
	
	/*3. 向 API 实例注册 SPI 实例 */
	mdApi->RegisterSpi(&md_handler);

	/*4. 向 API 实例注册前置地址*/
	mdApi->RegisterFront(marketFront);

	/*5. 初始化行情接口的工作线程。初始化之后，线程自动启动，并使用上一步中注册的地址向服务端请求建立连接。*/
	mdApi->Init();
	
	//Trade handler
	/*1. 创建 API 实例时不能指定数据的传输协议。默认TCP协议*/
	CThostFtdcTraderApi* traderApi = CThostFtdcTraderApi::CreateFtdcTraderApi("");
	TradeHandler trade_handler(traderApi, brokerId, userId, password);
	traderApi->RegisterSpi(&trade_handler);
	traderApi->RegisterFront(tradeFront);

	/*2. 订阅私有流：交易所向特定客户端发送的信息。如报单回报，成交回报。*/
	traderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	/*3. 订阅公有流：交易所向所有连接着的客户端发布的信息。比如说：合约场上交易状态。*/
	traderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	/*订阅模式
		 Restart：接收所有交易所当日曾发送过的以及之后可能会发送的所有该类消息。
		 Resume：接收客户端上次断开连接后交易所曾发送过的以及之后可能会发送的所有该类消息。
		 Quick：接收客户端登录之后交易所可能会发送的所有该类消息。*/

	traderApi->Init();

	std::this_thread::sleep_for(1s);
	//结算单确认
	trade_handler.ReqSettlementInfoConfirm();
	std::this_thread::sleep_for(1s);
	//查询账户
	trade_handler.ReqQryTradingAccount();
	std::this_thread::sleep_for(1s);
	//查询持仓
	trade_handler.ReqQryInvestorPositionDetail();
	std::this_thread::sleep_for(1s);
	//报单: 开多仓 rb1805 1手 3890
	trade_handler.ReqOrderInsert(const_cast<char*>("rb1805"), '0', const_cast<char*>("0"), 3890, 1);

	//Main Thread blocks here waiting for worker threads end 
	mdApi->Join();
	traderApi->Join();

	return 0;
}