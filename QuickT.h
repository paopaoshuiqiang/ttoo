#pragma once
#include <vector>
#include <sstream>
#include <iostream>
#include <basetsd.h>
#include <hash_map>
#include "../BA_COMM/BA_STRUCT.h"

using namespace std;


const	string		StrategyName = "TORATrade";

const   string		ConfigPath   = "Config.txt";
const   string		PoolPath	 = "PoolInfo.txt";
const   string		PortPath	 = "PortPath.txt";
const	string		HYName		 = "HYInfo.txt";

const int    StockSize          = 500;                                                   // ���й�Ʊ����



const int    KLTimeSize         = 1440;                                                   // ʱ��ڵ����
const int    KLTimeCycle        = 4*3600/KLTimeSize;                                     // ʱ��ڵ�Ƶ�ʣ��룩
const string KLTimeName         = "0_BasicInfo//KLTime_S10S.txt";                        // ʱ��ڵ��ļ� 30������ڵ�

const int    HQUpdateErrorS     = 300;                                                   // ������´��������룩
const int    HQTimeMin          = 83000;                                                 // ����������Сʱ�� �����ж�mdtime�Ƿ�����
const int    HQTimeMax          = 150000;

struct Flag
{
	string	FlagName;
	int		FlagValue;
	string	FlagDetail;
	int		FlagType;

};




class CheckFlag
{
public:
	int		mid;
	vector<Flag>			FlagInfo;
	hash_map<string,int>	FlagMap;

	void WriteCheck(string FlagName,int FlagValue,string FlagDetail,int FlagType);
	void ReadCheck(string FlagName,int& FlagValue,string& FlagDetail,int& FlagType);



	CheckFlag();
	~CheckFlag();



private:

};

struct ControlInfoStock
{
	string Code;
	string Name;
	string House;
	double PriceP;
	int    BuyTarget;
	int	   SellTarget;
	//int    HYId;
	//string HYCode;
	//string HYName;

	//double WeightH;   // �ֲ�Ȩ��
	//double WeightT;   // Ŀ��Ȩ��
	//double WeightU;   // ����Ȩ��
	//double WeightD;   // ����Ȩ��

	double ValueH;    // �ֲ���ֵ
	double ValueT;    // Ŀ����ֵ
	double ValueU;    // ������ֵ
	double ValueD;    // ������ֵ
	string Status;    // 12_֤ȯ״̬ T  ����  F  ���ƽ��ף�����ͣ�ơ���������
};



class ConfigFile
{
public:

	ConfigFile (void);
	~ConfigFile(void);

	//vector<ControlInfoHY>      HYInfo;     // ÿ����ҵ�Ľ�����Ϣ

	void Read_Configfile  (void);
	void Read_PoolInfo    (void);
	void Read_PortInfo    (void);
	void Read_HYInfo      (void);
	void Read_KLTime      (void);

	void Update_PoolInfo   (vector<Entrust>);
	void Update_PortInfo   (vector<Entrust>);
	void Update_HYInfo     (vector<Entrust>);

	void Result_TxtInfo(void); // �ļ���Ϣ

	// ��ǰ�ļ�
	vector<ControlInfoStock>               PoolInfo;   // ֤ȯ��Ϣ�ļ�: ���ļ� �ֲֺͽ��׵�����Ʒ��
	//vector<ControlInfoStock>   PortInfo;


	// �����ļ�
	// �̶�����_13
	int    BatchID;
	int    BatchSize;
	double ZFMax;              // �������Ƿ�������
	double ZFMin;              // ���ڴ��Ƿ�������
	int    BegTime_Am;         // ������ʼʱ�� 935
	int    EndTime_Am;         // �������ʱ�� 1129
	int    BegTime_Pm;         // ������ʼʱ�� 1301
	int    EndTime_Pm;         // �������ʱ�� 1455
	int    HQNofBeginS;        // ����Ԥ��ʱ�� ��
	int    CLNofBeginS;        // ����Ԥ��ʱ�� ��
	int    NofLag;             // ���׼�� ��
	int	   SingleStockHQAmt;   // ����еĹ�Ʊ��������ȱʧ�ĸ�����ֵ(���ڸ÷�ֵ��ֹ��������������ˢ�����飬С�ڸ÷�ֵ��������µ�����)
	double OddAmountFZ;        // ��ɽ�ֵ

	// ���Բ���_5
	double ImpactCost;         // ����ɱ�
	int    NofMaforSpread;     // �۲����
	int    NofMaforIndex;      // ָ������
	double TRatio;             // �̿ڱ���
	double TradeAmountFZ;      // ���ʽ��׽������
	double PendAmountFZ;       // �Ѿ�ί�е�û�гɽ����ķ�ֵ
	double NetAmountU;         // �������������
	double NetAmountD;         // �������������
	double TotalAmountFZ;      // ���ճɽ��������� ˫�ߵĽ��
	double TotalAmount;       // �˻��ܽ��
	string	IndexCode;		//�Ա�ָ��   000300|399006

private:

};

class Query
{
public:
	double					AmountBuy,AmountSell;
	vector<Account>			AccountReport;		//��ѯ�ֲ�
	vector<Entrust>			EnturstAccountAll,EntrustStraAll,EntrustCancel;

	void					Select_EntrustReport(int BMin,int BMax,double& CancelAmount,vector<int>& BCancel,int& BBeg);








