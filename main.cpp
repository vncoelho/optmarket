#include <stdlib.h>     /* getenv */
#include <unistd.h> /* sleep */
#include <sstream>

#include "bittrexApi.hpp"
#include "optMarket.hpp"

using namespace std;

string apikey, apisecret;

void readKeys(string& apikey, string& apisecret)
{
	//TODO
	//[17:15, 3/7/2017] Igor Machado: Mais simples, no terminal:  $ export CHAVE="1234"
	//[17:16, 3/7/2017] Igor Machado: No código: char* val=getenv("CHAVE") ;

	string str;
	ifstream t("keys.input");
	getline(t, str);
	getline(t, apikey);
	getline(t, str);
	getline(t, apisecret);

//	cout << apikey << endl;
//	cout << apisecret << endl;
//	getchar();
}
int main(void)
{
	//Reading apikey and secret located at keys.input
	readKeys(apikey, apisecret);

	// Bittrex main class
	BittrexAPI bittrex;

	// Optimization market functions with templates for automatic and expert training
	OptMarket optMarket;
	//Other -- Precise printing messages
	cout << std::setprecision(10);
	cout << std::fixed;

	double quantity = 0.01;
	double rate = 0.00313001;
	optMarket.optSellCalculatingProfit(bittrex,quantity,rate,"BTC-NEO");
//	optMarket.callBookOffers_ToTimeSeries_PlusAutomaticActions(bittrex, "BTC-NEO", 60);
//
	optMarket.optGetBalanceBittrex(bittrex,"NEO");

	optMarket.optGetBalanceBittrex(bittrex,"NEO");

	bittrex.setGetOrdersBook("BTC-NEO", 1);
	bittrex.setGetMarketSummary("BTC-NEO");
	bittrex.setGetBalance("NEO");
	bittrex.setCancelOrder("uuid");
	bittrex.setSellLimit("BTC-NEO", quantity, rate);
	bittrex.setBuyLimit("BTC-NEO", quantity, rate); //Min 50K * 10^-8
	bittrex.setVerifyMyOpenOrders("BTC-NEO");
	bittrex.setGetOrdersBook("BTC-NEO", 1);
	bittrex.setGetBalance("ANS");
	bittrex.setGetOrdersBook("BTC-NEO", 1);
	bittrex.setGetBalance("NEO");
	bittrex.setGetOrdersBook("BTC-ETH", 1);
	bittrex.setGetMarketSummary("BTC-NEO");

	bittrex.setGetBalance("BTC");

	bool print = true;
	bool sign = true;
	bool exportReply = false;

	InstuctionOptions instOpt(print, sign, exportReply);
	//Executing instruction
	string buffer = bittrex.callCurlPlataform(instOpt);

//	//Example for putting all orders into vectors, quantities and rates
//	vector<vector<double> > orderBookVectors = optMarket.transformBookToVectors(buffer);

	cout << "Finished with success!!" << endl;
	return 0;
}
