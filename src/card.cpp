#include "card.h"
#include <iostream>
#include <sstream>

int Card::_globalCounter = 10000000;

Card::Card(const std::string& pin, const std::string& customerId,
           const std::vector<std::string>& accountNumbers)
    : _pin(pin), _customerId(customerId), _accountNumbers(accountNumbers),
      _isActive(true), _failedPinAttempts(0), _maxPinAttempts(3) {
    _globalCounter++;
    std::ostringstream oss;
    oss << "4" << _globalCounter;
    _cardNumber = oss.str();
}

std::string Card::getCardNumber() const { return _cardNumber; }
std::string Card::getCustomerId() const { return _customerId; }
std::vector<std::string> Card::getAccountNumbers() const { return _accountNumbers; }
bool Card::isActive() const { return _isActive; }

bool Card::validatePin(const std::string& pin) {
    if (!_isActive) return false;
    if (pin == _pin) {
        _failedPinAttempts = 0;
        return true;
    }
    _failedPinAttempts++;
    if (_failedPinAttempts >= _maxPinAttempts) {
        _isActive = false;
    }
    return false;
}

bool Card::changePin(const std::string& oldPin, const std::string& newPin) {
    if (oldPin != _pin) return false;
    if (newPin.length() < 4 || newPin.length() > 6) return false;
    for (char c : newPin) {
        if (!std::isdigit(c)) return false;
    }
    _pin = newPin;
    return true;
}

void Card::block() { _isActive = false; }
void Card::unblock() { _isActive = true; _failedPinAttempts = 0; }

void Card::addAccount(const std::string& accountNumber) {
    for (const auto& acc : _accountNumbers) {
        if (acc == accountNumber) return;
    }
    _accountNumbers.push_back(accountNumber);
}

std::ostream& operator<<(std::ostream& os, const Card& card) {
    std::string masked = "****-****-****-" + card._cardNumber.substr(card._cardNumber.length() - 4);
    os << "Card: " << masked << " | Status: " << (card._isActive ? "ACTIVE" : "BLOCKED");
    return os;
}
