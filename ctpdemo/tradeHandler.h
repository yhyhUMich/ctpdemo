#pragma once
#include "ThostFtdcTraderApi.h"

class TradeHandler : public CThostFtdcTraderSpi {

public:
	TradeHandler(CThostFtdcTraderApi* api, const char* brokerId, const char* userId, const char* password);
	~TradeHandler();

	void ReqSettlementInfoConfirm();
	void ReqQryTradingAccount();
	void ReqQryInvestorPositionDetail();
	void ReqOrderInsert(TThostFtdcInstrumentIDType instId, TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp, TThostFtdcPriceType price, TThostFtdcVolumeType vol);
	void ReqOrderAction(TThostFtdcSequenceNoType orderSeq);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	void OnFrontConnected();
	void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRtnOrder(CThostFtdcOrderField *pOrder);
	void OnRtnTrade(CThostFtdcTradeField *pTrade);
	void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

private:
	CThostFtdcTraderApi * api;
	const char* brokerId;
	const char* userId;
	const char* password;
	int requestId = 0;
};