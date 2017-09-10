#ifndef OPTMARKET_HPP
#define OPTMARKET_HPP

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <stdio.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iterator>

#include "bittrexApi.hpp"
#include "OptFrame/Timer.hpp"

using namespace std;
using namespace optframe;

struct currencyBalance
{
	double balance, available, pending;
	string market;
	currencyBalance(string _market = "", double _balance = 0, double _available = 0, double _pending = 0) :
			market(_market), balance(_balance), pending(_pending), available(_available)
	{

	}
};

ostream& operator<<(ostream &os, const currencyBalance& obj)
{
	os << "balance:" <<  obj.balance << "\tavailable:" << obj.available << "\tpending:" << obj.pending << endl;
	return os;
}

enum BookOrderParameter
{
	BOOK_BUY_ORDERS_Q, BOOK_BUY_ORDERS_R, BOOK_SELL_ORDERS_Q, BOOK_SELL_ORDERS_R
};

template<class T>
ostream& operator<<(ostream &os, const vector<T> &obj)
{
	os << "vector(" << obj.size() << ") [";

	if (obj.size() > 0)
	{
		for (unsigned int i = 0; i < obj.size() - 1; i++)
			os << obj.at(i) << " , ";
		os << obj.at(obj.size() - 1);
	}

	os << "]";
	return os;
}

struct ordersMetrics
{
	double avg;
	double stdev;

	ordersMetrics()
	{
		avg = 0;
		stdev = 0;
	}

};

struct tsMetrics
{
	double macd;

};

struct myMarketTimeSeries
{
	vector<vector<ordersMetrics> > mTS; //vector of book metrics for with desired frequency
	int frequency;
	int maxTS;

	myMarketTimeSeries(int _frequency, int _maxTS) :
			frequency(_frequency), maxTS(_maxTS)
	{

	}

	virtual ~myMarketTimeSeries()
	{
	}

};

struct KahanAccumulation
{
	double sum;
	double correction;
};

static KahanAccumulation KahanSum(KahanAccumulation accumulation, double value)
{
	KahanAccumulation result;
	double y = value - accumulation.correction;
	double t = accumulation.sum + y;
	result.correction = (t - accumulation.sum) - y;
	result.sum = t;
	return result;
}

class OrderBookTimeSeries
{
private:
	vector<myMarketTimeSeries> vTS;

public:

	OrderBookTimeSeries(vector<int> vFequencies, vector<int> vMaxSamples)
	{
		int nTS = vFequencies.size();

		if (nTS == 0 || nTS != (int) vMaxSamples.size())
			cout << "error inside OrderBookTimeSeries constructor" << endl;

		for (int i = 0; i < nTS; i++)
		{
			myMarketTimeSeries mMTS(vFequencies[i], vMaxSamples[i]);
			vTS.push_back(mMTS);
		}

	}

	virtual ~OrderBookTimeSeries()
	{
	}

	int getNTS()
	{
		return vTS.size();
	}

	int getTSIterSize(int ts)
	{
		return vTS[ts].mTS.size();
	}

	int getTSIterFrequency(int ts)
	{
		return vTS[ts].frequency;
	}

	int getTSIterMaxSamples(int ts)
	{
		return vTS[ts].maxTS;
	}

	void setTSNewObservation(int ts, vector<ordersMetrics> vOM)
	{
		vTS[ts].mTS.push_back(vOM);
	}

	void deletaTSOldestObservation(int ts)
	{
		vTS[ts].mTS.erase(vTS[ts].mTS.begin());
	}

//	vector<ordersMetrics> getAvgLastSamples(int frequency)
//	{
//		int nSizeTSSeconds = vMetricsByIter.size();
//		//problem -- 60 seconds was not yet achieved
//		if (nSizeTSSeconds < 60)
//			return vMetricsByIter[0];
//
//		vector<ordersMetrics> vAvgOM(4);
//		for (int i = (nSizeTSSeconds - 1); i > (nSizeTSSeconds - 61); i--)
//		{
//			for (int m = 0; m < 4; m++)
//			{
//				vAvgOM[m].avg += vMetricsByIter[i][m].avg;
//				vAvgOM[m].stdev += vMetricsByIter[i][m].stdev;
//			}
//		}
//		for (int m = 0; m < 4; m++)
//		{
//			vAvgOM[m].avg /= 60;
//			vAvgOM[m].stdev /= 60;
//		}
//
//		return vAvgOM;
//	}

