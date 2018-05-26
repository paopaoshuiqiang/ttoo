#include "stdafx.h"
#pragma once
#include "stockapiimpl.h"
#include <string.h>
//#include "stdafx.h"
//#include <string>
#include <time.h>
#include <iostream>
#include <fstream>
#include <basetsd.h>
#include <hash_map>
#include <vector>
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <sstream>
#include <math.h>
#include <algorithm>
#include "../Includes/TS_Singleton.h"
#include "../BA_COMM/BA_FUNCTION.H"                 // ���ú��� 

#include "QuickT.h"
using namespace std;

#ifdef WIN32
#define GET_ACCURATE_TIME(v) 			\
{					\
	SYSTEMTIME t;			\
	GetLocalTime(&t);		\
	v=t.wHour*3600000+		\
	t.wMinute*60000+	\
	t.wSecond*1000+		\
	t.wMilliseconds;	\
}
#else
#define GET_ACCURATE_TIME(v)			\
{					\
struct timeval t;		\
	gettimeofday(&t,NULL);		\
	tm *now=localtime(&t.tv_sec);		\
	v=now->tm_hour*3600000+now->tm_min*60000+now->tm_sec*1000+		\
	t.tv_usec/1000;		\
}
#endif

extern const char * g_pInvestorID;
extern const char * g_pShareholderIDSH;
extern const char * g_pShareholderIDSZ;


////////////////////////////////////////
//�ӿ�ʵ����
///////////////////////////////////////



///////////////////////////////////////
//�ӿڳ�ʼ �Լ���������
///////////////////////////////////////
void CStockApi::Init(char * pTdFrontAddress,char * pMdFrontAddress)
{
	///��ʼ������
	m_pMdApi=CTORATstpMdApi::CreateTstpMdApi();
	m_pMdApi->RegisterFront(pMdFrontAddress);
	m_pMdApi->RegisterSpi(new CMktSpi(this));
	m_pMdApi->Init();

	///��ʼ������
	m_pTradeApi = CTORATstpTraderApi::CreateTstpTraderApi();
	m_pTradeApi->RegisterSpi(new CTradeSpi(this));
	m_pTradeApi->RegisterFront(pTdFrontAddress);
	m_pTradeApi->SubscribePrivateTopic(TORA_TERT_QUICK);
	m_pTradeApi->SubscribePublicTopic(TORA_TERT_QUICK);
	m_pTradeApi->Init();

	///��ʼ��Ŀ���Ʊ����(����������+��Ʊ����) 1��ͷ���Ϻ� 2��ͷ����֤
	m_setTargetSecurity.insert("1600000");
	m_setTargetSecurity.insert("1601818");
	m_setTargetSecurity.insert("2000001");
	CThread::Create();
}



void CStockApi::RtnDepthMarketData(CTORATstpMarketDataField *pDepthMarketData)
{
	if(!m_bReady){
		return;
	}

	char key[16]={0};
	sprintf_s(key,"%c%s",pDepthMarketData->ExchangeID,pDepthMarketData->SecurityID);
	set<string>::iterator iterSet=m_setTargetSecurity.find(key);
	if(iterSet!=m_setTargetSecurity.end()){
		///�������
		printf("SecurityID=%s,LastPrice=%f\n",pDepthMarketData->SecurityID,pDepthMarketData->LastPrice);

		///�����ǵ���
		double r=(pDepthMarketData->LastPrice-pDepthMarketData->PreClosePrice)/pDepthMarketData->PreClosePrice;

		///�����;���������ֻ��Ʊ������;����ֹͣ��������
		map<string,list<CTORATstpOrderField>>::iterator iterMap=m_mapAppendOrder.find(pDepthMarketData->SecurityID);
		if(iterMap!=m_mapAppendOrder.end()){
			///���ί��ʱ��Holding Time
			int currentTime=0;
			GET_ACCURATE_TIME(currentTime);
			list<CTORATstpOrderField>::iterator iterList=iterMap->second.begin();
			while(iterList!=iterMap->second.end()){
				///����10��ͳ���
				if(currentTime-iterList->RequestID>10000){
					Cancel(iterList->SessionID,iterList->FrontID,iterList->ExchangeID,iterList->SecurityID,iterList->OrderRef);
				}
				iterList++;
			}

			///ֻҪ����;���Ͳ��ܼ�������
			if(iterMap->second.size()>0){
				return ;
			}
		}

		///���ֲ�,����ֲִ���1000000��ֹͣ����
		const CTORATstpPositionField *pPosition=GetPosition(pDepthMarketData->SecurityID);
		if(pPosition!=nullptr&&pPosition->CurrentPosition>=1000000){
			return;
		}

		if(pPosition!=nullptr){
			printf("%d,%d,%d\n",pPosition->CurrentPosition,pPosition->AvailablePosition,pPosition->TodayBSFrozen);
		}
		///�ǵ�����-1%~1%֮�������
		if((r>-0.01&&r<0.01)){
			LimitBuy(pDepthMarketData->ExchangeID,pDepthMarketData->SecurityID,pDepthMarketData->LastPrice,1000);
		}
	}
}



