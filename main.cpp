#include <bits/stdc++.h>
#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstdlib>
#include <sys/stat.h> // for mkdir
#include <sys/types.h>
#include <direct.h> // for _mkdir on Windows
#include <regex>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#define MKDIR(dir) _mkdir(dir)

using namespace std;

struct Transaction
{
    string crypto;
    double amount;
    double price;
    string type;
    string timestamp;
};

struct Transactions
{
    int buyDay = -1;
    int sellDay = -1;
    int profit = 0;
};
int maxProfitWithBuySell(const vector<int> &prices, vector<Transactions> &transactions)
{
    int n = prices.size();
    if (n == 0)
        return 0;

    vector<int> leftProfit(n, 0), rightProfit(n, 0);
    vector<int> leftBuy(n, 0), rightSell(n, 0);

    int minPrice = prices[0], minDay = 0;
    for (int i = 1; i < n; ++i)
    {
        if (prices[i] < minPrice)
        {
            minPrice = prices[i];
            minDay = i;
        }
        if (prices[i] - minPrice > leftProfit[i - 1])
        {
            leftProfit[i] = prices[i] - minPrice;
            leftBuy[i] = minDay;
        }
        else
        {
            leftProfit[i] = leftProfit[i - 1];
            leftBuy[i] = leftBuy[i - 1];
        }
    }

    int maxPrice = prices[n - 1], maxDay = n - 1;
    for (int i = n - 2; i >= 0; --i)
    {
        if (prices[i] > maxPrice)
        {
            maxPrice = prices[i];
            maxDay = i;
        }
        if (maxPrice - prices[i] > rightProfit[i + 1])
        {
            rightProfit[i] = maxPrice - prices[i];
            rightSell[i] = maxDay;
        }
        else
        {
            rightProfit[i] = rightProfit[i + 1];
            rightSell[i] = rightSell[i + 1];
        }
    }

    int maxTotalProfit = 0;
    for (int i = 0; i < n; ++i)
    {
        int total = leftProfit[i] + rightProfit[i];
        if (total > maxTotalProfit)
        {
            maxTotalProfit = total;
            transactions.clear();
            if (leftProfit[i] > 0)
                transactions.push_back({leftBuy[i], i, leftProfit[i]});
            if (rightProfit[i] > 0)
                transactions.push_back({i, rightSell[i], rightProfit[i]});
        }
    }

    return maxTotalProfit;
}

struct DayPrice {
    string date;
    double averagePrice;
};

int calculateMaxProfitForCoin() {
    string filename = "prices_30days_minmax.csv";
    string coin;
    cout << "Enter coin name: ";
    cin >> coin;

    ifstream file(filename);
    if (!file) {
        cerr << "File not found.\n";
        return 1;
    }

    vector<DayPrice> prices;
    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        stringstream ss(line);
        string date, name, minStr, maxStr;
        getline(ss, date, ',');
        getline(ss, name, ',');
        getline(ss, minStr, ',');
        getline(ss, maxStr, ',');

        if (name == coin) {
            double minP = stod(minStr);
            double maxP = stod(maxStr);
            prices.push_back({date, (minP + maxP) / 2});
        }
    }

    if (prices.size() < 2) {
        cout << "Not enough data to optimize.\n";
        return 0;
    }

    double minPrice = prices[0].averagePrice;
    string minDate = prices[0].date;
    double maxProfit = 0;
    string buyDate, sellDate;

    for (size_t i = 1; i < prices.size(); ++i) {
        double profit = prices[i].averagePrice - minPrice;
        if (profit > maxProfit) {
            maxProfit = profit;
            buyDate = minDate;
            sellDate = prices[i].date;
        }
        if (prices[i].averagePrice < minPrice) {
            minPrice = prices[i].averagePrice;
            minDate = prices[i].date;
        }
    }
    cout << fixed << setprecision(0); // Ensures numerical output with 2 decimal places
    cout << "\n--- Price Optimization for " << coin << " ---\n";
    if (maxProfit > 0) {
        cout << "Buy on : " << buyDate << "\n";
        cout << "Sell on: " << sellDate << "\n";
        cout << "Max Profit: " << maxProfit <<" INR"<< "\n";
    } else {
        cout << "No profitable opportunity found in the given range.\n";
    }

    return 0;
}
string toUpper(const string &input)
{
    string upper = input;
    transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    return upper;
}

