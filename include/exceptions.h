#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class ATMError : public std::runtime_error {
public:
    explicit ATMError(const std::string& msg) : std::runtime_error(msg) {}
};

class InvalidPINError : public ATMError {
public:
    explicit InvalidPINError(const std::string& msg = "Invalid PIN.") : ATMError(msg) {}
};

class CardBlockedError : public ATMError {
public:
    explicit CardBlockedError(const std::string& msg = "Card is blocked.") : ATMError(msg) {}
};

class InsufficientBalanceError : public ATMError {
public:
    explicit InsufficientBalanceError(const std::string& msg = "Insufficient balance.") : ATMError(msg) {}
};

class InsufficientATMFundsError : public ATMError {
public:
    explicit InsufficientATMFundsError(const std::string& msg = "ATM has insufficient cash.") : ATMError(msg) {}
};

class InvalidAmountError : public ATMError {
public:
    explicit InvalidAmountError(const std::string& msg = "Invalid amount.") : ATMError(msg) {}
};

class AccountInactiveError : public ATMError {
public:
    explicit AccountInactiveError(const std::string& msg = "Account is inactive.") : ATMError(msg) {}
};

class DailyLimitExceededError : public ATMError {
public:
    explicit DailyLimitExceededError(const std::string& msg = "Daily limit exceeded.") : ATMError(msg) {}
};

class InvalidAccountError : public ATMError {
public:
    explicit InvalidAccountError(const std::string& msg = "Invalid account.") : ATMError(msg) {}
};

class MaxPINAttemptsError : public ATMError {
public:
    explicit MaxPINAttemptsError(const std::string& msg = "Max PIN attempts exceeded. Card blocked.") : ATMError(msg) {}
};

class SameAccountTransferError : public ATMError {
public:
    explicit SameAccountTransferError(const std::string& msg = "Cannot transfer to the same account.") : ATMError(msg) {}
};

class MinimumBalanceError : public ATMError {
public:
    explicit MinimumBalanceError(const std::string& msg = "Minimum balance not maintained.") : ATMError(msg) {}
};

#endif
