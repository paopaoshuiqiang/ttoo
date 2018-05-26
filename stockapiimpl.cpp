#include "stdafx.h"
#pragma once
//#include "stdafx.h"
//#include <string>
//#include <time.h>
#include <iostream>
//#include <fstream>
//#include <basetsd.h>
//#include <hash_map>
//#include <vector>
//#include <windows.h>
//#include <stdio.h>
//#include <mmsystem.h>
//#include <sstream>
//#include <math.h>
//#include <algorithm>
//#include "../Includes/TS_Singleton.h"
//#include "../BA_COMM/BA_FUNCTION.H"                 // 公用函数 
#include "stockapiimpl.h"
#include <string.h>

#include <iomanip>                                  // setprecision 放在最后防止cout is ambious

using namespace std;

//#ifdef WIN32
//#define GET_ACCURATE_TIME(v) 			\
//		{					\
//		SYSTEMTIME t;			\
//		GetLocalTime(&t);		\
//		v=t.wHour*3600000+		\
//		t.wMinute*60000+	\
//		t.wSecond*1000+		\
//		t.wMilliseconds;	\
//		}
//#else
//#define GET_ACCURATE_TIME(v)			\
//		{					\
//struct timeval t;		\
//	gettimeofday(&t,NULL);		\
//	tm *now=localtime(&t.tv_sec);		\
//	v=now->tm_hour*3600000+now->tm_min*60000+now->tm_sec*1000+		\
//	t.tv_usec/1000;		\
//		}
//#endif

extern const char * g_pMdAddress;
extern const char * g_pTdAddress;
extern const char * g_pTdUserID;
extern const char * g_pInvestorID;
extern const char * g_pTdPasswd;
extern const char * g_pMdUserID;
extern const char * g_pMdPasswd;
extern const char * g_pDepartmentID;
extern const char * g_pShareholderIDSH;
extern const char * g_pShareholderIDSZ;

CTradeSpi::CTradeSpi(CStockApi* pStockApi):m_pStockApi(pStockApi)
{


}

CTradeSpi::~CTradeSpi()
{

}
//////////////////////////////
//第一步
//接口初始化后 开始登陆账户  ->
/////////////////////////////
void CTradeSpi::OnFrontConnected()
{
	printf("CTradeSpi::OnFrontConnected\n");
	printf("Login to td\n");
	//已连接，发送登陆请求
	CTORATstpReqUserLoginField reqUserLoginField;
	memset(&reqUserLoginField, 0, sizeof(CTORATstpReqUserLoginField));
	strcpy_s(reqUserLoginField.LogInAccount, g_pTdUserID);
	reqUserLoginField.LogInAccountType = TORA_TSTP_LACT_UserID;
	strcpy_s(reqUserLoginField.Password, g_pTdPasswd);
	strcpy_s(reqUserLoginField.DepartmentID, g_pDepartmentID); //资金账户方式登录时必填,此时可不填
	strcpy_s(reqUserLoginField.UserProductInfo, "CPTZ");
	strcpy_s(reqUserLoginField.InterfaceProductInfo, "CPTZ");
	strcpy_s(reqUserLoginField.ProtocolInfo, "CPTZ");
	strcpy_s(reqUserLoginField.MacAddress, "");
	strcpy_s(reqUserLoginField.OneTimePassword, "");
	strcpy_s(reqUserLoginField.ClientIPAddress, "10.1.20.111");
	strcpy_s(reqUserLoginField.Lang, "zh_cn");
	strcpy_s(reqUserLoginField.TerminalInfo, "1123ni3498ebf");
	strcpy_s(reqUserLoginField.MacAddress, "12:34:EC:23:ED:3E");
	//调用登陆接口
	m_pStockApi->m_pTradeApi->ReqUserLogin(&reqUserLoginField,1);   //返回OnRspUserLogin 
}

void CTradeSpi::OnFrontDisconnected(int nReason)
{
	printf("CTradeSpi::OnFrontDisconnected\n");
}

////////////////////////////////////////
//第二部 登陆完之后查询持仓 ->
/////////////////////////////////////////
void CTradeSpi::OnRspUserLogin(CTORATstpRspUserLoginField *pRspUserLoginField, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pRspInfo!=nullptr){
		printf("CTradeSpi::OnRspUserLogin:ErrorID=%d,ErrorMsg=%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
	}

	if(!pRspInfo->ErrorID){
		m_pStockApi->UpdateSessionInfo(pRspUserLoginField->FrontID,pRspUserLoginField->SessionID,stoi(pRspUserLoginField->MaxOrderRef));
	}

	///查询仓位
	CTORATstpQryPositionField Qryfield;
	memset(&Qryfield, 0, sizeof(CTORATstpQryPositionField));
	strcpy_s(Qryfield.InvestorID, g_pInvestorID);
	//返回持仓记录，单次返回OnRspQryPosition
	m_pStockApi->m_pTradeApi->ReqQryPosition(&Qryfield, 2);  
}