int getValidInput(const string &prompt, int min, int max)
{
    int value;
    while (true)
    {
        cout << prompt;
        cin >> value;
        if (!cin.fail() && value >= min && value <= max)
            break;
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Please enter a number between " << min << " and " << max << ".\n";
    }
    return value;
}

string currentTimestamp()
{
    time_t now = time(0);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", localtime(&now));
    return string(buf);
}

class CryptoTracker
{
public:
    vector<pair<string, double>> getCryptos()
    {
        vector<pair<string, double>> cryptos;
        ifstream file("prices.csv");

        if (!file.is_open())
        {
            cerr << "❌ Unable to open prices.csv\n";
            return cryptos;
        }

        string line;
        getline(file, line); // Skip header

        while (getline(file, line))
        {
            stringstream ss(line);
            string name, priceStr;
            if (getline(ss, name, ',') && getline(ss, priceStr, ','))
            {
                transform(name.begin(), name.end(), name.begin(), ::tolower);
                double price = stod(priceStr);
                cryptos.emplace_back(name, price);
            }
        }

        file.close();
        return cryptos;
    }
};

class CryptoApp
{
    CryptoTracker tracker;
    vector<Transaction> transactions;
    map<string, double> holdings;
    vector<string> watchlist;

    void addTransaction(long loginID)
    {
        vector<pair<string, double>> cryptos = tracker.getCryptos();
        cout << fixed << setprecision(2);
        for (int i = 0; i < cryptos.size(); ++i)
        {
            cout << i + 1 << ". " << toUpper(cryptos[i].first) << " - " << cryptos[i].second << " INR\n";
        }

        int choice = getValidInput("Select crypto: ", 1, cryptos.size());
        string crypto = cryptos[choice - 1].first;
        double marketPrice = cryptos[choice - 1].second;

        string type;
        cout << "Type (buy/sell): ";
        cin >> type;

        double amount;
        cout << "Enter amount: ";
        cin >> amount;

        string folderName = to_string(loginID);
        string filePath = "users/" + folderName + "/assets.csv";

        map<string, tuple<double, double>> assets; // name -> (quantity, price)
        double cashBalance = 0;

        ifstream inFile(filePath);
        string line;
        getline(inFile, line); // Skip header
        while (getline(inFile, line))
        {
            stringstream ss(line);
            string name;
            double qty, price, total;
            getline(ss, name, ',');
            ss >> qty;
            ss.ignore();
            ss >> price;
            ss.ignore();
            ss >> total;

            if (name == "Cash")
                cashBalance = total;
            else
                assets[name] = make_tuple(qty, price);
        }
        inFile.close();

        double transactionCost = amount * marketPrice;

        if (type == "buy")
        {
            if (transactionCost > cashBalance)
            {
                cout << "❌ Not enough cash. Available: ₹" << fixed << setprecision(2) << cashBalance
                     << ", Required: ₹" << transactionCost << "\n";
                return;
            }
            holdings[crypto] += amount;
            cashBalance -= transactionCost;

            auto &[qty, price] = assets[crypto];
            qty += amount;
            price = marketPrice;
        }
        else if (type == "sell")
        {
            double heldQty = get<0>(assets[crypto]);
            if (heldQty < amount)
            {
                cout << "❌ Not enough crypto to sell. Available: " << fixed << setprecision(2)
                     << heldQty << ", Required: " << amount << "\n";
                return;
            }
            holdings[crypto] -= amount;
            cashBalance += transactionCost;

            auto &[qty, price] = assets[crypto];
            qty -= amount;
            if (qty < 0)
                qty = 0;
        }
        else
        {
            cout << "❌ Invalid transaction type.\n";
            return;
        }

        Transaction tx = {crypto, amount, marketPrice, type, currentTimestamp()};
        transactions.push_back(tx);

        // Update asset file (with fixed + setprecision for normal form)
        ofstream outFile(filePath);
        outFile << fixed << setprecision(0);
        outFile << "Name,Quantity,Price,Total\n";
        outFile << "Cash," << 0 << "," << cashBalance << "," << cashBalance << "\n";
        for (auto &[name, tup] : assets)
        {
            double q = get<0>(tup), p = get<1>(tup);
            if (name != "Cash" && q > 0)
                outFile << name << "," << q << "," << p << "," << q * p << "\n";
        }
        outFile.close();

        // Append transaction
        string txFilePath = "users/" + folderName + "/transactions.csv";
        ofstream txFile(txFilePath, ios::app);
        txFile << fixed << setprecision(0);
        txFile.seekp(0, ios::end);
        if (txFile.tellp() == 0)
            txFile << "Crypto,Amount,Price,Type,Timestamp\n";
        txFile << crypto << "," << amount << "," << marketPrice << "," << type << "," << currentTimestamp() << "\n";
        txFile.close();

        cout << "✅ Transaction complete. Assets updated.\n";
    }


