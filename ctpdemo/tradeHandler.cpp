#pragma warning(disable :4996)

#include <iostream>
#include <vector>

#include "tradeHandler.h"

// �Ự����
int	 frontId;	//ǰ�ñ��
int	 sessionId;	//�Ự���
char orderRef[13];

char MapDirection(char src, bool toOrig);
char MapOffset(char src, bool toOrig);

std::vector<CThostFtdcOrderField*> orderList;
std::vector<CThostFtdcTradeField*> tradeList;

TradeHandler::TradeHandler(CThostFtdcTraderApi* api, const char* brokerId, const char* userId, const char* password) {
	this->api = api;
	this->brokerId = brokerId;
	this->userId = userId;
	this->password = password;
}

TradeHandler::~TradeHandler() {
}

void TradeHandler::OnFrontConnected() {
	std::cout << "trade front connected" << std::endl;
	
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, brokerId);
	strcpy_s(req.UserID, userId);
	strcpy_s(req.Password, password);
	int ret = api->ReqUserLogin(&req, 0);
}

void TradeHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		// ����Ự����	
		frontId = pRspUserLogin->FrontID;
		sessionId = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf(orderRef, "%d", ++nextOrderRef);

		std::cout << " response | login success...tradingday:" << pRspUserLogin->TradingDay << std::endl;
	}
}

void TradeHandler::ReqSettlementInfoConfirm() {
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, brokerId);
	strcpy(req.InvestorID, userId);
	int ret = api->ReqSettlementInfoConfirm(&req, ++requestId);

	std::cout << " request | SettlementInfo Confirmation..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void TradeHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm) {
		std::cout << " response | Settlement..."
			<< pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...confirmed" << std::endl;
	}
}

void TradeHandler::ReqQryTradingAccount() {
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, brokerId);
	strcpy(req.InvestorID, userId);
	int ret = api->ReqQryTradingAccount(&req, ++requestId);
	std::cout << " request | trading account query..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void TradeHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount) {
		double FrozenCash = pTradingAccount->FrozenCommission + pTradingAccount->FrozenMargin;

		std::cout << " response | Ȩ��:" << pTradingAccount->Balance
			<< " ����:" << pTradingAccount->Available
			<< " ��֤��:" << pTradingAccount->CurrMargin
			<< " ƽ��ӯ��:" << pTradingAccount->CloseProfit
			<< " �ֲ�ӯ��" << pTradingAccount->PositionProfit
			<< " ������:" << pTradingAccount->Commission
			<< " ���ᱣ֤��:" << pTradingAccount->FrozenMargin
			<< " ����������:" << pTradingAccount->FrozenCommission
			<< " �����ʽ�:" << pTradingAccount->FrozenCash
			<< std::endl;
	}
}

void TradeHandler::ReqQryInvestorPositionDetail() {
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, brokerId);
	strcpy(req.InvestorID, userId);
	int ret = api->ReqQryInvestorPositionDetail(&req, ++requestId);

	std::cout << " request | send position query..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void TradeHandler::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail) {
		std::cout << " response | ��Լ:" << pInvestorPositionDetail->InstrumentID
			<< " ����:" << pInvestorPositionDetail->Direction
			<< " ����:" << pInvestorPositionDetail->Volume
			<< " �ɽ�����:" << pInvestorPositionDetail->CloseVolume
			<< " �۸�:" << pInvestorPositionDetail->OpenPrice
			<< " ����:" << pInvestorPositionDetail->OpenDate
			<< " ������:" << pInvestorPositionDetail->TradingDay
			<< std::endl;
	}
}