void CTradeSpi::OnRspOrderInsert(CTORATstpInputOrderField *pInputOrderField, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if(pRspInfo!=nullptr&&pRspInfo->ErrorID!=0){
		printf("CTradeSpi::OnRspOrderInsert:%d,%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
		CTORATstpInputOrderField* pContent= new CTORATstpInputOrderField;
		(*pContent)=(*pInputOrderField);
		Message msg={RSPORDERINSERT,(void*)pContent};
		m_pStockApi->PostMessage(msg);
	}
}

void CTradeSpi::OnRspOrderAction(CTORATstpInputOrderActionField *pInputOrderActionField, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cout<<"撤单代码："<< pInputOrderActionField->OrderRef <<endl;
	cout<<endl;
	if(pRspInfo!=nullptr){
		
		printf("CTradeSpi::OnRspOrderAction:ErrorID=%d,ErrorMsg=%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
	}
}

///////////////////////////
//持仓查询    最后要查询委托
//////////////////////////

void CTradeSpi::OnRspQryPosition(CTORATstpPositionField *pPosition, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if(pRspInfo!=nullptr&&pRspInfo->ErrorID!=0){
		printf("CTradeSpi::OnRspQryPosition:ErrorID=%d,ErrorMsg=%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
		return;
	}
	if(pPosition){
		CTORATstpPositionField* pContent= new CTORATstpPositionField;
		(*pContent)=(*pPosition);
		Message msg={POSITION,(void*)pContent};
		m_pStockApi->PostMessage(msg);
	}
	if(bIsLast){
		
		CTORATstpQryOrderField Qryfield;
		memset(&Qryfield, 0, sizeof(CTORATstpQryOrderField));
		strcpy_s(Qryfield.InvestorID, g_pInvestorID);
		//委托查询
		m_pStockApi->m_pTradeApi->ReqQryOrder(&Qryfield, 2);
	}
}


void CTradeSpi::OnRspQryOrder(CTORATstpOrderField *pOrder, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if(pRspInfo!=nullptr&&pRspInfo->ErrorID!=0){
		printf("CTradeSpi::OnRspQryOrder:ErrorID=%d,ErrorMsg=%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
		return;
	}

	if(pOrder){
		if(pOrder->OrderStatus!=TORA_TSTP_OST_Canceled&&pOrder->OrderStatus!=TORA_TSTP_OST_AllTraded){
			m_pStockApi->Cancel(pOrder->SessionID,pOrder->FrontID,pOrder->ExchangeID,pOrder->SecurityID,pOrder->OrderRef);
		}
	}
	if(bIsLast){
		m_pStockApi->m_bReady=true;
	}
}

void CTradeSpi::OnRtnOrder(CTORATstpOrderField *pOrder) 
{
	printf("CTradeSpi::OnRtnOrder,OrderStatus=%c\n",pOrder->OrderStatus);
	CTORATstpOrderField* pContent= new CTORATstpOrderField;
	(*pContent)=(*pOrder);
	Message msg={RTNORDER,(void*)pContent};
	m_pStockApi->PostMessage(msg);
}

void CTradeSpi::OnRtnTrade(CTORATstpTradeField *pTrade)
{
	printf("CTradeSpi::OnRtnTrade\n");
	CTORATstpTradeField* pContent= new CTORATstpTradeField;
	(*pContent)=(*pTrade);
	Message msg={RTNTRADE,(void*)pContent};
	m_pStockApi->PostMessage(msg);
}

CMktSpi::CMktSpi(CStockApi* pStockApi):m_pStockApi(pStockApi)
{

}

CMktSpi::~CMktSpi()
{

}

void CMktSpi::OnRspSubMarketData(CTORATstpSpecificSecurityField *pSpecificSecurity, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if(pRspInfo!=nullptr){
		printf("CMktSpi::OnRspSubMarketData:%d,%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
	}
}

///接收行情
void CMktSpi::OnRtnDepthMarketData(CTORATstpMarketDataField *pDepthMarketData) 
{
	CTORATstpMarketDataField* pContent= new CTORATstpMarketDataField;
	(*pContent)=(*pDepthMarketData);
	Message msg={MKTDATA,(void*)pContent};
	m_pStockApi->PostMessage(msg);
}

void CMktSpi::OnFrontConnected()
{
	printf("CMktSpi::OnFrontConnected\n");
	printf("Login to md\n");
	CTORATstpReqUserLoginField loginField;
	memset(&loginField,0,sizeof(CTORATstpReqUserLoginField));
	strcpy_s(loginField.LogInAccount,g_pMdUserID);
	loginField.LogInAccountType=TORA_TSTP_LACT_UserID;
	strcpy_s(loginField.Password,g_pMdPasswd);
	int ret=m_pStockApi->m_pMdApi->ReqUserLogin(&loginField,1);
}

void CMktSpi::OnFrontDisconnected(int nReason)
{
	printf("CMktSpi::OnFrontDisconnected\n");
}

void CMktSpi::OnRspUserLogin(CTORATstpRspUserLoginField *pRspUserLogin, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pRspInfo!=nullptr){
		printf("CMktSpi::OnRspUserLogin:ErrorID=%d,ErrorMsg=%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
	}

	if(!pRspInfo->ErrorID){
		printf("Subscribe Market Data\n");
		char *p[]={"00000000"};
		m_pStockApi->m_pMdApi->SubscribeMarketData(p,1,TORA_TSTP_EXD_COMM);
	}else{
		printf("login md failed!\n");
	}
}