CStockApi::CStockApi()
{
	m_pTradeApi=nullptr;
	m_pMdApi=nullptr;
	m_bReady=false;
}

CStockApi::~CStockApi()
{
	m_nSessionID=0;
	m_nFrontID=0;
	m_nMaxOrderRef=0;
}

bool CStockApi::InitInstance()
{
	pPosManager= new CPositionManager(this);
	return true;
}

void CStockApi::ExitThread()
{

}

void CStockApi::Run()
{
	Message msg;
	while(true){
		///����Ϣ��Ҫ����
		if(PopMessage(msg)){
			switch(msg.msgType){
			case ORDERINSERT:{
				CTORATstpInputOrderField* pInputOrder=(CTORATstpInputOrderField*)msg.m_pContent;
				ReqOrderInsert(pInputOrder);
				delete pInputOrder;                
				break;
							 }
			case ORDERACTION:{
				CTORATstpInputOrderActionField* pInputOrderAction=(CTORATstpInputOrderActionField*)msg.m_pContent;
				ReqOrderAction(pInputOrderAction);
				delete pInputOrderAction;
				break;
							 }
			case RSPORDERINSERT:{
				CTORATstpInputOrderField* pInputOrder=(CTORATstpInputOrderField*)msg.m_pContent;
				RspOrderInsert(pInputOrder);
				delete pInputOrder;                
				break;
								}
			case RTNORDER:{
				CTORATstpOrderField* pOrder=(CTORATstpOrderField*)msg.m_pContent;
				RtnOrder(pOrder);
				delete pOrder;
				break;
						  }
			case RTNTRADE:{
				CTORATstpTradeField* pTrade=(CTORATstpTradeField*)msg.m_pContent;
				RtnTrade(pTrade);
				delete pTrade;
				break;
						  }
			case POSITION:{
				CTORATstpPositionField* pPosition=(CTORATstpPositionField*)msg.m_pContent;
				InitPosition(pPosition);
				delete pPosition;
				break;
						  }
			case MKTDATA:{
				CTORATstpMarketDataField* pMktData=(CTORATstpMarketDataField*)msg.m_pContent;
				RtnDepthMarketData(pMktData);
				delete pMktData;
				break;
						 }
			default:{
				break; 
					}                    
			}
		}
	}
}




void CStockApi::Release()
{
	m_pTradeApi->Release();
	m_pMdApi->Release();
}

void CStockApi::PostMessage(Message& msg)
{
	m_mtx.lock();
	m_msgQueue.push(msg);
	m_mtx.unlock();
}


bool CStockApi::PopMessage(Message& msg)
{
	if(m_msgQueue.empty()){
		return false;
	}
	m_mtx.lock();
	msg=m_msgQueue.front();
	m_msgQueue.pop();
	m_mtx.unlock();
	return true;
}

string CStockApi::GetMaxOrderRef()
{
	char maxOrderRef[13]={0};
	sprintf_s(maxOrderRef,"%012d",++m_nMaxOrderRef);
	return maxOrderRef;
}

const char *CStockApi::GetShareHolderID(TTORATstpExchangeIDType ExchangeID)
{
	return ExchangeID==TORA_TSTP_EXD_SSE?g_pShareholderIDSH:g_pShareholderIDSZ;
}

int CStockApi::ReqOrderInsert(CTORATstpInputOrderField *pInputOrder)
{
	if(!pPosManager->OrderInsert(pInputOrder)){
		printf("Order Check Failed\n");
		return 1;
	}
	///��������
	strcpy_s(pInputOrder->OrderRef,GetMaxOrderRef().c_str());
	m_pTradeApi->ReqOrderInsert(pInputOrder,stoi(pInputOrder->OrderRef));
	PushOrderKey(to_string(m_nFrontID)+to_string(m_nSessionID)+pInputOrder->OrderRef);//��������map �����䷢�����
	
	/*inputorder����û��frontid��sessionid
	frontid+sessionid+orderrefΨһ����һ�ʱ���
	������ʱ��Ҫ��sessionid��frontid����Ҫ������*/

	m_mapAppendOrder[pInputOrder->SecurityID].push_back(TransferOrder(*pInputOrder)); //
	return 0;
}

