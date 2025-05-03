#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <string>

using namespace std;

struct Item {
    string name;
    double btcValue;  // Value in Bitcoin
    double priority;  // User's priority for the item (1-10)
    
    Item(string n, double btc, double prio) : name(n), btcValue(btc), priority(prio) {}
};

bool compareByDensity(const Item &a, const Item &b) {
    double ratioA = a.priority / a.btcValue;
    double ratioB = b.priority / b.btcValue;
    return ratioA > ratioB;
}

vector<pair<string, int>> unboundedKnapsack(double budget, const vector<Item> &items) {
    vector<double> dp(budget * 1e8 + 1, 0);  // Using satoshis for precision
    vector<vector<pair<string, int>>> itemsUsed(budget * 1e8 + 1);
    
    int budgetSatoshi = static_cast<int>(budget * 1e8);
    
    for (int b = 1; b <= budgetSatoshi; b++) {
        for (const Item &item : items) {
            int itemSatoshi = static_cast<int>(item.btcValue * 1e8);
            if (itemSatoshi <= b) {
                if (dp[b - itemSatoshi] + item.priority > dp[b]) {
                    dp[b] = dp[b - itemSatoshi] + item.priority;
                    itemsUsed[b] = itemsUsed[b - itemSatoshi];
                    
                    bool found = false;
                    for (auto &entry : itemsUsed[b]) {
                        if (entry.first == item.name) {
                            entry.second++;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        itemsUsed[b].emplace_back(item.name, 1);
                    }
                }
            }
        }
    }
    
    return itemsUsed[budgetSatoshi];
}

vector<pair<string, double>> fractionalKnapsack(double budget, vector<Item> items) {
    sort(items.begin(), items.end(), compareByDensity);
    
    vector<pair<string, double>> result;
    double remaining = budget;
    
    for (const Item &item : items) {
        if (remaining <= 0) break;
        
        if (item.btcValue <= remaining) {
            result.emplace_back(item.name, 1.0);
            remaining -= item.btcValue;
        } else {
            double fraction = remaining / item.btcValue;
            result.emplace_back(item.name, fraction);
            remaining = 0;
        }
    }
    
    return result;
}

void displayResults(double budget, 
                   const vector<pair<string, int>> &unboundedResult,
                   const vector<pair<string, double>> &fractionalResult) {
    cout << "\n\n=== CryptoFlux Budget Allocation Advisor ===\n";
    cout << "For your budget of " << fixed << setprecision(8) << budget << " BTC:\n\n";
    
    cout << "Unbounded Knapsack Approach (buy multiple whole items):\n";
    if (unboundedResult.empty()) {
        cout << "  Cannot afford any items with this budget.\n";
    } else {
        double totalSpent = 0.0;
        for (const auto &entry : unboundedResult) {
            for (const Item &item : items) {
                if (item.name == entry.first) {
                    totalSpent += item.btcValue * entry.second;
                    cout << "  " << entry.second << " x " << entry.first 
                         << " (" << fixed << setprecision(8) << item.btcValue * entry.second << " BTC)\n";
                    break;
                }
            }
        }
        cout << "  Total spent: " << fixed << setprecision(8) << totalSpent << " BTC\n";
    }
    
    cout << "\nFractional Knapsack Approach (buy fractions of items):\n";
    if (fractionalResult.empty()) {
        cout << "  Cannot afford any items with this budget.\n";
    } else {
        double totalSpent = 0.0;
        for (const auto &entry : fractionalResult) {
            for (const Item &item : items) {
                if (item.name == entry.first) {
                    double cost = item.btcValue * entry.second;
                    totalSpent += cost;
                    cout << "  " << fixed << setprecision(2) << entry.second * 100 << "% of " << entry.first
                         << " (" << fixed << setprecision(8) << cost << " BTC)\n";
                    break;
                }
            }
        }
        cout << "  Total spent: " << fixed << setprecision(8) << totalSpent << " BTC\n";
    }
}

void budgetAllocator() {
    cout << "\n=== CryptoFlux Monthly Budget Allocator ===\n";
    cout << "This tool helps you determine what you can buy with your Bitcoin budget.\n\n";
    
    double budget;
    cout << "Enter your monthly Bitcoin budget (in BTC): ";
    cin >> budget;
    
    vector<Item> items;
    cout << "\nEnter items you're considering to purchase (enter 'done' when finished):\n";
    
    while (true) {
        string name;
        cout << "Item name (or 'done'): ";
        cin.ignore();
        getline(cin, name);
        
        if (name == "done") break;
        
        double btcValue;
        cout << "Price in BTC: ";
        cin >> btcValue;
        
        double priority;
        cout << "Your priority for this item (1-10, 10 being highest): ";
        cin >> priority;
        
        items.emplace_back(name, btcValue, priority);
    }
    
    if (items.empty()) {
        cout << "No items entered. Exiting budget allocator.\n";
        return;
    }
    
    auto unboundedResult = unboundedKnapsack(budget, items);
    auto fractionalResult = fractionalKnapsack(budget, items);
    
    displayResults(budget, unboundedResult, fractionalResult);
    
    cout << "\nRecommendation:\n";
    if (!unboundedResult.empty() && !fractionalResult.empty()) {
        cout << "The Unbounded approach lets you buy whole items, while the Fractional approach\n";
        cout << "may provide better value by allowing partial purchases of high-priority items.\n";
    }
    cout << "Current Bitcoin price considerations are based on your local valuation.\n";
}

int main() {
    cout << "Welcome to CryptoFlux Terminal\n";
    budgetAllocator();
    return 0;
}
