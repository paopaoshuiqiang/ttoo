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

const int    StockSize          = 500;                                                   // 所有股票数量



const int    KLTimeSize         = 1440;                                                   // 时间节点个数
const int    KLTimeCycle        = 4*3600/KLTimeSize;                                     // 时间节点频率（秒）
const string KLTimeName         = "0_BasicInfo//KLTime_S10S.txt";                        // 时间节点文件 30秒抽样节点

const int    HQUpdateErrorS     = 300;                                                   // 行情更新错误间隔（秒）
const int    HQTimeMin          = 83000;                                                 // 行情的最大最小时间 用于判断mdtime是否正常
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

	//double WeightH;   // 持仓权重
	//double WeightT;   // 目标权重
	//double WeightU;   // 上限权重
	//double WeightD;   // 下限权重

	double ValueH;    // 持仓市值
	double ValueT;    // 目标市值
	double ValueU;    // 上限市值
	double ValueD;    // 下限市值
	string Status;    // 12_证券状态 T  正常  F  限制交易（包括停牌、黑名单）
};



class ConfigFile
{
public:

	ConfigFile (void);
	~ConfigFile(void);

	//vector<ControlInfoHY>      HYInfo;     // 每个行业的交易信息

	void Read_Configfile  (void);
	void Read_PoolInfo    (void);
	void Read_PortInfo    (void);
	void Read_HYInfo      (void);
	void Read_KLTime      (void);

	void Update_PoolInfo   (vector<Entrust>);
	void Update_PortInfo   (vector<Entrust>);
	void Update_HYInfo     (vector<Entrust>);

	void Result_TxtInfo(void); // 文件信息

	// 盘前文件
	vector<ControlInfoStock>               PoolInfo;   // 证券信息文件: 读文件 持仓和交易的所有品种
	//vector<ControlInfoStock>   PortInfo;


	// 配置文件
	// 固定参数_13
	int    BatchID;
	int    BatchSize;
	double ZFMax;              // 超过此涨幅不交易
	double ZFMin;              // 低于此涨幅不交易
	int    BegTime_Am;         // 上午起始时间 935
	int    EndTime_Am;         // 上午结束时间 1129
	int    BegTime_Pm;         // 下午起始时间 1301
	int    EndTime_Pm;         // 下午结束时间 1455
	int    HQNofBeginS;        // 行情预热时间 秒
	int    CLNofBeginS;        // 行情预热时间 秒
	int    NofLag;             // 交易间隔 秒
	int	   SingleStockHQAmt;   // 组合中的股票在行情中缺失的个数阀值(大于该阀值禁止后续动作，继续刷新行情，小于该阀值允许后续下单操作)
	double OddAmountFZ;        // 零股金额阀值

	// 策略参数_5
	double ImpactCost;         // 冲击成本
	int    NofMaforSpread;     // 价差均线
	int    NofMaforIndex;      // 指数均线
	double TRatio;             // 盘口比例
	double TradeAmountFZ;      // 单笔交易金额上限
	double PendAmountFZ;       // 已经委托单没有成交金额的阀值
	double NetAmountU;         // 净买入金额的上限
	double NetAmountD;         // 净买入金额的下限
	double TotalAmountFZ;      // 当日成交金额的上限 双边的金额
	double TotalAmount;       // 账户总金额
	string	IndexCode;		//对比指数   000300|399006

private:

};

class Query
{
public:
	double					AmountBuy,AmountSell;
	vector<Account>			AccountReport;		//查询持仓
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
	// 行情数据结构
	string Code;                               // 股票代码
	string House;
	//int    HYId;							 //hyid是从0 开始的

	// 8个
	int    TimeHQ;                             // 行情时间
	int    UpdateStauts;                       // 更新是否正常 1 ture 0 false

	double PriceP;                             // 前收盘
	double PriceO;							   //开盘价
	double PriceC  ,PriceB  ,PriceS;           // 成交价     买一价    卖一价
	UINT64 SharesB ,SharesS;                   // 买一量     卖一量

	double IPriceC,IPriceB,IPriceS;            // 标准价格
	double PriceB1 ,PriceB2 ,PriceB3;          // 前3个image的买一价
	double PriceS1 ,PriceS2 ,PriceS3;          // 前3个image的卖一价
	double SharesB1 ,SharesB2 ,SharesB3;	   // 前3个image的买一量
	double SharesS1 ,SharesS2 ,SharesS3;	   // 前3个image的卖一量
	double dMaxB,dMaxS;						   // 前3个image买卖量最大值
};


class HQ
{
public:
	//股票个数
	int					StockCurrentKid[StockSize];		//个股当前的k线 id
	double				StockHQ[StockSize][KLTimeSize];	//个股的k线 信息

	/////////////////////////////
	//直接判断，然后updateSignal
	/////////////////////////////
	//double				HS300[KLTimeSize];				//300指数  下跌0.8%
	//double				CYB[KLTimeSize];				//创业板	   下跌1%


	int					HQStartTime;					//行情开始时间					

	// 前收校验
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
	string Code;              // 股票代码
	string House;             // 市场
	//int    HYId;              // 行业代码

	int    BuyTarget;                // 日内交易方向（1 买   -1 卖   0 待定）
	int    SellTarget;                // 日内交易方向（1 买   -1 卖   0 待定）
	int    LastTradeTime;     // 最近成交时间
	int    HQStatus;          // 股票状态（1 正常  0 停牌）
	int    NonWaitingStatus;  // 是否需要等待 防止连续交易

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

	double				DataForTS[StockSize][KLTimeSize];			//股票行情时间序列
	double				WriteDataForTS(int StockMa,int Id,int CurrentTime,double SpreadC);

	vector<SignalCross> PoolSignal;
	void				Init_PoolSignal();
	void				Update_PoolSignal(int StockPosition);
	void				Listen_PoolSignal(int StockPosition);


	//行业指数行情信息
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
	// 策略组合
	vector<Account>          PortAccount;  
	vector<Account>          PoolAccount;  

	hash_map<string,double>  PortValueH;        // 股票市值持仓
	hash_map<string,double>  PortValueU;        // 股票市值上限
	hash_map<string,double>  PortValueD;        // 股票市值下限
	hash_map<string,double>  PortValueUDiff;    // 股票市值上限距离
	hash_map<string,double>  PortValueDDiff;    // 股票市值下限距离

	hash_map<int,double>     HYValueH;          // 行业市值持仓
	hash_map<int,double>     HYValueU;          // 行业市值上限
	hash_map<int,double>     HYValueD;          // 行业市值下限
	hash_map<int,double>     HYValueUDiff;      // 行业市值上限距离
	hash_map<int,double>     HYValueDDiff;      // 行业市值下限距离

	// 账户函数
	void Init_PoolAccount    (void);
	void Init_PoolValue      (double& PoolValue); // 计算总市值

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