    void showTransactions(long loginID)
    {
        string folderName = to_string(loginID);
        string filePath = "users/" + folderName + "/transactions.csv";

        ifstream file(filePath);
        if (!file)
        {
            cout << "\n❌ No transaction history found.\n";
            return;
        }

        string line;
        getline(file, line); // Skip header

        cout << "\nTransaction History:\n";
        while (getline(file, line))
        {
            stringstream ss(line);
            string crypto, amountStr, priceStr, type, timestamp;
            getline(ss, crypto, ',');
            getline(ss, amountStr, ',');
            getline(ss, priceStr, ',');
            getline(ss, type, ',');
            getline(ss, timestamp);

            double amount = stod(amountStr);
            double price = stod(priceStr);

            cout << toUpper(crypto) << " | " << type << " | Amount: " << fixed << setprecision(2) << amount
                 << " @ Price/Amount: " << fixed << setprecision(2) << price << " | " << timestamp << "\n";
        }

        file.close();
    }
    string toLower(const string &s)
    {
        string result = s;
        for (char &c : result)
            c = tolower(c);
        return result;
    }

    void showPortfolioValue(long loginID)
    {
        string folderName = to_string(loginID);
        string assetPath = "users/" + folderName + "/assets.csv";
        string pricePath = "Prices.csv";

        ifstream assetFile(assetPath);
        if (!assetFile)
        {
            cout << "❌ Asset file not found.\n";
            return;
        }

        // Load prices from prices.csv into a map
        map<string, double> priceMap;
        ifstream priceFile(pricePath);
        if (!priceFile)
        {
            cout << "❌ Price file not found.\n";
            return;
        }

        string pline;
        while (getline(priceFile, pline))
        {
            stringstream ss(pline);
            string crypto;
            double price;
            getline(ss, crypto, ',');
            ss >> price;
            priceMap[toLower(crypto)] = price;
        }
        priceFile.close();

        // Now read assets and compute total value
        string line;
        getline(assetFile, line); // Skip header

        double total = 0.0;
        cout << "\nCurrent Portfolio:\n";
        cout << fixed << setprecision(2);

        while (getline(assetFile, line))
        {
            stringstream ss(line);
            string name, qtyStr, priceStr, totalStr;
            getline(ss, name, ',');
            getline(ss, qtyStr, ',');
            getline(ss, priceStr, ',');
            getline(ss, totalStr, ',');

            double quantity = stod(qtyStr);
            double displayPrice, value;

            if (name == "Cash")
            {
                displayPrice = stod(priceStr);
                value = displayPrice;
                cout << "Cash Balance: " << displayPrice << "\n";
            }
            else
            {
                string lowerName = toLower(name);
                if (priceMap.find(lowerName) != priceMap.end())
                {
                    displayPrice = priceMap[lowerName];
                    value = quantity * displayPrice;
                    cout << toUpper(name) << ": " << quantity << " units @ " << displayPrice << " = " << value << " INR\n";
                }
                else
                {
                    cout << toUpper(name) << ": " << quantity << " units @ ❌ Price not found\n";
                    value = 0;
                }
            }

            total += value;
        }

        assetFile.close();
        cout << "\nTotal Portfolio Value: " << total << " INR\n";
    }