	Query();
	~Query();

private:

};

///////////////////////////////////////////////////////////////////////////////
//HQ
////////////////////////////////////////////////////////////////////////////////

struct HQCross
{
	// �������ݽṹ
	string Code;                               // ��Ʊ����
	string House;
	//int    HYId;							 //hyid�Ǵ�0 ��ʼ��

	// 8��
	int    TimeHQ;                             // ����ʱ��
	int    UpdateStauts;                       // �����Ƿ����� 1 ture 0 false

	double PriceP;                             // ǰ����
	double PriceO;							   //���̼�
	double PriceC  ,PriceB  ,PriceS;           // �ɽ���     ��һ��    ��һ��
	UINT64 SharesB ,SharesS;                   // ��һ��     ��һ��

	double IPriceC,IPriceB,IPriceS;            // ��׼�۸�
	double PriceB1 ,PriceB2 ,PriceB3;          // ǰ3��image����һ��
	double PriceS1 ,PriceS2 ,PriceS3;          // ǰ3��image����һ��
	double SharesB1 ,SharesB2 ,SharesB3;	   // ǰ3��image����һ��
	double SharesS1 ,SharesS2 ,SharesS3;	   // ǰ3��image����һ��
	double dMaxB,dMaxS;						   // ǰ3��image���������ֵ
};


class HQ
{
public:
	//��Ʊ����
	int					StockCurrentKid[StockSize];		//���ɵ�ǰ��k�� id
	double				StockHQ[StockSize][KLTimeSize];	//���ɵ�k�� ��Ϣ

	/////////////////////////////
	//ֱ���жϣ�Ȼ��updateSignal
	/////////////////////////////
	//double				HS300[KLTimeSize];				//300ָ��  �µ�0.8%
	//double				CYB[KLTimeSize];				//��ҵ��	   �µ�1%


	int					HQStartTime;					//���鿪ʼʱ��					

	// ǰ��У��
	int  PreHQLost;
	void CheckPreHQ  (void);

	vector<HQCross>		PoolHQ;							

	void				CheckPreHQ();

	void Init_PoolHQ (void);   int Update_PoolHQ  (MarketData& md,string House);




	HQ();
	~HQ();

private:

};

//////////////////////////////////////////////////////////////////////////////
//Signal
//////////////////////////////////////////////////////////////////////////////

struct SignalCross
{
	string Code;              // ��Ʊ����
	string House;             // �г�
	//int    HYId;              // ��ҵ����

	int    BuyTarget;                // ���ڽ��׷���1 ��   -1 ��   0 ������
	int    SellTarget;                // ���ڽ��׷���1 ��   -1 ��   0 ������
	int    LastTradeTime;     // ����ɽ�ʱ��
	int    HQStatus;          // ��Ʊ״̬��1 ����  0 ͣ�ƣ�
	int    NonWaitingStatus;  // �Ƿ���Ҫ�ȴ� ��ֹ��������

	double SpreadC;
	double SpreadB;
	double SpreadS;
	double SpreadMa;
	double CloseMa;

	double BiasC;
	double BiasB;
	double BiasS;
};



class Signal
{
public:

	double				DataForTS[StockSize][KLTimeSize];			//��Ʊ����ʱ������
	double				WriteDataForTS(int StockMa,int Id,int CurrentTime,double SpreadC);

	vector<SignalCross> PoolSignal;
	void				Init_PoolSignal();
	void				Update_PoolSignal(int StockPosition);
	void				Listen_PoolSignal(int StockPosition);


	//��ҵָ��������Ϣ
	//double				IndexHY[HYSize][KLTimeSize][4];	

	//void				Init_IndexSignal();
	//void				Update_IndexSignal();

	void				Send_TOrder(Order torder);


	Signal();
	~Signal();

private:

};





//////////////////////////////////////////////////////////////////////////////
//LogicAccount
///////////////////////////////////////////////////////////////////////////////

class LogicAccount
{
public:
	// �������
	vector<Account>          PortAccount;  
	vector<Account>          PoolAccount;  

	hash_map<string,double>  PortValueH;        // ��Ʊ��ֵ�ֲ�
	hash_map<string,double>  PortValueU;        // ��Ʊ��ֵ����
	hash_map<string,double>  PortValueD;        // ��Ʊ��ֵ����
	hash_map<string,double>  PortValueUDiff;    // ��Ʊ��ֵ���޾���
	hash_map<string,double>  PortValueDDiff;    // ��Ʊ��ֵ���޾���

	hash_map<int,double>     HYValueH;          // ��ҵ��ֵ�ֲ�
	hash_map<int,double>     HYValueU;          // ��ҵ��ֵ����
	hash_map<int,double>     HYValueD;          // ��ҵ��ֵ����
	hash_map<int,double>     HYValueUDiff;      // ��ҵ��ֵ���޾���
	hash_map<int,double>     HYValueDDiff;      // ��ҵ��ֵ���޾���

	// �˻�����
	void Init_PoolAccount    (void);
	void Init_PoolValue      (double& PoolValue); // ��������ֵ

	void Init_PortAccount    (void);
	void Update_PortAccount  (void);
	void Init_PortValue      (void);
	void Update_PortValue    (void);	

	void Init_HYValue       (void);
	void Update_HYValue     (void);
	LogicAccount();
	~LogicAccount();

private:

};

