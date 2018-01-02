#pragma once

#include "ThostFtdcMdApi.h"

class MdHandler : public CThostFtdcMdSpi {

public:
	MdHandler(CThostFtdcMdApi *api, const char* brokerId, const char* userId, const char* password);
	~MdHandler();

	void OnFrontConnected();
	void OnFrontDisconnected(int nReason);
	void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);
	void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

private:
	CThostFtdcMdApi * api;
	const char* brokerId;
	const char* userId;
	const char* password;
};