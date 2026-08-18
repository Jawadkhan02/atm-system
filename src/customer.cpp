#include "customer.h"
#include "account.h"
#include "card.h"
#include <iostream>
#include <sstream>

int Customer::_globalCounter = 1000;

Customer::Customer(const std::string& name, const std::string& phone, const std::string& email)
    : _name(name), _phone(phone), _email(email) {
    _globalCounter++;
    std::ostringstream oss;
    oss << "CUST-" << _globalCounter;
    _customerId = oss.str();
}

std::string Customer::getCustomerId() const { return _customerId; }
std::string Customer::getName() const { return _name; }
std::string Customer::getPhone() const { return _phone; }
std::string Customer::getEmail() const { return _email; }
std::map<std::string, Account*> Customer::getAccounts() const { return _accounts; }
std::vector<Card*> Customer::getCards() const { return _cards; }

void Customer::addAccount(Account* account) {
    _accounts[account->getAccountNumber()] = account;
}

Account* Customer::getAccount(const std::string& accountNumber) const {
    auto it = _accounts.find(accountNumber);
    if (it != _accounts.end()) return it->second;
    return nullptr;
}

std::vector<std::string> Customer::getAccountNumbers() const {
    std::vector<std::string> nums;
    for (auto& pair : _accounts) {
        nums.push_back(pair.first);
    }
    return nums;
}

void Customer::addCard(Card* card) {
    _cards.push_back(card);
}

std::ostream& operator<<(std::ostream& os, const Customer& cust) {
    std::ostringstream accStream;
    bool first = true;
    for (auto& pair : cust._accounts) {
        if (!first) accStream << ", ";
        accStream << pair.first;
        first = false;
    }
    os << "Customer: " << cust._name << " (" << cust._customerId << ")"
       << " | Phone: " << cust._phone
       << " | Email: " << cust._email
       << " | Accounts: [" << accStream.str() << "]";
    return os;
}