	void updateTS(const vector<ordersMetrics>& vOM, const long int iter)
	{
		int nTS = getNTS();

		for (int ts = 0; ts < nTS; ts++)
		{
			int tsFrequency = getTSIterFrequency(ts);
			int tsSize = getTSIterSize(ts);
			if (iter > tsSize * tsFrequency)
			{
				setTSNewObservation(ts, vOM); //TODO
//				setTSNewObservation(ts,getAvgLastSamples(tsFrequency)); //TODO get average values of last
				if (getTSIterFrequency(ts) > getTSIterMaxSamples(ts))
					deletaTSOldestObservation(ts);
			}

		}
	}

//	double* convertVectorOfOrderMetricsToSumOrStdTS(const vector<vector<ordersMetrics> > tsOM)
//	{
//		int nSamples = tsOM.size();
//		double* ts = new double[nSamples];
//		for (int s = 0; s < nSamples; s++)
//			ts[s] = tsOM[s][0].avg;
//		return ts;
//	}

	vector<double> convertVectorOfOrderMetricsToSumOrStdTS(const vector<vector<ordersMetrics> > tsOM)
	{
		int nSamples = tsOM.size();
		vector<double> ts(nSamples);
		for (int s = 0; s < nSamples; s++)
			ts[s] = tsOM[s][0].avg;
		return ts;
	}

	// simple moving average
	vector<double> moving_average(const vector<double>& values, const int periods)
	{
		int size = values.size();
		vector<double> averages;
		double sum = 0;
		for (int i = 0; i < size; i++)
			if (i < periods)
			{
				sum += values[i];
				averages.push_back((i == periods - 1) ? sum / (double) periods : 0);
			}
			else
			{
				sum = sum - values[i - periods] + values[i];
				averages.push_back(sum / (double) periods);
			}
		return averages;
	}

	void subtractVectorsSameSize(const vector<double>& a, const vector<double>& b, vector<double>& result)
	{
		transform(a.begin(), a.end(), b.begin(), std::back_inserter(result), [&](double l, double r)
		{
			return std::abs(l - r);
		});
	}
	void callMetrics()
	{
		vector<double> values = convertVectorOfOrderMetricsToSumOrStdTS(vTS[0].mTS);

		vector<double> mAVGFast = moving_average(values, 26);
		vector<double> mAVGSlow = moving_average(values, 12);
		vector<double> MACDLine;
		subtractVectorsSameSize(mAVGSlow, mAVGFast, MACDLine);
		vector<double> signalLine = moving_average(MACDLine, 9);
		vector<double> MACDHistogram;
		subtractVectorsSameSize(MACDLine, signalLine, MACDHistogram);

		cout << "============================" << endl;
		cout << "MACD Histogram" << endl;
		cout << MACDHistogram << endl;
		cout << "============================" << endl;
//		getchar();
	}
//		TA_RetCode retCode;
//
//		retCode = TA_Initialize();
//
//		if (retCode != TA_SUCCESS)
//			printf("Cannot initialize TA-Lib (%d)!\n", retCode);
//		else
//		{
//			printf("TA-Lib correctly initialized.\n");
//
//			/* ... other TA-Lib functions can be used here. */
//
////			TA_RetCode TA_MACD( int    startIdx,
////			                    int    endIdx,
////			                    const double inReal[],
////			                    int           optInFastPeriod, /* From 2 to 100000 */
////			                    int           optInSlowPeriod, /* From 2 to 100000 */
////			                    int           optInSignalPeriod, /* From 1 to 100000 */
////			                    int          *outBegIdx,
////			                    int          *outNBElement,
////			                    double        outMACD[],
////			                    double        outMACDSignal[],
////			                    double        outMACDHist[] );
//			double outMACD[50];
//			double outMACDSignal[50];
//			double outMACDHist[50];
//			int *outBegIdx;
//			int *outNBElement;
//			double* inReal = convertVectorOfOrderMetricsToSumOrStdTS(vTS[0].mTS);
//
//			retCode = TA_MACD(0, 49, inReal, 27, 5, 9, outBegIdx, outNBElement, outMACD, outMACDSignal, outMACDHist);
//
//			cout<<outMACD<<endl;
//			cout<<outMACDSignal<<endl;
//			cout<<outMACDHist<<endl;
//			getchar();
//			TA_Shutdown();

//		cout << "time:\t" << time << endl;
//		ts.vMetricsByIter.push_back(vOM);
//
//		if ((int) ts.vMetricsByIter.size() > ts.maxTSIter)
//			ts.vMetricsByIter.erase(ts.vMetricsByIter.begin());
//
//		int nSamplesTSMin = ts.vMetricsByMin.size();
//		if (time > int(nSamplesTSMin * 60)) //since samples increase it just enter again after 60s
//		{
//			ts.vMetricsByMin.push_back(getAvgLastMinute(ts.vMetricsByIter));
//
//			if ((int) ts.vMetricsByMin.size() > ts.maxTSMin)
//				ts.vMetricsByMin.erase(ts.vMetricsByMin.begin());
//		}

//		v.erase( v.begin(), v.size() > N ?  v.begin() + N : v.end() );

};

