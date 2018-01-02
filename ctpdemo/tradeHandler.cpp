#pragma warning(disable :4996)

#include <iostream>
#include <vector>

#include "tradeHandler.h"

// 会话参数
int	 frontId;	//前置编号
int	 sessionId;	//会话编号
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

		// 保存会话参数	
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

		std::cout << " response | 权益:" << pTradingAccount->Balance
			<< " 可用:" << pTradingAccount->Available
			<< " 保证金:" << pTradingAccount->CurrMargin
			<< " 平仓盈亏:" << pTradingAccount->CloseProfit
			<< " 持仓盈亏" << pTradingAccount->PositionProfit
			<< " 手续费:" << pTradingAccount->Commission
			<< " 冻结保证金:" << pTradingAccount->FrozenMargin
			<< " 冻结手续费:" << pTradingAccount->FrozenCommission
			<< " 冻结资金:" << pTradingAccount->FrozenCash
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
		std::cout << " response | 合约:" << pInvestorPositionDetail->InstrumentID
			<< " 方向:" << pInvestorPositionDetail->Direction
			<< " 数量:" << pInvestorPositionDetail->Volume
			<< " 成交数量:" << pInvestorPositionDetail->CloseVolume
			<< " 价格:" << pInvestorPositionDetail->OpenPrice
			<< " 日期:" << pInvestorPositionDetail->OpenDate
			<< " 交易日:" << pInvestorPositionDetail->TradingDay
			<< std::endl;
	}
}

void TradeHandler::ReqOrderInsert(TThostFtdcInstrumentIDType instId, TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp, TThostFtdcPriceType price, TThostFtdcVolumeType vol) {

	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, brokerId);		//应用单元代码	
	strcpy(req.InvestorID, userId);		//投资者代码	
	strcpy(req.InstrumentID, instId);	//合约代码	
	strcpy(req.OrderRef, orderRef);		//报单引用
	int nextOrderRef = atoi(orderRef);
	sprintf(orderRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;			//价格类型=限价	
	req.Direction = MapDirection(dir, true);				//买卖方向	
	req.CombOffsetFlag[0] = MapOffset(kpp[0], true);		//THOST_FTDC_OF_Open; //组合开平标志:开仓
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;		//组合投机套保标志
	req.LimitPrice = price;									//价格
	req.VolumeTotalOriginal = vol;							//数量	
	req.TimeCondition = THOST_FTDC_TC_GFD;					//有效期类型:当日有效
	req.VolumeCondition = THOST_FTDC_VC_AV;					//成交量类型:任何数量
	req.MinVolume = 1;										//最小成交量:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;	//触发条件:立即
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//强平原因:非强平	
	req.IsAutoSuspend = 0;									//自动挂起标志:否	
	req.UserForceClose = 0;									//用户强评标志:否

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
//接收到报错的信息
void TradeHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	IsErrorRspInfo(pRspInfo);
}

//解析报错的信息
bool TradeHandler::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo) {
	// 如果ErrorID != 0, 说明收到了错误的响应
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