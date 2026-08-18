#ifndef BANK_H
#define BANK_H

#include <string>
#include <map>

class Customer;
class Account;
class Card;

class Bank {
private:
    std::string _name;
    std::map<std::string, Customer*> _customers;
    std::map<std::string, Account*> _accounts;
    std::map<std::string, Card*> _cards;

public:
    explicit Bank(const std::string& name);
    ~Bank();

    std::string getName() const;

    Customer* createCustomer(const std::string& name, const std::string& phone, const std::string& email);
    Account* createAccount(const std::string& customerId, const std::string& accountType,
                           const std::string& pin, double initialBalance = 0);
    Card* issueCard(const std::string& customerId, const std::string& cardPin);

    Account* getAccount(const std::string& accountNumber) const;
    Account* findAccount(const std::string& accountNumber) const;
    std::pair<Card*, Customer*> getCustomerByCard(const std::string& cardNumber) const;
    std::map<std::string, Account*> getAllAccounts() const;

    friend std::ostream& operator<<(std::ostream& os, const Bank& bank);
};

#endif
