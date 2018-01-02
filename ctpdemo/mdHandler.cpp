#include <iostream>
#include "mdHandler.h"

MdHandler::MdHandler(CThostFtdcMdApi* api, const char* brokerId, const char* userId, const char* password) {
	this->api = api;
	this->brokerId = brokerId;
	this->userId = userId;
	this->password = password;
}

MdHandler::~MdHandler() {
}

void MdHandler::OnFrontConnected() {
	std::cout << "market front connected" << std::endl;
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));

	strcpy_s(req.BrokerID, brokerId);
	strcpy_s(req.UserID, userId);
	strcpy_s(req.Password, password);
	
	int ret = api->ReqUserLogin(&req, 0);
}

void MdHandler::OnFrontDisconnected(int nReason) {
	std::cout << "disconnected: " << nReason << std::endl;
}

void MdHandler::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "logged in" << std::endl;
	std::cout << "trading day: " << pRspUserLogin->TradingDay << std::endl;
	std::cout << "system name: " << pRspUserLogin->SystemName << std::endl;
	std::cout << "session id: " << pRspUserLogin->SessionID << std::endl;
	std::cout << "request id:" << nRequestID << std::endl;
	std::cout << "ErrorID: " << pRspInfo->ErrorID << std::endl;

	char contract[] = "rb1805";
	char* contracts[] = { contract };
	int ret = api->SubscribeMarketData(contracts, 1);
	
	std::cout << ret << std::endl;
}

void MdHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "OnRspSubMarketData: " << pRspInfo->ErrorMsg << std::endl;
}

void MdHandler::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << "OnRspUnSubMarketData: " << pRspInfo->ErrorMsg << std::endl;
}

void MdHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) {
	std::cout << pDepthMarketData->InstrumentID << ": LastPrice is " << pDepthMarketData->LastPrice;
	std::cout << " AskPrice1 is " << pDepthMarketData->AskPrice1 << std::endl;
}

void MdHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	std::cout << pRspInfo->ErrorID << std::endl;
}