    void viewCurrentAssets(long loginID)
    {
        string folderName = to_string(loginID);
        string filePath = "users/" + folderName + "/assets.csv";
        ifstream file(filePath);

        if (!file.is_open())
        {
            cout << "No asset data found for user " << loginID << ".\n";
            return;
        }

        cout << "\t\t" << left << setw(15) << "Name"
             << setw(15) << "Quantity"
             << setw(20) << "Price (INR)"
             << setw(20) << "Total Value (INR)" << "\n";

        cout << "\t\t" << string(70, '-') << "\n";
        int i = 1;
        string line;
        while (getline(file, line))
        {
            if (i == 1)
            {
                i++;
                continue;
            }
            stringstream ss(line);
            string name, quantity, price, total;
            getline(ss, name, ',');
            getline(ss, quantity, ',');
            getline(ss, price, ',');
            getline(ss, total, ',');

            cout << "\t\t" << left << setw(15) << name
                 << setw(15) << quantity
                 << setw(20) << price
                 << setw(20) << total << "\n";
        }

        file.close();
        cout << endl;
    }

public:
bool isFileEmpty(const string &filePath) {
    ifstream file(filePath);
    return file.peek() == ifstream::traits_type::eof();
}

void storeCurrentAssets(long loginID)
{
    string folderName = to_string(loginID);
    string filePath = "users/" + folderName + "/assets.csv";

    bool writeHeader = isFileEmpty(filePath);

    ofstream file(filePath, ios::app);
    file << fixed << setprecision(0);

    if (writeHeader) {
        file << "Name,Quantity,Price,Total\n";
    }

    double cash;
    cout << "\t\tEnter available cash (INR): ";
    cin >> cash;
    file << "Cash,0," << cash << "," << cash << "\n";

    int n;
    cout << "\t\tEnter number of cryptocurrencies you hold: ";
    cin >> n;

    for (int i = 0; i < n; ++i)
    {
        string name;
        double quantity, price;
        cout << "\t\tEnter name of cryptocurrency: ";
        cin >> name;
        cout << "\t\tEnter quantity: ";
        cin >> quantity;
        cout << "\t\tEnter purchase price per unit (INR): ";
        cin >> price;
        file << name << "," << quantity << "," << price << "," << quantity * price << "\n";
    }

    file.close();
}

    void showPrices()
    {
        ifstream file("Prices.csv");
        if (!file.is_open())
        {
            cerr << "❌ Could not open prices.csv\n";
            return;
        }

        string line, name, price;
        getline(file, line); // skip header

        cout << left << setw(15) << "\t\tCryptocurrency" << right << setw(15) << "Price (INR)\n";
        cout << "\t\t" << string(30, '-') << "\n";

        while (getline(file, line))
        {
            stringstream ss(line);
            getline(ss, name, ',');
            getline(ss, price);
            cout << "\t\t" << left << setw(15) << name << right << setw(15) << price << "\n";
        }
        cout << endl;
        file.close();
    }

    void portfolioMenu(long loginID)
    {
        while (true)
        {
            cout << "\n--- Portfolio Menu ---\n";
            cout << "1. Add Transaction\n2. Show Transactions\n3. Show Holdings\n4. Show Portfolio Value\n5. Back to Main\n";
            int choice = getValidInput("Enter choice: ", 1, 5);
            if (choice == 5)
                break;
            switch (choice)
            {
            case 1:
                addTransaction(loginID);
                break;
            case 2:
                showTransactions(loginID);
                break;
            case 3:
                viewCurrentAssets(loginID);
                break;
            case 4:
                showPortfolioValue(loginID);
                break;
            }
        }
    }

    void manageWatchlist()
    {
        while (true)
        {
            cout << "\n--- Watchlist Menu ---\n";
            cout << "1. View Watchlist\n2. Add to Watchlist\n3. Remove from Watchlist\n4. Back\n";
            int choice = getValidInput("Enter choice: ", 1, 4);
            if (choice == 4)
                break;
            switch (choice)
            {
            case 1:
                cout << "\nYour Watchlist:\n";
                for (auto &c : watchlist)
                    cout << toUpper(c) << "\n";
                break;
            case 2:
            {
                string c;
                cout << "Enter crypto to add: ";
                cin >> c;
                watchlist.push_back(c);
                cout << toUpper(c) << " added to watchlist.\n";
                break;
            }
            case 3:
            {
                string c;
                cout << "Enter crypto to remove: ";
                cin >> c;
                watchlist.erase(remove(watchlist.begin(), watchlist.end(), c), watchlist.end());
                cout << toUpper(c) << " removed from watchlist.\n";
                break;
            }
            }
        }
    }
};
void escape();
void login();
void createAccount();
void homePage();
int mainMenu();