class OptMarket
{
private:

public:

	OptMarket()
	{

	}

	virtual ~OptMarket()
	{
	}

	currencyBalance getMarketBalanceFromAPI(string buffer, string market)
	{
//		cout << market << endl;
//		cout << buffer.find(market) << endl;
//		getchar();
		string cutUntilDesignedCurrencyBalance = buffer.substr(buffer.find(market));
//		cout << cutUntilDesignedCurrencyBalance << endl;
//		getchar();
		size_t pos = cutUntilDesignedCurrencyBalance.find(":");
		size_t posEnds = cutUntilDesignedCurrencyBalance.find("Available");
		string balance = cutUntilDesignedCurrencyBalance.substr(pos + 1, posEnds - pos - 3);
//		cout << "current balance of " << market << " is:" << balance << endl;
		currencyBalance cB(market, stod(balance), 0, 0);
		return cB;
	}

	void optGetBalanceBittrex(BittrexAPI& bittrex, string market)
	{
		bool print = false;
		bool sign = true; //If you want to sign
		bool exportReply = false;

		bittrex.setGetBalance(market);
		string buffer = bittrex.callCurlPlataform(InstuctionOptions(print, sign, exportReply));
		currencyBalance cBT = getMarketBalanceFromAPI(buffer, market);

		cout << cBT << endl;

	}

	void optSellCalculatingProfit(BittrexAPI& bittrex, double quantity, double rate, string market)
	{
		double theoricalTotal = (quantity * rate);
		double costs = theoricalTotal * (bittrex.getFeesPerOrder());
		double realTotal = theoricalTotal - costs;
		double actualRate = realTotal / rate;
		cout << "Selling rate:" << rate << "\tq:" << quantity << "\ntTotal:" << theoricalTotal << "\trTotal:" << realTotal << "\tcosts:" << costs << "\taRate:" << actualRate << endl;

		string baseCurrency = market.substr(0, market.find("-"));
		string tradedCurrency = market.substr(market.find("-") + 1);

		bool print = true;
		bool sign = true; //If you want to sign
		bool exportReply = false;

		InstuctionOptions instOpt(print, sign, exportReply);

		bittrex.setGetBalance(tradedCurrency);
		string buffer = bittrex.callCurlPlataform(InstuctionOptions(false, true, false));
		currencyBalance cBT = getMarketBalanceFromAPI(buffer, tradedCurrency);
		currencyBalance cBB = getMarketBalanceFromAPI(buffer, baseCurrency);

		bittrex.setSellLimit(market, quantity, rate);
		buffer = bittrex.callCurlPlataform(InstuctionOptions(true, true, false));

		//TODO - Create a function for checking pending orders and wait
		//Go to the multicore, multigpu and run all this in parallel

		bittrex.setGetBalance(tradedCurrency);
		buffer = bittrex.callCurlPlataform(InstuctionOptions(false, true, false));
		currencyBalance cBTNew = getMarketBalanceFromAPI(buffer, tradedCurrency);
		currencyBalance cBBNew = getMarketBalanceFromAPI(buffer, baseCurrency);

		cout << "Traded(Old,New):(" << cBT.balance << "," << cBTNew.balance << ")" << endl;
		cout << "Base(Old,New):(" << cBB.balance << "," << cBBNew.balance << ")" << endl;
		cout << "Sold value of traded is: " << cBTNew.balance - cBT.balance << endl;
		cout << "Amount of coins bought for the base currency is: " << cBBNew.balance - cBB.balance << endl;
		getchar();

	}

