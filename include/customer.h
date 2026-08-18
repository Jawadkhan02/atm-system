#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>
#include <map>
#include <vector>
#include <iostream>

class Account;
class Card;

class Customer {
private:
    static int _globalCounter;
    std::string _customerId;
    std::string _name;
    std::string _phone;
    std::string _email;
    std::map<std::string, Account*> _accounts;
    std::vector<Card*> _cards;

public:
    Customer(const std::string& name, const std::string& phone, const std::string& email);

    std::string getCustomerId() const;
    std::string getName() const;
    std::string getPhone() const;
    std::string getEmail() const;
    std::map<std::string, Account*> getAccounts() const;
    std::vector<Card*> getCards() const;

    void addAccount(Account* account);
    Account* getAccount(const std::string& accountNumber) const;
    std::vector<std::string> getAccountNumbers() const;
    void addCard(Card* card);

    friend std::ostream& operator<<(std::ostream& os, const Customer& cust);
};

#endif