struct Credentials
{
    string name;
    long loginID;
    long password;
    bool verified;
};

struct Edge {
    int from, to;
    double weight;
};

string stripQuotes(const string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

void findArbitrage() {
    unordered_map<string, int> currencyIndex;
    unordered_map<string, double> inrPrices;
    vector<string> currencies;
    vector<Edge> edges;

    ifstream priceFile("Prices.csv");
    if (!priceFile.is_open()) {
        cerr << "Failed to open Prices.csv\n";
        return;
    }

    string line;
    getline(priceFile, line); // skip header
    while (getline(priceFile, line)) {
        stringstream ss(line);
        string name, priceStr;
        getline(ss, name, ',');
        getline(ss, priceStr, ',');
        name = stripQuotes(name);
        priceStr = stripQuotes(priceStr);
        if (!priceStr.empty()) {
            double price = stod(priceStr);
            inrPrices[name] = price;
        }
    }
    priceFile.close();

    unordered_map<string, string> nameToSymbol = {
        {"Bitcoin", "BTC"}, {"Ethereum", "ETH"}, {"Solana", "SOL"},
        {"Ripple", "XRP"}, {"Dogecoin", "DOGE"}, {"Cardano", "ADA"}
    };

    for (const auto& pair : inrPrices) {
        string symbol = nameToSymbol[pair.first];
        if (currencyIndex.find(symbol) == currencyIndex.end()) {
            currencyIndex[symbol] = currencies.size();
            currencies.push_back(symbol);
        }
    }

    ifstream rateFile("crypto_pairwise_rates_cryptocompare.csv");
    if (!rateFile.is_open()) {
        cerr << "Failed to open crypto_pairwise_rates_cryptocompare.csv\n";
        return;
    }

    getline(rateFile, line); // skip header
    while (getline(rateFile, line)) {
        stringstream ss(line);
        string timestamp, from, to, rateStr;
        getline(ss, timestamp, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');
        getline(ss, rateStr, ',');

        from = stripQuotes(from);
        to = stripQuotes(to);
        rateStr = stripQuotes(rateStr);
        if (rateStr.empty()) continue;

        double rate = stod(rateStr);

        for (const string& curr : {from, to}) {
            if (currencyIndex.find(curr) == currencyIndex.end()) {
                currencyIndex[curr] = currencies.size();
                currencies.push_back(curr);
            }
        }

        int u = currencyIndex[from];
        int v = currencyIndex[to];
        edges.push_back({u, v, -log(rate)});
    }
    rateFile.close();

    int n = currencies.size();
    vector<double> dist(n, numeric_limits<double>::infinity());
    vector<int> parent(n, -1);
    bool arbitrage = false;

    for (int source = 0; source < n; ++source) {
        dist.assign(n, numeric_limits<double>::infinity());
        parent.assign(n, -1);
        dist[source] = 0.0;

        for (int i = 0; i < n - 1; ++i) {
            for (const Edge& e : edges) {
                if (dist[e.from] + e.weight < dist[e.to]) {
                    dist[e.to] = dist[e.from] + e.weight;
                    parent[e.to] = e.from;
                }
            }
        }

        for (const Edge& e : edges) {
            if (dist[e.from] + e.weight < dist[e.to]) {
                arbitrage = true;
                cout << "Arbitrage opportunity detected!\n";

                vector<int> cycle;
                int x = e.to;
                for (int i = 0; i < n; ++i) x = parent[x];

                int cycleStart = x;
                cycle.push_back(cycleStart);
                x = parent[cycleStart];
                while (x != cycleStart && x != -1) {
                    cycle.push_back(x);
                    x = parent[x];
                }
                if (x == -1) {
                    cout << "Cycle reconstruction failed.\n";
                    return;
                }
                cycle.push_back(cycleStart);
                reverse(cycle.begin(), cycle.end());

                if (cycle.size() <= 2) {
                    cout << "Cycle involves only a single currency: " << currencies[cycleStart] << endl;
                    cout << "No other valid arbitrage cycle found." << endl;
                } else {
                    cout << "Cycle: ";
                    for (int node : cycle)
                        cout << currencies[node] << " ";
                    cout << endl;
                }
                return;
            }
        }
    }

    if (!arbitrage) {
        cout << "No arbitrage opportunity found.\n";
    }
}
// struct DayPrice {
//     string date;
//     double averagePrice;
// };

void analyzeTrend(const vector<DayPrice>& prices, int index, int& inc, int& dec, int& same) {
    if (index == prices.size() - 1) return;

    if (prices[index].averagePrice < prices[index + 1].averagePrice)
        inc++;
    else if (prices[index].averagePrice > prices[index + 1].averagePrice)
        dec++;
    else
        same++;

    analyzeTrend(prices, index + 1, inc, dec, same);
}

void trendAnalysis() {
    string filename = "prices_30days_minmax.csv"; // your file name
    string coin;
    cout << "Enter coin name: ";
    cin >> coin;

    ifstream file(filename);
    if (!file) {
        cerr << "Failed to open file." << endl;
        return ;
    }

    string line;
    vector<DayPrice> coinPrices;

    getline(file, line); // skip header
    while (getline(file, line)) {
        stringstream ss(line);
        string date, name, minStr, maxStr;
        getline(ss, date, ',');
        getline(ss, name, ',');
        getline(ss, minStr, ',');
        getline(ss, maxStr, ',');

        if (name == coin) {
            double minP = stod(minStr);
            double maxP = stod(maxStr);
            coinPrices.push_back({date, (minP + maxP) / 2});
        }
    }

    file.close();

    if (coinPrices.size() < 2) {
        cout << "Not enough data to analyze trend.\n";
        return ;
    }

    int inc = 0, dec = 0, same = 0;
    analyzeTrend(coinPrices, 0, inc, dec, same);

    cout << "\n--- Price Trend Analysis for " << coin << " ---\n";
    cout << "Days Up     : " << inc << "\n";
    cout << "Days Down   : " << dec << "\n";
    cout << "Days Stable : " << same << "\n";

    if (inc > dec && inc > same)
        cout << "Overall Trend: Mostly Increasing\n";
    else if (dec > inc && dec > same)
        cout << "Overall Trend: Mostly Decreasing\n";
    else
        cout << "Overall Trend: Highly Fluctuating / Stable\n";

    return ;
}


int mainMenu(Credentials user)
{
    int choice;

    system("cls");
    system("color 4F");

    while (true)
    {

        cout << "\n";
        cout << "\t\t**************************************\n";
        cout << "\t\t***                                ***\n";
        cout << "\t\t***            MAIN MENU           ***\n";
        cout << "\t\t***                                ***\n";
        cout << "\t\t**************************************\n";

        // cout << "\nEnter your choice : ";
        cout << "\n\t\t 1. View Prices. \n";
        cout << "\n\t\t 2. Enter Current Holdings. \n";
        cout << "\n\t\t 3. Portfolio Menu. \n";
        cout << "\n\t\t 4. Profitability Analysis. \n";
        cout << "\n\t\t 5. Find Arbitrage. \n";
        cout << "\n\t\t 6. Trend Analysis. \n";
        cout << "\n\t\t 7. LogOut. \n";
        cout << "\n\t\t 8. Exit ";
        cout << "\n";
        cout << "\n Enter your choice: \n --> ";
        cin >> choice;
        cout << endl;
        CryptoApp app;
        // Use the passed-in choice only once; get new input afterward
        if (choice < 1 || choice > 8)
        {
            choice = getValidInput("Enter choice: ", 1, 8);
        }

        switch (choice)
        {
        case 1:
            app.showPrices();
            break;
        case 2:
            app.storeCurrentAssets(user.loginID);
            break;
        case 3:
            app.portfolioMenu(user.loginID);
            break;
        case 4:
            calculateMaxProfitForCoin();
            break;
        case 5:
            findArbitrage();
            break;
        case 6:
            trendAnalysis();
            break;
        case 7:
            homePage();
            break;
        case 8:
            escape();
            mainMenu(user);
        }

        // Reset choice to an invalid number to prompt for input next loop
        choice = -1;
    }
}

// Function prototypes
bool validateDOB(int dob);
bool validateName(const string &name);
bool loginIDExists(long loginID);
void writeCredentialsToFile(long loginID, long password);
bool readCredentialsFromFile(long loginID, long password);

void login()
{
    // sleep(1);
    system("cls"); // Note: system("cls") works on Windows only
    system("color E1");

    Credentials user;

    cout << "**************************************\n";
    cout << "*                 Login              *\n";
    cout << "**************************************\n";

    cout << "Enter your Login ID: ";
    cin >> user.loginID;
    cout << "Enter your Password: ";
    cin >> user.password;

    if (readCredentialsFromFile(user.loginID, user.password))
    {
        cout << "**************************************\n";
        cout << "*          Login Successful          *\n";
        cout << "**************************************\n";
        cout << "\nMAIN MENU Loading.....\n";

        sleep(1);
        mainMenu(user);
    }
    else
    {
        cout << "Login failed. Please check your credentials or create a new account.\n";

        while (true)
        {
            cout << "Options:\n1. Create a new account\n2. Return to Home page\n3. Retry\n4. Exit\n";
            cout << "Enter your choice: ";
            int choice;
            cin >> choice;

            switch (choice)
            {
            case 1:
                createAccount();
                return;
            case 2:
                homePage();
                return;
            case 3:
                login();
                return;
            default:
                login();
            }
        }
    }
}

bool readCredentialsFromFile(long loginID, long password)
{
    ifstream file("Credentials.csv");
    long storedID, storedPass;
    char comma;

    while (file >> storedID >> comma >> storedPass)
    {
        if (storedID == loginID && storedPass == password)
            return true;
    }
    return false;
}

void createUserDirectory(long loginID)
{
    string baseDir = "users";
    string userDir = baseDir + "/" + to_string(loginID);

    // Create user-specific directory
    if (MKDIR(userDir.c_str()) == 0)
    {
        // cout << "✅ Directory created for user: " << userDir << endl;
    }
    else
    {
        cout << "Could not create directory (it may already exist): " << userDir << endl;
    }
}

void createAccount()
{
    string name;
    int dob = 0;
    Credentials newUser;

    sleep(1);
    system("cls");
    system("color 2f");

    cout << "**************************************\n";
    cout << "*         CREATE AN ACCOUNT          *\n";
    cout << "**************************************\n";
    cout << "To Create a New Account\n";

retryName:
    cout << "Enter your first name: ";
    cin >> name;

    if (validateName(name))
    {
        newUser.name = name;
        cout << "----------Name Validated Successfully----------- \n";
    }
    else
    {
        cout << "\nInvalid Name. Please use only letters and spaces. Retry.....\n";
        goto retryName;
    }

retryDOB:
    cout << "Enter your date of birth (format DDMMYY): ";
    cin >> dob;

    if (!validateDOB(dob))
    {
        cout << "\nWrong Date of Birth... Retry.....\n";
        goto retryDOB;
    }
    else
    {
        cout << "-----------Date Validated Successfully------------ \n";
    }

    long newLoginID;
    while (true)
    {
        cout << "Enter a new Login ID: ";
        cin >> newLoginID;

        if (loginIDExists(newLoginID))
        {
            cout << "Login ID already exists. Please choose a different one.\n";
        }
        else
        {
            newUser.loginID = newLoginID;
            break;
        }
    }

    cout << "Enter a new Password: ";
    cin >> newUser.password;

    newUser.verified = true;

    writeCredentialsToFile(newUser.loginID, newUser.password);
    createUserDirectory(newLoginID);
    cout << "\nAccount created successfully!\n";
    cout << "Now you will be guided to the login page.\nEnter your credentials there to LOGIN\n";

    sleep(2);
    login();
}

bool validateDOB(int dob)
{
    int day = dob / 10000;
    int month = (dob / 100) % 100;
    int year = dob % 100;

    if (year < 0 || year > 99 || month < 1 || month > 12 || day < 1 || day > 31)
        return false;

    if ((month == 2 && day > 29) ||
        ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30))
        return false;

    return true;
}

bool validateName(const string &name)
{
    if (name.empty())
        return false;

    for (char ch : name)
    {
        if (!isalpha(ch) && ch != ' ')
            return false;
    }
    return true;
}

bool loginIDExists(long loginID)
{
    ifstream file("Program Data/Credentials.txt");
    if (!file.is_open())
        return false;

    long storedLoginID, storedPassword;
    char comma;

    while (file >> storedLoginID >> comma >> storedPassword)
    {
        if (loginID == storedLoginID)
        {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void writeCredentialsToFile(long loginID, long password)
{
    // Open file for appending
    ofstream file("Credentials.csv", ios::app);
    if (!file.is_open())
    {
        cout << "Error opening the file for writing." << endl;
        return;
    }

    // Write login credentials in the format "loginID,password"
    file << loginID << "," << password << "\n";

    file.close(); // Automatically flushes and closes the file
}

void escape()
{
    cout << "\nAre you sure you want to exit? (YES/NO)--- ";
    char ch;
    cin.ignore(); // Clear input buffer
    cin.get(ch);

    if (ch == 'Y' || ch == 'y')
    {
        system("cls"); // Windows-specific
        system("color 3E");

        cout << "\n\t\t\t\t\t\t  ________________________________________________________________________";
        cout << "\n\t\t\t\t\t\t||                                                                        ||";
        cout << "\n\t\t\t\t\t\t|| ---------- |     |    /\\    |\\    | |  /      \\     / |=====| |    |   ||";
        cout << "\n\t\t\t\t\t\t||     ||     |     |   /  \\   | \\   | | /        \\   /  |     | |    |   ||";
        cout << "\n\t\t\t\t\t\t||     ||     |=====|  /====\\  |  \\  | |/          \\=/   |     | |    |   ||";
        cout << "\n\t\t\t\t\t\t||     ||     |     | /      \\ |   \\ | |\\           |    |     | |    |   ||";
        cout << "\n\t\t\t\t\t\t||     ||     |     |/        \\|    \\| | \\          |    |=====| |====|   ||";
        cout << "\n\t\t\t\t\t\t||________________________________________________________________________||\n";

        sleep(2);
        system("cls");
        exit(0);
    }
    else
    {
    }
}
void homePage()
{
    system("cls");      // Use "clear" on Linux/macOS
    system("color 17"); // Windows-specific; ignored on UNIX
    int choice;

    // Display the TaskMaster X logo and menu options
    cout << "\n  _____________________________________________________________________\n";
    cout << "||                                                                     ||\n";
    cout << "||   \\      /\\      / |===== |      |====== /=====\\ |\\    /| |=====    ||\n";
    cout << "||    \\    /  \\    /  |_____ |      |       |     | | \\  / | |_____    ||\n";
    cout << "||     \\  /    \\  /   |      |      |       |     | |  \\/  | |         ||\n";
    cout << "||      \\/      \\/    |===== |_____ |====== \\=====/ |      | |=====    ||\n";
    cout << "||                           TO Cryptoflux                             ||\n";
    cout << "||     MENU:-                                                          ||\n";
    cout << "||   1. Login                                                          ||\n";
    cout << "||   2. Create a new account                                           ||\n";
    cout << "||   3. Close CRYPTOFLUX                                               ||\n";
    cout << "||_____________________________________________________________________||\n";
    cout << "ENTER YOUR CHOICE: ";
    cin >> choice;
    cout << "\nLoading.....\n";
    sleep(1); // use Sleep(1000) if using <windows.h>

    switch (choice)
    {
    case 1:
        login();
        break;
    case 2:
        createAccount();
        break;
    case 3:
        escape();
        homePage();
        break;
    default:
        homePage();
    }
}; // Forward declaration

int main()
{
    // Call the PowerShell script to update prices.csv
    // system("powershell.exe -ExecutionPolicy Bypass -File fetch-prices-inr.ps1");
    homePage();
}