	vector<ordersMetrics> returnOrdersBookAverageValues(const vector<vector<double> > orderBookVectors, const double percentage, const int nOrders = -1)
	{
		vector<KahanAccumulation> vSum(4);
		int nBuying = orderBookVectors[BOOK_BUY_ORDERS_Q].size();
		int nSelling = orderBookVectors[BOOK_SELL_ORDERS_Q].size();

		int nBuyOrdersToAvg = nOrders, nSellOrdersToAvg = nOrders;
		if (nOrders == -1)
		{
			nBuyOrdersToAvg = nBuying * percentage;
			nSellOrdersToAvg = nBuying * percentage;
		}

		KahanAccumulation init =
		{ 0 };
		vSum[BOOK_BUY_ORDERS_Q] = accumulate(orderBookVectors[BOOK_BUY_ORDERS_Q].begin(), orderBookVectors[BOOK_BUY_ORDERS_Q].begin() + nBuyOrdersToAvg, init, KahanSum);
		vSum[BOOK_BUY_ORDERS_R] = std::accumulate(orderBookVectors[BOOK_BUY_ORDERS_R].begin(), orderBookVectors[BOOK_BUY_ORDERS_R].begin() + nBuyOrdersToAvg, init, KahanSum);
		vSum[BOOK_SELL_ORDERS_Q] = accumulate(orderBookVectors[BOOK_SELL_ORDERS_Q].begin(), orderBookVectors[BOOK_SELL_ORDERS_Q].begin() + nSellOrdersToAvg, init, KahanSum);
		vSum[BOOK_SELL_ORDERS_R] = accumulate(orderBookVectors[BOOK_SELL_ORDERS_R].begin(), orderBookVectors[BOOK_SELL_ORDERS_R].begin() + nSellOrdersToAvg, init, KahanSum);

		vector<ordersMetrics> vMetrics(4);

		vMetrics[BOOK_BUY_ORDERS_Q].avg = vSum[BOOK_BUY_ORDERS_Q].sum / nBuyOrdersToAvg;
		vMetrics[BOOK_BUY_ORDERS_R].avg = vSum[BOOK_BUY_ORDERS_R].sum / nBuyOrdersToAvg;
		vMetrics[BOOK_SELL_ORDERS_Q].avg = vSum[BOOK_SELL_ORDERS_Q].sum / nSellOrdersToAvg;
		vMetrics[BOOK_SELL_ORDERS_R].avg = vSum[BOOK_SELL_ORDERS_R].sum / nSellOrdersToAvg;

		for (int i = 0; i < (int) vMetrics.size(); i++)
		{
			vMetrics[i].stdev = 0.0;
			double m = vMetrics[i].avg;

			for_each(orderBookVectors[i].begin(), orderBookVectors[i].begin() + nBuyOrdersToAvg, [&](const double d)
			{
				vMetrics[i].stdev += (d - m) * (d - m);
			});

			vMetrics[i].stdev = sqrt(vMetrics[i].stdev / (orderBookVectors[i].size() - 1));
		}

		return vMetrics;
	}

	vector<ordersMetrics> callBookOfOffers_UpdateVectors_ReturnMetrics(BittrexAPI& bittrex, const string market, const int depth)
	{
		bittrex.setGetOrdersBook(market, depth);

		bool print = false;
		bool sign = false;
		bool exportReply = false;

		InstuctionOptions instOpt(print, sign, exportReply);
		//Executing instruction
		string buffer = bittrex.callCurlPlataform(instOpt);

		//Example for putting all orders into vectors, quantities and rates
		vector<vector<double> > orderBookVectors = transformBookToVectors(buffer);

		double percentage = 0.25;
		int nOrders = 10;

		return returnOrdersBookAverageValues(orderBookVectors, percentage, nOrders);
//		cout << "average values are:\t";
//		for (int i = 0; i < (int) vMetrics.size(); i++)
//			cout << vMetrics[i].avg << "/" << vMetrics[i].stdev << "\t";
//		cout << endl;
	}

