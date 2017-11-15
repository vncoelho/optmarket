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
#include <assert.h>

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

struct marketOperation
{
	int id, type;
	double q, p, t; //quantity, price and total
	struct tm tm;

	//string tradeTimestamp;

	marketOperation()
	{

	}

	marketOperation(int _id, int _type, struct tm _tm, double _q, double _p, double _t) :
			id(_id), type(_type), tm(_tm), q(_q), p(_p), t(_t)
	{

	}
};

ostream& operator<<(ostream &os, const currencyBalance& obj)
{
	os << "balance=" << obj.balance << "\tavailable=" << obj.available << "\tpending=" << obj.pending << endl;
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

struct ordersBookBasicInfo
{
	double avg;
	double stdev;

	ordersBookBasicInfo()
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
	vector<vector<ordersBookBasicInfo> > mTS; //vector of book metrics for with desired frequency
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

	void setTSNewObservation(int ts, vector<ordersBookBasicInfo> vOM)
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

	void updateTSBasicInfo(const vector<ordersBookBasicInfo>& vOM, const long int iter)
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

	vector<double> convertVectorOfOrderMetricsToSumOrStdTS(const vector<vector<ordersBookBasicInfo> > tsOM)
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

	//double percentage is only used when fixed nOrder is not given
	vector<ordersBookBasicInfo> calculateOrdersBook_Basic_Info(const vector<vector<double> > orderBookVectors, const double percentage, const int nOrders = -1)
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

		vector<ordersBookBasicInfo> vMetrics(4);

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
		int sizeMACD = MACDHistogram.size();
		cout << "printing reverse MACD Histogram for TS 0 -- size: " << sizeMACD << endl;
		for (int i = (sizeMACD - 1); i >= 0; i--)
			cout << MACDHistogram[i] << ",";
		cout << "\n============================" << endl;
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

	string getHMAC2(string keyParam, string message)
	{
		//	    char data[] = "Hi! Donate us some coins";
		//		char key[] = "BTC:17GrXw3qdz1G6DRxvtmhnhLzFXjRK5z6NY";
		//		char key[] = "ETH:0xd35fb76723636e11Fa580665A54E19F781C6Cd8a";
		//		char key[] = "ANS:AZZZsZJVqZwfHUx55p6UdY1Vx7qCjhnjMy";

		char key[10000]; //todo huge size for avoiding smashing -- not efficient but works
		char data[10000];
		strcpy(key, keyParam.c_str());
		strncpy(data, message.c_str(), sizeof(data));

		unsigned char* digest;

		//============================================
		//Example extracted from http://www.askyb.com/cpp/openssl-hmac-hasing-example-in-cpp/
		//accessed on February, 26th 2013 in C++ OpenSSL
		//=============================================
		// Using sha1 hash engine here.
		// You may use other hash engines. e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
		digest = HMAC(EVP_sha512(), key, strlen(key), (unsigned char*) data, strlen(data), NULL, NULL);

		// Be careful of the length of string with the chosen hash engine.
		//SHA1 produces a 20-byte hash value which rendered as 40 characters.
		// Change the length accordingly with your chosen hash engine
		char mdString[SHA512_DIGEST_LENGTH];
		for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
			sprintf(&mdString[i * 2], "%02x", (unsigned int) digest[i]);

		//printf("HMAC digest: %s\n", mdString);
		//=============================================

		string output = mdString;
		return output;
	}

	static size_t writerToString(void *contents, size_t size, size_t nmemb, void *userp)
	{
		((string*) userp)->append((char*) contents, size * nmemb);
		size_t realsize = size * nmemb;
		return realsize;
	}

	string callCurlPlataform(const InstuctionOptions& instOpt, const stringstream& ssURL)
	{
		//For more information: check: https://curl.haxx.se/libcurl/c/getinmemory.html
		if (ssURL.str() == "")
		{
			cout << "empty instruction." << endl;
			getchar();
			return "";
		}
		else
		{
			cout << "\n=============================================" << endl;
			cout << "Executing instruction\t Signed:" << instOpt.signMessage << "\n" << ssURL.str() << endl;
			//			cout << "press any key to continue..." << endl;
			cout << "=============================================" << endl;
			//			getchar();
		}

		CURL *curl_handle;

		string readBuffer;

		curl_global_init(CURL_GLOBAL_ALL);

		// curl session begins
		curl_handle = curl_easy_init();

		// set destination URL
		curl_easy_setopt(curl_handle, CURLOPT_URL, (const char* ) ssURL.str().c_str());

		/* signing instruction */
		if (instOpt.signMessage == true)
		{
			string signature = getHMAC2(API_PRIVATE_KEY, ssURL.str());
			cout << "\n----------------------------------------------" << endl;
			cout << "HMAC signature (header purpose): \n" << signature << endl;
			cout << "----------------------------------------------" << endl;
			stringstream ssSignature;
			ssSignature << "apisign:" << signature;

			struct curl_slist *headerlist = NULL;
			headerlist = curl_slist_append(headerlist, (const char*) ssSignature.str().c_str());

			//setting header with signature
			curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headerlist);
		}

		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

		// send all data to this function
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writerToString);
		// and filling a string buffer
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);

		//let's get it
		CURLcode ans = curl_easy_perform(curl_handle);

		/* check for errors */
		if (ans != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() found problems (failed): %s\n", curl_easy_strerror(ans));
			return "curl_easy_perform Failed Error";
			//exit(1);
		}
		else
		{
			cout << "\n=============================================" << endl;
			cout << "=============================================" << endl;
			cout << "Server answer:" << endl;
			if (instOpt.printAnswer)
				cout << readBuffer << endl;
			else
				cout << "message optionally omitted" << endl;

			cout << "=============================================\n" << endl;
			//		getchar();
		}

		if (instOpt.exportBuffer)
		{
			cout << "=============================================" << endl;
			cout << "Exporting header and body messages..." << endl;
			exportReply(curl_handle);
			cout << "=============================================" << endl;
		}

		/* cleanup curl stuff */
		curl_easy_cleanup(curl_handle);

		return readBuffer;
	}

	// ===========================================================
	// ===========================================================
	//	Other useful functions
	// ===========================================================
	static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
		return written;
	}

	void exportReply(CURL* curl_handle)
	{
		static const char *headerfilename = "headAnswer.out";
		FILE *headerfile;
		static const char *bodyfilename = "bodyAnswer.out";
		FILE *bodyfile;

		CURLcode res = curl_easy_perform(curl_handle);

		/* open the header file */
		headerfile = fopen(headerfilename, "wb");
		if (!headerfile)
		{
			curl_easy_cleanup(curl_handle);
			cout << "error on exporting header file!" << endl;
			exit(-1);
		}

		/* open the body file */
		bodyfile = fopen(bodyfilename, "wb");
		if (!bodyfile)
		{
			curl_easy_cleanup(curl_handle);
			fclose(headerfile);
			cout << "error on exporting body file!" << endl;
			exit(-1);

		}

		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

		/* we want the headers be written to this file handle */
		curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, headerfile);

		/* we want the body be written to this file handle instead of stdout */
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, bodyfile);

		/* get it! */
		curl_easy_perform(curl_handle);

		/* close the header file */
		fclose(headerfile);

		/* close the body file */
		fclose(bodyfile);
	}
	// ===========================================================
	// ===========================================================

	bool checkFail(const string buffer)
	{
		cout << buffer << endl;
		size_t posFalse = buffer.find("false");
		size_t posBad = buffer.find("Bad");
		size_t posError = buffer.find("Error");
		if ((posFalse < buffer.size()) || (posBad < buffer.size()) || (posError < buffer.size()))
			return false;

		return true;
	}

	void printFail(bool check)
	{
		if (!check)
			cerr << "Error" << endl;

	}

	currencyBalance filterBittrexBalanceForSpecificMarket(string buffer, string market)
	{
		assert(checkFail(buffer));
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

	void optGetBalanceBittrex(BittrexAPI& bittrex, const string market)
	{
		bool print, sign, exportReply;

		bittrex.setGetBalance(market);
		string buffer = callCurlPlataform(InstuctionOptions(print = false, sign = true, exportReply = false), bittrex.getURL());
		currencyBalance cBT = filterBittrexBalanceForSpecificMarket(buffer, market);

		cout << "optMarket::" << cBT << endl;
		getchar();
	}

	double optGetLastTradeBittrex(BittrexAPI& bittrex, const string market)
	{
		bool print, sign, exportReply;
		double lastTrade = -1;

		bittrex.setGetTicker(market);
		string buffer = callCurlPlataform(InstuctionOptions(print = false, sign = false, exportReply = false), bittrex.getURL());
		//assert(checkFail(buffer));
		bool check = checkFail(buffer);
		if (!check)
			printFail(check);
		else
		{
			size_t pos = buffer.find("Last");

			string last = buffer.substr(pos + 6, buffer.size() - 2 - pos - 6);
			lastTrade = stod(last);

			cout << "optMarket::LastTradeWas:" << lastTrade << endl;
		}
		return lastTrade;
	}

	string cutJSONObjVar(string& stringJsonToCut)
	{
		size_t posB = stringJsonToCut.find(":");
		size_t posE1 = stringJsonToCut.find(",");
		size_t posE2 = stringJsonToCut.find("}");
		size_t posE = min(posE1, posE2);
		string var = stringJsonToCut.substr(posB + 1, posE - posB - 1);
		stringJsonToCut = stringJsonToCut.substr(posE + 2);
//		cout << var << endl;
//		cout<<stringJsonToCut<<endl;
//		getchar();
		return var;
	}

	vector<marketOperation> optGetMarketHistoryBittrex(BittrexAPI& bittrex, const string market)
	{
		bool print, sign, exportReply;

		bittrex.setGetMarketHistory(market);
		string buffer = callCurlPlataform(InstuctionOptions(print = false, sign = false, exportReply = false), bittrex.getURL());
		//assert(checkFail(buffer));
		bool check = checkFail(buffer);
		if (!check)
			printFail(check);
		else
		{

			buffer = buffer.substr(buffer.find("[{"));
			cout << buffer << endl;

			bool finish = false;
			while (!finish)
			{
				marketOperation mOP;
				mOP.id = stoi(cutJSONObjVar(buffer));
				string timestamp = cutJSONObjVar(buffer);
				mOP.q = stod(cutJSONObjVar(buffer));
				mOP.p = stod(cutJSONObjVar(buffer));
				mOP.t = stod(cutJSONObjVar(buffer));
				cutJSONObjVar(buffer); //skip fillType
				string orderType = cutJSONObjVar(buffer);
				cout << mOP.id << endl;
				cout << timestamp << endl;
				cout << mOP.p << endl;
				cout << mOP.q << endl;
				cout << mOP.t << endl;
				cout << orderType << endl;
				getchar();
			}
			getchar();
			size_t pos = buffer.find("Last");

			string last = buffer.substr(pos + 6, buffer.size() - 2 - pos - 6);
			double lastTrade = stod(last);

			cout << "optMarket::LastTradeWas:" << lastTrade << endl;
			//return lastTrade;
		}

	}

	void optSellCalculatingProfit(BittrexAPI& bittrex, double quantity, double rate, string market)
	{
		double theoricalTotal = (quantity * rate);
		double costs = theoricalTotal * (bittrex.getFeesPerOrder());
		double realTotal = theoricalTotal - costs;
		double actualRate = realTotal / rate;
		cout << "optMarket:: selling rate:" << rate << "\tq:" << quantity << "\ntTotal:" << theoricalTotal << "\trTotal:" << realTotal << "\tcosts:" << costs << "\taRate:" << actualRate << endl;

		string baseCurrency = market.substr(0, market.find("-"));
		string tradedCurrency = market.substr(market.find("-") + 1);

		bool print = true;
		bool sign = true; //If you want to sign
		bool exportReply = false;

		InstuctionOptions instOpt(print, sign, exportReply);

		bittrex.setGetBalance(tradedCurrency);
		string buffer = callCurlPlataform(InstuctionOptions(false, true, false), bittrex.getURL());
		currencyBalance cBT = filterBittrexBalanceForSpecificMarket(buffer, tradedCurrency);
		currencyBalance cBB = filterBittrexBalanceForSpecificMarket(buffer, baseCurrency);

		bittrex.setSellLimit(market, quantity, rate);
		buffer = callCurlPlataform(InstuctionOptions(true, true, false), bittrex.getURL());

		//TODO - Create a function for checking pending orders and wait
		//Go to the multicore, multigpu and run all this in parallel

		bittrex.setGetBalance(tradedCurrency);
		buffer = callCurlPlataform(InstuctionOptions(false, true, false), bittrex.getURL());
		currencyBalance cBTNew = filterBittrexBalanceForSpecificMarket(buffer, tradedCurrency);
		currencyBalance cBBNew = filterBittrexBalanceForSpecificMarket(buffer, baseCurrency);

		cout << "optMarket::traded(Old,New):(" << cBT.balance << "," << cBTNew.balance << ")" << endl;
		cout << "Base(Old,New):(" << cBB.balance << "," << cBBNew.balance << ")" << endl;
		cout << "Sold value of traded is: " << cBTNew.balance - cBT.balance << endl;
		cout << "Amount of coins bought for the base currency is: " << cBBNew.balance - cBB.balance << endl;
		getchar();

	}

	vector<vector<double> > callBookOfOffers_UpdateVectors(BittrexAPI& bittrex, const string market, const int depth)
	{
		bittrex.setGetOrdersBook(market, depth);

		bool print, sign, exportReply;

		InstuctionOptions instOpt(print = false, sign = false, exportReply = false);
		//Executing instruction
		string buffer = callCurlPlataform(instOpt, bittrex.getURL());

		//Example for putting all orders into vectors, quantities and rates
		return transformBookToVectors(buffer);
	}

	void callBookOffers_ToTimeSeries_PlusAutomaticActions(BittrexAPI& bittrex, const string market, const int depth)
	{
		int secondsPerUpdate = -5; // if lower than 0, no sleep

		//frequencies (seconds) and maxSamples determines the TS that will be kept
		vector<int> frequencies =
		{ 1, 5, 10, 60 };
		vector<int> maxSamples =
		{ 3600 * 3, 500, 360 * 3, 36 * 3 };

		//Class with all Time Series from the Order Book
		OrderBookTimeSeries oBTS(frequencies, maxSamples);

		Timer tTotal; //Timer used for updating time series
		int nMaxOfIterations = 10000;
		for (int i = 0; i < nMaxOfIterations; i++)
		{
			Timer tnow;
			vector<vector<double> > vBookOfOrders = callBookOfOffers_UpdateVectors(bittrex, market, depth);

			double percentage = 0.25;
			int nOrders = 10;
			vector<ordersBookBasicInfo> vAvgStd = oBTS.calculateOrdersBook_Basic_Info(vBookOfOrders, percentage, nOrders);

			cout << "optMarket:: |AVG/SD| -- |depth,depth*%,nOrders|: " << depth << "/" << depth * percentage << "/" << nOrders << " |buyQ,buyR,sellQ,sellR|\n";
			for (int i = 0; i < (int) vAvgStd.size(); i++)
				cout << vAvgStd[i].avg << "/" << vAvgStd[i].stdev << "\t";
			cout << endl;

			oBTS.updateTSBasicInfo(vAvgStd, round(tTotal.now()));
			oBTS.callMetrics();

			cout << "optMarket::Spent time:" << tnow.now() << endl;
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