void TradeHandler::ReqOrderInsert(TThostFtdcInstrumentIDType instId, TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp, TThostFtdcPriceType price, TThostFtdcVolumeType vol) {

	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, brokerId);		//Ӧ�õ�Ԫ����	
	strcpy(req.InvestorID, userId);		//Ͷ���ߴ���	
	strcpy(req.InstrumentID, instId);	//��Լ����	
	strcpy(req.OrderRef, orderRef);		//��������
	int nextOrderRef = atoi(orderRef);
	sprintf(orderRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;			//�۸�����=�޼�	
	req.Direction = MapDirection(dir, true);				//��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0], true);		//THOST_FTDC_OF_Open; //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;		//���Ͷ���ױ���־
	req.LimitPrice = price;									//�۸�
	req.VolumeTotalOriginal = vol;							//����	
	req.TimeCondition = THOST_FTDC_TC_GFD;					//��Ч������:������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV;					//�ɽ�������:�κ�����
	req.MinVolume = 1;										//��С�ɽ���:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;	//��������:����
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;									//�Զ������־:��	
	req.UserForceClose = 0;									//�û�ǿ����־:��

	int ret = api->ReqOrderInsert(&req, ++requestId);

	std::cout << " request | send order..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void TradeHandler::OnRtnOrder(CThostFtdcOrderField *pOrder) {
	CThostFtdcOrderField* order = new CThostFtdcOrderField();
	memcpy(order, pOrder, sizeof(CThostFtdcOrderField));

	bool founded = false;
	unsigned int i = 0;

	for (i = 0; i < orderList.size(); i++) {
		if (orderList[i]->BrokerOrderSeq == order->BrokerOrderSeq) {
			founded = true;
			break;
		}
	}

	if (founded) 
		orderList[i] = order;
	else
		orderList.push_back(order);
	

	std::cout << " response | order submitted...sequence:" << order->BrokerOrderSeq << "--" << order->OrderStatus << "--" << order->VolumeTotalOriginal << "--" << order->VolumeTotal << std::endl;
}

void TradeHandler::ReqOrderAction(TThostFtdcSequenceNoType orderSeq) {
	
	bool found = false;
	unsigned int i = 0;
	for (i = 0; i < orderList.size(); i++) {
		if (orderList[i]->BrokerOrderSeq == orderSeq) {
			found = true; 
			break;
		}
	}
	
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, brokerId);
	strcpy(req.InvestorID, userId);
	strcpy(req.ExchangeID, orderList[i]->ExchangeID);
	strcpy(req.OrderSysID, orderList[i]->OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;

	int ret = api->ReqOrderAction(&req, ++requestId);

	std::cout << " request | send cancel order..." << ((ret == 0) ? "success" : "fail") << std::endl;
}

void TradeHandler::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction) {
		std::cout << " response | cancel order success..."
			<< "exchange id:" << pInputOrderAction->ExchangeID
			<< "order system id:" << pInputOrderAction->OrderSysID << std::endl;
	}
}

void TradeHandler::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	CThostFtdcTradeField* trade = new CThostFtdcTradeField();
	memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));

	bool founded = false;
	unsigned int i = 0;
	for (i = 0; i < tradeList.size(); i++) {
		if (tradeList[i]->TradeID == trade->TradeID) {
			founded = true;
			break;
		}
	}
	if (founded)
		tradeList[i] = trade;
	else
		tradeList.push_back(trade);

	std::cout << " response | order has been traded...trader sequence:" << trade->TradeID << "--" << trade->Direction << "--" << trade->OffsetFlag << "--" << trade->Volume << "--" << trade->TradeType << "--" << trade->HedgeFlag << std::endl;
}
//���յ��������Ϣ
void TradeHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	IsErrorRspInfo(pRspInfo);
}

//�����������Ϣ
bool TradeHandler::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo) {
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	
	if (ret) {
		std::cout << " response | " << pRspInfo->ErrorMsg << "--" << pRspInfo->ErrorID << std::endl;
	}

	return ret;
}

char MapDirection(char src, bool toOrig = true) {
	if (toOrig) {
		if ('b' == src || 'B' == src)		
			src = '0';
		else if ('s' == src || 'S' == src)		
			src = '1';
	}
	else {
		if ('0' == src)		
			src = 'B';		
		else if ('1' == src)		
			src = 'S';		
	}

	return src;
}
char MapOffset(char src, bool toOrig = true) {
	if (toOrig) {
		if ('o' == src || 'O' == src) 
			src = '0';
		else if ('c' == src || 'C' == src)
			src = '1';		
		else if ('j' == src || 'J' == src)
			src = '3';
	}
	else {
		if ('0' == src)		
			src = 'O';		
		else if ('1' == src)		
			src = 'C';		
		else if ('3' == src)		
			src = 'J';		
	}
	return src;
}