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
	/*1. ʹ�ú��� CreateFtdcMdApi ���� CThostFtdcMdApi ��ʵ�������е�һ�������Ǳ������ļ����ɵ�Ŀ¼������
	��������ӿڻ��׽ӿ��ڱ������ɵ����ļ�����׺��Ϊ.con�����ļ��м�¼�ſͻ����յ������е�����������������
	�������������Ƿ�ʹ�� UDP ����ģʽ�� true ��ʾʹ�� UDP ģʽ�� false ��ʾʹ�� TCP ģʽ��*/
	CThostFtdcMdApi* mdApi = CThostFtdcMdApi::CreateFtdcMdApi("", true);

	/*2. ���� SPI ʵ���� MdHandler ���Լ������������࣬�̳��� CThostFtdcMdSpi��*/
	MdHandler md_handler(mdApi, brokerId, userId, password);
	
	/*3. �� API ʵ��ע�� SPI ʵ�� */
	mdApi->RegisterSpi(&md_handler);

	/*4. �� API ʵ��ע��ǰ�õ�ַ*/
	mdApi->RegisterFront(marketFront);

	/*5. ��ʼ������ӿڵĹ����̡߳���ʼ��֮���߳��Զ���������ʹ����һ����ע��ĵ�ַ���������������ӡ�*/
	mdApi->Init();
	
	//Trade handler
	/*1. ���� API ʵ��ʱ����ָ�����ݵĴ���Э�顣Ĭ��TCPЭ��*/
	CThostFtdcTraderApi* traderApi = CThostFtdcTraderApi::CreateFtdcTraderApi("");
	TradeHandler trade_handler(traderApi, brokerId, userId, password);
	traderApi->RegisterSpi(&trade_handler);
	traderApi->RegisterFront(tradeFront);

	/*2. ����˽���������������ض��ͻ��˷��͵���Ϣ���籨���ر����ɽ��ر���*/
	traderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	/*3. ���Ĺ������������������������ŵĿͻ��˷�������Ϣ������˵����Լ���Ͻ���״̬��*/
	traderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	/*����ģʽ
		 Restart���������н��������������͹����Լ�֮����ܻᷢ�͵����и�����Ϣ��
		 Resume�����տͻ����ϴζϿ����Ӻ����������͹����Լ�֮����ܻᷢ�͵����и�����Ϣ��
		 Quick�����տͻ��˵�¼֮���������ܻᷢ�͵����и�����Ϣ��*/

	traderApi->Init();

	std::this_thread::sleep_for(1s);
	//���㵥ȷ��
	trade_handler.ReqSettlementInfoConfirm();
	std::this_thread::sleep_for(1s);
	//��ѯ�˻�
	trade_handler.ReqQryTradingAccount();
	std::this_thread::sleep_for(1s);
	//��ѯ�ֲ�
	trade_handler.ReqQryInvestorPositionDetail();
	std::this_thread::sleep_for(1s);
	//����: ����� rb1805 1�� 3890
	trade_handler.ReqOrderInsert(const_cast<char*>("rb1805"), '0', const_cast<char*>("0"), 3890, 1);

	//Main Thread blocks here waiting for worker threads end 
	mdApi->Join();
	traderApi->Join();

	return 0;
}