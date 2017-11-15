#ifndef BITTREX_API_HPP
#define BITTREX_API_HPP

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <cstring>
#include <sstream>
//#include <crypt.h>

#include <curl/curl.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

using namespace std;

extern string API_PUBLIC_KEY;
extern string API_PRIVATE_KEY;

//Options for signing message with your private key and/or exporting json to file or string
struct InstuctionOptions
{
	bool printAnswer, exportBuffer, signMessage;
	InstuctionOptions(bool _printAnswer = true, bool _signMessage = false, bool _exportBuffer = false) :
			printAnswer(_printAnswer), signMessage(_signMessage), exportBuffer(_exportBuffer)
	{

	}
};

class BittrexAPI
{
private:
	stringstream ssURL;
	double feePerOrder = 0.0025; //market fees for order = 0,25%

public:

	BittrexAPI()
	{

	}

	virtual ~BittrexAPI()
	{
	}

	double getFeesPerOrder()
	{
		return feePerOrder;
	}



	double getNonce()
	{
		return time(NULL);
	}

	const stringstream& getURL()
	{
		return ssURL;
	}

	// ===========================================================
	//Public API
	// ===========================================================

	void setGetMarketSummary(string market)
	{
		ssURL.str("");
		ssURL << "https://bittrex.com/api/v1.1/public/getmarketsummary?market=" << market;
	}

	void setGetOrdersBook(const string market, const int depth)
	{
		ssURL.str("");
		ssURL << "https://bittrex.com/api/v1.1/public/getorderbook?market=" << market << "&type=both&depth=" << depth;
	}

	void setGetTicker(const string market)
	{
		ssURL.str("");
		ssURL << "https://bittrex.com/api/v1.1/public/getticker?market=" << market;
	}

	void setGetMarketHistory(string market)
	{
		ssURL.str("");
		ssURL << "https://bittrex.com/api/v1.1/public/getmarkethistory?market=" << market;
	}

	// ===========================================================
	//Market
	// ===========================================================

	void setBuyLimit(string market, double quantity, double rate)
	{
		ssURL.str("");
		ssURL << "https://bittrex.com/api/v1.1/market/buylimit?apikey=" << API_PUBLIC_KEY << "&nonce=" << getNonce() << "&market=" << market << "&quantity=" << quantity << "&rate=" << rate;

	}

	void setSellLimit(string market, double quantity, double rate)
	{
		ssURL.str("");
		ssURL << "https://bittrex.com/api/v1.1/market/selllimit?apikey=" << API_PUBLIC_KEY << "&nonce=" << getNonce() << "&market=" << market << "&quantity=" << quantity << "&rate=" << rate;

	}

	void setCancelOrder(string uuid)
	{
		ssURL.str("");
		ssURL << "https://bittrex.com/api/v1.1/market/cancel?apikey=" << API_PUBLIC_KEY << "&nonce=" << getNonce() << "&uuid=" << uuid;
	}

	void setVerifyMyOpenOrders(string market = "empty")
	{
		ssURL.str("");
		if (market != "empty")
			ssURL << "https://bittrex.com/api/v1.1/market/getopenorders?apikey=" << API_PUBLIC_KEY << "&nonce=" << getNonce() << "&market=" << market;
		else
			ssURL << "https://bittrex.com/api/v1.1/market/getopenorders?apikey=" << API_PUBLIC_KEY << "&nonce=" << getNonce();
	}

	// ===========================================================
	// ===========================================================

	// ===========================================================
	//Account Api
	// ===========================================================

	void setGetBalance(string currency = "empty")
	{
		ssURL.str("");
		if (currency != "empty")
			ssURL << "https://bittrex.com/api/v1.1/account/getbalances?apikey=" << API_PUBLIC_KEY << "&nonce=" << getNonce() << "&currency=" << currency;
		else
			ssURL << "https://bittrex.com/api/v1.1/account/getbalances?apikey=" << API_PUBLIC_KEY << "&nonce=" << getNonce();
	}

	//TODO
	//	ssURL << "https://bittrex.com/api/v1.1/account/getdepositaddress?apikey="
	//			<< apikey << "&nonce=" << nonce << "&currency=DOGE";


};

#endif /*BITTREX_API_HPP*/