int CStockApi::ReqOrderAction(CTORATstpInputOrderActionField *pInputOrderAction)
{
	///��������
	strcpy_s(pInputOrderAction->OrderActionRef,GetMaxOrderRef().c_str());
	m_pTradeApi->ReqOrderAction(pInputOrderAction,stoi(pInputOrderAction->OrderActionRef));
	return 0;
}

void CStockApi::RspOrderInsert(CTORATstpInputOrderField *pInputOrderField)
{
	pPosManager->RspOrderInsert(pInputOrderField);
}

void CStockApi::RtnOrder(CTORATstpOrderField *pOrder)
{
	///���²�λ
	pPosManager->RtnOrder(pOrder);

	///������;��
	map<string,list<CTORATstpOrderField>>::iterator iterMap=m_mapAppendOrder.find(pOrder->SecurityID);
	if(iterMap==m_mapAppendOrder.end()){
		return;
	}

	///�ѳ�������ȫ���ɽ�����ί���б�ͳ����б���ɾ��,�������ί��
	list<CTORATstpOrderField>::iterator iterList;
	if(pOrder->OrderStatus==TORA_TSTP_OST_Canceled||pOrder->OrderStatus==TORA_TSTP_OST_AllTraded){
		///��ί���б���ɾ��
		iterList = iterMap->second.begin();
		while(iterList!=iterMap->second.end()){
			if((iterList->FrontID==pOrder->FrontID)&&(iterList->SessionID==pOrder->SessionID)&&(!strcmp(iterList->OrderRef,pOrder->OrderRef))){
				iterMap->second.erase(iterList);
				break;
			}
			iterList++;
		}
	}else{
		iterList = iterMap->second.begin();
		while(iterList!=iterMap->second.end()){
			if((iterList->FrontID==pOrder->FrontID)&&(iterList->SessionID==pOrder->SessionID)&&(!strcmp(iterList->OrderRef,pOrder->OrderRef))){
				int requestid=iterList->RequestID;
				*iterList=*pOrder;
				iterList->RequestID=requestid;
				break;
			}
			iterList++;
		}
	}
	return;
}

void CStockApi::RtnTrade(CTORATstpTradeField *pTrade)
{
	pPosManager->RtnTrade(pTrade);
}

void CStockApi::InitPosition(CTORATstpPositionField* pPosition)
{
	pPosManager->InitPosition(pPosition);
}


//
const CTORATstpOrderField& CStockApi::TransferOrder(const CTORATstpInputOrderField& inputOrder)
{
	static CTORATstpOrderField rtnOrder;
	rtnOrder.SessionID=m_nSessionID;
	rtnOrder.FrontID=m_nFrontID;
	strcpy_s(rtnOrder.OrderRef,inputOrder.OrderRef);
	rtnOrder.ExchangeID=inputOrder.ExchangeID;
	strcpy_s(rtnOrder.SecurityID,inputOrder.SecurityID);
	rtnOrder.OrderPriceType=inputOrder.OrderPriceType;
	rtnOrder.Direction=inputOrder.Direction;
	rtnOrder.LimitPrice=inputOrder.LimitPrice;
	rtnOrder.VolumeTotalOriginal=inputOrder.VolumeTotalOriginal;
	rtnOrder.VolumeTraded=0;
	rtnOrder.VolumeTotal=inputOrder.VolumeTotalOriginal;
	GET_ACCURATE_TIME(rtnOrder.RequestID);//���淢��ʱ��
	return rtnOrder;
}

bool CStockApi::IsSentOrder(const string& key)
{
	if(m_setSentOrder.find(key)!=m_setSentOrder.end()){
		return true;
	}
	return false;
}

void CStockApi::PushOrderKey(const string& key)
{
	m_setSentOrder.insert(key);
}

void CStockApi::UpdateSessionInfo(int FrontID,int SessionID,int MaxOrderRef)
{
	m_nSessionID=SessionID;
	m_nFrontID=FrontID;
	m_nMaxOrderRef=MaxOrderRef;
}

