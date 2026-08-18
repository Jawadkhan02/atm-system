#ifndef CARD_H
#define CARD_H

#include <string>
#include <vector>
#include <sstream>

class Card {
private:
    static int _globalCounter;
    std::string _cardNumber;
    std::string _pin;
    std::string _customerId;
    std::vector<std::string> _accountNumbers;
    bool _isActive;
    int _failedPinAttempts;
    const int _maxPinAttempts;

public:
    Card(const std::string& pin, const std::string& customerId,
         const std::vector<std::string>& accountNumbers);

    std::string getCardNumber() const;
    std::string getCustomerId() const;
    std::vector<std::string> getAccountNumbers() const;
    bool isActive() const;

    bool validatePin(const std::string& pin);
    bool changePin(const std::string& oldPin, const std::string& newPin);
    void block();
    void unblock();
    void addAccount(const std::string& accountNumber);

    friend std::ostream& operator<<(std::ostream& os, const Card& card);
};

#endif
