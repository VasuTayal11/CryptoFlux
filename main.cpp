#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <cmath>
using namespace std;

struct Transaction {
    string crypto;
    double amount;
    double price;
    string type;
    string timestamp;
};

string toUpper(const string& input) {
    string upper = input;
    transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    return upper;
}

int getValidInput(const string& prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (!cin.fail() && value >= min && value <= max) break;
        cin.clear(); cin.ignore(10000, '\n');
        cout << "Please enter a number between " << min << " and " << max << ".\n";
    }
    return value;
}

double getMockPrice(const string& crypto) {
    if (crypto == "bitcoin") return 42000.50;
    if (crypto == "ethereum") return 3200.25;
    if (crypto == "dogecoin") return 0.08;
    return 100.0;
}

string currentTimestamp() {
    time_t now = time(0);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", localtime(&now));
    return string(buf);
}

class CryptoTracker {
public:
    vector<string> getCryptos() {
        return {"bitcoin", "ethereum", "dogecoin"};
    }
};

class CryptoApp {
    CryptoTracker tracker;
    vector<Transaction> transactions;
    map<string, double> holdings;
    vector<string> watchlist;

    void addTransaction() {
        auto cryptos = tracker.getCryptos();
        for (int i = 0; i < cryptos.size(); i++) {
            cout << i + 1 << ". " << toUpper(cryptos[i]) << "\n";
        }
        int choice = getValidInput("Select crypto: ", 1, cryptos.size());
        string crypto = cryptos[choice - 1];

        double amount;
        cout << "Enter amount: ";
        cin >> amount;

        string type;
        cout << "Type (buy/sell): ";
        cin >> type;

        double price = getMockPrice(crypto);

        Transaction tx = {crypto, amount, price, type, currentTimestamp()};
        transactions.push_back(tx);
        if (type == "buy") holdings[crypto] += amount;
        else holdings[crypto] -= amount;

        cout << "Transaction added.\n";
    }

    void showHoldings() {
        cout << "\nYour Holdings:\n";
        for (auto& pair : holdings) {
            cout << toUpper(pair.first) << ": " << pair.second << " units\n";
        }
    }

    void showTransactions() {
        cout << "\nTransaction History:\n";
        for (auto& tx : transactions) {
            cout << toUpper(tx.crypto) << " | " << tx.type << " | Amount: " << tx.amount
                 << " @ Price: $" << tx.price << " | " << tx.timestamp << "\n";
        }
    }

    void savePortfolioToFile(const string& filename) {
        ofstream file(filename);
        for (auto& tx : transactions) {
            file << tx.crypto << "," << tx.amount << "," << tx.price << "," << tx.type << "," << tx.timestamp << "\n";
        }
        cout << "Portfolio saved to " << filename << "\n";
    }

    void loadPortfolioFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Failed to open " << filename << "\n";
            return;
        }
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string crypto, amountStr, priceStr, type, timestamp;
            if (!getline(ss, crypto, ',') ||
                !getline(ss, amountStr, ',') ||
                !getline(ss, priceStr, ',') ||
                !getline(ss, type, ',') ||
                !getline(ss, timestamp)) {
                cout << "Malformed line skipped: " << line << "\n";
                continue;
            }
            try {
                double amount = stod(amountStr);
                double price = stod(priceStr);
                transactions.push_back({crypto, amount, price, type, timestamp});
                if (type == "buy") holdings[crypto] += amount;
                else if (type == "sell") holdings[crypto] -= amount;
                else cout << "Invalid transaction type in file: " << type << "\n";
            } catch (...) {
                cout << "Error parsing line: " << line << "\n";
            }
        }
        cout << "Portfolio loaded from " << filename << "\n";
    }

    void showPortfolioValue() {
        double total = 0.0;
        for (auto& pair : holdings) {
            double price = getMockPrice(pair.first);
            total += pair.second * price;
        }
        cout << fixed << setprecision(2);
        cout << "\nTotal Portfolio Value: $" << total << "\n";
    }

public:
    void showPrices() {
        for (auto& crypto : tracker.getCryptos()) {
            cout << fixed << setprecision(2) << getMockPrice(crypto) << " " << toUpper(crypto) << "\n";
        }
    }

    void portfolioMenu() {
        while (true) {
            cout << "\n--- Portfolio Menu ---\n";
            cout << "1. Add Transaction\n2. Show Holdings\n3. Show Transactions\n4. Save Portfolio\n5. Load Portfolio\n6. Show Portfolio Value\n7. Back to Main\n";
            int choice = getValidInput("Enter choice: ", 1, 7);
            if (choice == 7) break;
            switch (choice) {
                case 1: addTransaction(); break;
                case 2: showHoldings(); break;
                case 3: showTransactions(); break;
                case 4: savePortfolioToFile("portfolio.csv"); break;
                case 5: loadPortfolioFromFile("portfolio.csv"); break;
                case 6: showPortfolioValue(); break;
            }
        }
    }

    void manageWatchlist() {
        while (true) {
            cout << "\n--- Watchlist Menu ---\n";
            cout << "1. View Watchlist\n2. Add to Watchlist\n3. Remove from Watchlist\n4. Back\n";
            int choice = getValidInput("Enter choice: ", 1, 4);
            if (choice == 4) break;
            switch (choice) {
                case 1:
                    cout << "\nYour Watchlist:\n";
                    for (auto& c : watchlist) cout << toUpper(c) << "\n";
                    break;
                case 2: {
                    string c;
                    cout << "Enter crypto to add: ";
                    cin >> c;
                    watchlist.push_back(c);
                    cout << toUpper(c) << " added to watchlist.\n";
                    break;
                }
                case 3: {
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

int main() {
    CryptoApp app;
    while (true) {
        cout << "\n--- Main Menu ---\n";
        cout << "1. Show Prices\n2. Portfolio Menu\n3. Manage Watchlist\n4. Exit\n";
        int choice = getValidInput("Enter choice: ", 1, 4);
        switch (choice) {
            case 1: app.showPrices(); break;
            case 2: app.portfolioMenu(); break;
            case 3: app.manageWatchlist(); break;
            case 4: cout << "Goodbye!\n"; return 0;
        }
    }
}