int CStockApi::LimitBuy(TTORATstpExchangeIDType ExchangeID,const string& SecurityID,double LimitPrice,int Volume)
{
	CTORATstpInputOrderField* pContent= new CTORATstpInputOrderField;
	memset(pContent,0,sizeof(CTORATstpInputOrderField));
	strcpy_s(pContent->InvestorID, g_pInvestorID);
	strcpy_s(pContent->SecurityID, SecurityID.c_str());
	pContent->ExchangeID = ExchangeID;
	strcpy_s(pContent->ShareholderID, GetShareHolderID(ExchangeID));
	pContent->Direction = TORA_TSTP_D_Buy;
	pContent->OrderPriceType = TORA_TSTP_OPT_LimitPrice;
	pContent->TimeCondition = TORA_TSTP_TC_GFD;
	pContent->VolumeCondition = TORA_TSTP_VC_AV;
	pContent->LimitPrice = LimitPrice;
	pContent->VolumeTotalOriginal = Volume;
	strcpy_s(pContent->OrderRef, "");
	pContent->CombOffsetFlag[0] = TORA_TSTP_OF_Open;
	pContent->CombOffsetFlag[1] = '\0';
	pContent->CombHedgeFlag[0] = TORA_TSTP_HF_Speculation;
	pContent->CombHedgeFlag[1] = '\0';
	pContent->MinVolume = 0;
	pContent->UserForceClose = 0;
	pContent->IsSwapOrder = 0;
	pContent->Operway = 'u';
	Message msg={ORDERINSERT,(void*)pContent};
	PostMessage(msg);
	return ORDERINSERT;
}

int CStockApi::LimitSell(TTORATstpExchangeIDType ExchangeID,const string& SecurityID,double LimitPrice,int Volume)
{
	CTORATstpInputOrderField* pContent= new CTORATstpInputOrderField;
	memset(pContent,0,sizeof(CTORATstpInputOrderField));
	strcpy_s(pContent->InvestorID, g_pInvestorID);
	strcpy_s(pContent->SecurityID, SecurityID.c_str());
	pContent->ExchangeID = ExchangeID;
	strcpy_s(pContent->ShareholderID, GetShareHolderID(ExchangeID));
	pContent->Direction = TORA_TSTP_D_Sell;
	pContent->OrderPriceType = TORA_TSTP_OPT_LimitPrice;
	pContent->TimeCondition = TORA_TSTP_TC_GFD;
	pContent->VolumeCondition = TORA_TSTP_VC_AV;
	pContent->LimitPrice = LimitPrice;
	pContent->VolumeTotalOriginal = Volume;
	strcpy_s(pContent->OrderRef, "");
	pContent->CombOffsetFlag[0] = TORA_TSTP_OF_Open;
	pContent->CombOffsetFlag[1] = '\0';
	pContent->CombHedgeFlag[0] = TORA_TSTP_HF_Speculation;
	pContent->CombHedgeFlag[1] = '\0';
	pContent->MinVolume = 0;
	pContent->UserForceClose = 0;
	pContent->IsSwapOrder = 0;
	pContent->Operway = 'u';
	Message msg={ORDERINSERT,(void*)pContent};
	PostMessage(msg);
	return ORDERINSERT;
}

int CStockApi::Cancel(int sessionid,int frontid,TTORATstpExchangeIDType ExchangeID,const string& SecurityID,const string& orderref)
{
	CTORATstpInputOrderActionField *pContent=new CTORATstpInputOrderActionField;
	memset(pContent, 0, sizeof(CTORATstpInputOrderActionField));
	strcpy_s(pContent->InvestorID, g_pInvestorID);
	strcpy_s(pContent->SecurityID, SecurityID.c_str());
	strcpy_s(pContent->OrderRef,orderref.c_str());
	pContent->FrontID = frontid;
	pContent->SessionID = sessionid;
	pContent->ActionFlag = TORA_TSTP_AF_Delete;
	pContent->ExchangeID = ExchangeID;
	Message msg={ORDERACTION,(void*)pContent};
	PostMessage(msg);
	return ORDERACTION;
}

const CTORATstpPositionField* CStockApi::GetPosition(const string& key)
{
	return pPosManager->GetPosition(key);
}

const list<CTORATstpOrderField>* CStockApi::GetOrderInfo(TTORATstpExchangeIDType ExchangeID,const string& SecurityID)
{
	map<string,list<CTORATstpOrderField>>::iterator iter=m_mapAppendOrder.find(SecurityID);
	if(iter==m_mapAppendOrder.end()){
		return nullptr;
	}else{
		return &iter->second;
	}
	return nullptr;
}