	void callBookOffers_ToTimeSeries_PlusAutomaticActions(BittrexAPI& bittrex, const string market, const int depth)
	{
		int secondsPerUpdate = -5; // no sleep if lower than 0

		vector<int> frequencies =
		{ 1, 5, 10, 60 };
		vector<int> maxSamples =
		{ 3600 * 3, 500, 360 * 3, 36 * 3 };
		OrderBookTimeSeries oBTS(frequencies, maxSamples);

		Timer tTotal;
		for (int i = 0; i < 10000; i++)
		{
			Timer tnow;
			vector<ordersMetrics> vMetrics = callBookOfOffers_UpdateVectors_ReturnMetrics(bittrex, market, depth);

			cout << "average values (bQ,bR,sQ,sR):\t";
			for (int i = 0; i < (int) vMetrics.size(); i++)
				cout << vMetrics[i].avg << "/" << vMetrics[i].stdev << "\t";
			cout << endl;

			oBTS.updateTS(vMetrics, round(tTotal.now()));

			oBTS.callMetrics();

			cout << "spent time was " << tnow.now() << endl;
			for (int ts = 0; ts < oBTS.getNTS(); ts++)
				cout << oBTS.getTSIterSize(ts) << "\t";
			cout << endl;

			int timeToSleep = secondsPerUpdate - tnow.now();
			if (timeToSleep > 0)
				sleep(secondsPerUpdate - tnow.now());
		}
	}

	vector<vector<double> > transformBookToVectors(string buffer)
	{
		vector<vector<double> > buyAndSellOrders;

		size_t pos = buffer.find("sell");
		string bufferBuy = buffer.substr(0, pos);
		string bufferSell = buffer.substr(pos);

		//one iteration for buying and another one for selling
		for (int i = 0; i < 2; i++)
		{

			string bufferToTransform;
			if (i == 0)
				bufferToTransform = bufferBuy;
			else
				bufferToTransform = bufferSell;

			//Cutting vector until Quantity begins
			pos = bufferToTransform.find("Quantity");
			bufferToTransform = bufferToTransform.substr(pos);

			size_t posStartValue = 0, quantityValuePosEnds = 0, posRateValueEnds = 0;

			vector<double> vQuantity;
			vector<double> vRate;
			while (posRateValueEnds < bufferToTransform.size())
			{
				posStartValue = bufferToTransform.find(":");
				quantityValuePosEnds = bufferToTransform.find(",");
				//		cout<<bufferBuy<<endl;
				//		getchar();

				string quantityValue = bufferToTransform.substr(posStartValue + 1, quantityValuePosEnds - posStartValue - 1);
				posRateValueEnds = bufferToTransform.find("}");
				string rateValue = bufferToTransform.substr(quantityValuePosEnds + 8, posRateValueEnds - quantityValuePosEnds - 8);

				//		string quantityValue= bufferBuy.substr (pos+10,11);
				// + 5 is just a number of pushing to the next value
//				cout << "\n\n extracted value is:" << endl;
//				cout << "quantityValue\t" << quantityValue << endl;
//				cout << "rateValue\t" << rateValue << endl;
//				getchar();
				double quantity = stod(quantityValue);
				double rate = stod(rateValue);

				vQuantity.push_back(quantity);
				vRate.push_back(rate);

				bufferToTransform = bufferToTransform.substr(posRateValueEnds + 3);
			}
			buyAndSellOrders.push_back(vQuantity);
			buyAndSellOrders.push_back(vRate);

//			cout << buyAndSellOrders << endl;
//			cout<<"\npress any key to continue"<<endl;
//			getchar();
		}

//		cout<<"\nConverted to vector! press any key to continue"<<endl;
//		getchar();

		return buyAndSellOrders;
	}

};

#endif /*OPTMARKET_HPP*/
