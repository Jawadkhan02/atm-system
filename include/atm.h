#ifndef ATM_H
#define ATM_H

#include <string>
#include <map>
#include <vector>

class Bank;
class Card;
class Customer;
class Account;

class ATM {
private:
    Bank& _bank;
    std::string _location;
    std::map<int, int> _notes;
    Card* _currentCard;
    Customer* _currentCustomer;
    Account* _selectedAccount;

    static const std::vector<int> SUPPORTED_DENOMINATIONS;

    bool canDispense(double amount);
    std::map<int, int> dispenseCash(double amount);
    void printDenominations(const std::map<int, int>& dispensed);
    void displayMenu();
    bool selectAccount();
    void checkBalance();
    void deposit();
    void withdraw();
    void transfer();
    void changePin();
    void miniStatement();

public:
    ATM(Bank& bank, const std::string& location,
        int notes500 = 20, int notes1000 = 30, int notes5000 = 10);

    std::string getLocation() const;
    double totalCash() const;

    bool insertCard();
    void run();
    void ejectCard();
};

#endif
