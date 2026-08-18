#include "bank.h"
#include "customer.h"
#include "account.h"
#include "card.h"
#include "exceptions.h"
#include <iostream>

Bank::Bank(const std::string& name) : _name(name) {}

Bank::~Bank() {
    for (auto& pair : _accounts) delete pair.second;
    for (auto& pair : _customers) delete pair.second;
    for (auto& pair : _cards) delete pair.second;
}

std::string Bank::getName() const { return _name; }

Customer* Bank::createCustomer(const std::string& name, const std::string& phone, const std::string& email) {
    auto* customer = new Customer(name, phone, email);
    _customers[customer->getCustomerId()] = customer;
    return customer;
}

Account* Bank::createAccount(const std::string& customerId, const std::string& accountType,
                             const std::string& pin, double initialBalance) {
    auto it = _customers.find(customerId);
    if (it == _customers.end()) {
        throw InvalidAccountError("Customer " + customerId + " not found.");
    }

    Account* account = nullptr;
    if (accountType == "SAVINGS" || accountType == "savings") {
        account = new SavingsAccount(it->second->getName(), pin, initialBalance);
    } else if (accountType == "CURRENT" || accountType == "current") {
        account = new CurrentAccount(it->second->getName(), pin, initialBalance);
    } else {
        throw InvalidAccountError("Invalid account type: " + accountType);
    }

    it->second->addAccount(account);
    _accounts[account->getAccountNumber()] = account;
    return account;
}

Card* Bank::issueCard(const std::string& customerId, const std::string& cardPin) {
    auto it = _customers.find(customerId);
    if (it == _customers.end()) {
        throw InvalidAccountError("Customer " + customerId + " not found.");
    }

    std::vector<std::string> accNums = it->second->getAccountNumbers();
    auto* card = new Card(cardPin, customerId, accNums);
    it->second->addCard(card);
    _cards[card->getCardNumber()] = card;
    return card;
}

Account* Bank::getAccount(const std::string& accountNumber) const {
    auto it = _accounts.find(accountNumber);
    if (it == _accounts.end()) {
        throw InvalidAccountError("Account " + accountNumber + " not found.");
    }
    return it->second;
}

Account* Bank::findAccount(const std::string& accountNumber) const {
    auto it = _accounts.find(accountNumber);
    if (it != _accounts.end()) return it->second;
    return nullptr;
}

std::pair<Card*, Customer*> Bank::getCustomerByCard(const std::string& cardNumber) const {
    auto it = _cards.find(cardNumber);
    if (it == _cards.end()) return {nullptr, nullptr};
    auto cit = _customers.find(it->second->getCustomerId());
    if (cit == _customers.end()) return {it->second, nullptr};
    return {it->second, cit->second};
}

std::map<std::string, Account*> Bank::getAllAccounts() const { return _accounts; }

std::ostream& operator<<(std::ostream& os, const Bank& bank) {
    os << "Bank: " << bank._name
       << " | Customers: " << bank._customers.size()
       << " | Accounts: " << bank._accounts.size();
    return os;
}
