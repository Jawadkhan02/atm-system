#include "account.h"
#include "exceptions.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

int Account::_globalCounter = 1000000;

Account::Account(const std::string& accountHolder, const std::string& pin, double initialBalance)
    : _accountHolder(accountHolder), _pin(pin), _balance(initialBalance),
      _isActive(true), _dailyWithdrawn(0), _dailyTransferred(0) {
    _globalCounter++;
    std::ostringstream oss;
    oss << "100" << _globalCounter;
    _accountNumber = oss.str();
    _lastTransactionDate = std::time(nullptr);
}

Account::~Account() {
    for (auto* txn : _transactions) {
        delete txn;
    }
    _transactions.clear();
}

void Account::resetDailyLimitsIfNeeded() {
    std::time_t now = std::time(nullptr);
    std::tm* today = std::localtime(&now);
    std::tm* last = std::localtime(&_lastTransactionDate);

    if (today->tm_yday != last->tm_yday || today->tm_year != last->tm_year) {
        _dailyWithdrawn = 0;
        _dailyTransferred = 0;
        _lastTransactionDate = now;
    }
}

std::string Account::getAccountNumber() const { return _accountNumber; }
std::string Account::getAccountHolder() const { return _accountHolder; }
double Account::getBalance() const { return _balance; }
bool Account::isActive() const { return _isActive; }

std::vector<Transaction*> Account::getTransactions() const { return _transactions; }
double Account::getDailyWithdrawn() { resetDailyLimitsIfNeeded(); return _dailyWithdrawn; }
double Account::getDailyTransferred() { resetDailyLimitsIfNeeded(); return _dailyTransferred; }

bool Account::validatePin(const std::string& pin) const { return pin == _pin; }

bool Account::changePin(const std::string& oldPin, const std::string& newPin) {
    if (oldPin != _pin) return false;
    if (newPin.length() < 4 || newPin.length() > 6) return false;
    for (char c : newPin) {
        if (!std::isdigit(c)) return false;
    }
    _pin = newPin;
    return true;
}

void Account::activate() { _isActive = true; }
void Account::deactivate() { _isActive = false; }

void Account::checkActive() const {
    if (!_isActive) {
        throw AccountInactiveError("Account " + _accountNumber + " is inactive.");
    }
}

void Account::validateAmount(double amount) const {
    if (amount <= 0) {
        throw InvalidAmountError("Amount must be positive.");
    }
}

double Account::checkBalance() const { return _balance; }

Transaction* Account::deposit(double amount) {
    checkActive();
    validateAmount(amount);
    _balance += amount;
    auto* txn = new DepositTransaction(amount, _accountNumber);
    txn->setStatus("COMPLETED");
    _transactions.push_back(txn);
    return txn;
}

Transaction* Account::withdraw(double amount) {
    checkActive();
    validateAmount(amount);
    resetDailyLimitsIfNeeded();

    if (amount < getMinWithdrawal()) {
        std::ostringstream oss;
        oss << "Minimum withdrawal amount is Rs. " << std::fixed << std::setprecision(0)
            << getMinWithdrawal() << ".";
        throw InvalidAmountError(oss.str());
    }

    if (amount > getMaxWithdrawalPerTransaction()) {
        std::ostringstream oss;
        oss << "Maximum withdrawal per transaction is Rs. " << std::fixed << std::setprecision(0)
            << getMaxWithdrawalPerTransaction() << ".";
        throw InvalidAmountError(oss.str());
    }

    double dailyLimit = calculateWithdrawalLimit();
    if (_dailyWithdrawn + amount > dailyLimit) {
        std::ostringstream oss;
        oss << "Daily withdrawal limit of Rs. " << std::fixed << std::setprecision(0) << dailyLimit
            << " exceeded. Already withdrawn: Rs. " << _dailyWithdrawn << ".";
        throw DailyLimitExceededError(oss.str());
    }

    double fee = withdrawalFee(amount);
    double total = amount + fee;

    if (_balance - total < getMinimumBalance()) {
        std::ostringstream oss;
        oss << "Insufficient balance. Minimum balance: Rs. " << std::fixed << std::setprecision(0)
            << getMinimumBalance() << ". With fee Rs. " << fee << ", total needed: Rs. " << total << ".";
        throw MinimumBalanceError(oss.str());
    }

    _balance -= total;
    _dailyWithdrawn += amount;
    auto* txn = new WithdrawalTransaction(amount, _accountNumber);
    txn->setStatus("COMPLETED");
    _transactions.push_back(txn);
    return txn;
}

std::pair<Transaction*, Transaction*> Account::transfer(double amount, Account* targetAccount) {
    checkActive();
    targetAccount->checkActive();
    validateAmount(amount);

    if (_accountNumber == targetAccount->getAccountNumber()) {
        throw SameAccountTransferError("Cannot transfer to the same account.");
    }

    resetDailyLimitsIfNeeded();

    double dailyLimit = calculateTransferLimit();
    if (_dailyTransferred + amount > dailyLimit) {
        std::ostringstream oss;
        oss << "Daily transfer limit of Rs. " << std::fixed << std::setprecision(0) << dailyLimit
            << " exceeded. Already transferred: Rs. " << _dailyTransferred << ".";
        throw DailyLimitExceededError(oss.str());
    }

    double fee = transferFee(amount);
    double total = amount + fee;

    if (_balance - total < getMinimumBalance()) {
        std::ostringstream oss;
        oss << "Insufficient balance for transfer. Minimum balance: Rs. " << std::fixed
            << std::setprecision(0) << getMinimumBalance() << ". With fee Rs. " << fee
            << ", total needed: Rs. " << total << ".";
        throw MinimumBalanceError(oss.str());
    }

    _balance -= total;
    targetAccount->_balance += amount;
    _dailyTransferred += amount;

    auto* senderTxn = new TransferTransaction(amount, _accountNumber,
                                              targetAccount->getAccountNumber(), true);
    senderTxn->setStatus("COMPLETED");
    _transactions.push_back(senderTxn);

    auto* receiverTxn = new TransferTransaction(amount, targetAccount->getAccountNumber(),
                                                _accountNumber, false);
    receiverTxn->setStatus("COMPLETED");
    targetAccount->_transactions.push_back(receiverTxn);

    return {senderTxn, receiverTxn};
}

std::vector<Transaction*> Account::getMiniStatement(int count) const {
    int n = std::min(count, static_cast<int>(_transactions.size()));
    if (n <= 0) return {};
    return std::vector<Transaction*>(_transactions.end() - n, _transactions.end());
}

std::ostream& operator<<(std::ostream& os, const Account& acc) {
    os << acc.accountType() << " | Acc: " << acc._accountNumber
       << " | Holder: " << acc._accountHolder
       << " | Balance: Rs. " << std::fixed << std::setprecision(0) << acc._balance
       << " | Status: " << (acc._isActive ? "ACTIVE" : "INACTIVE");
    return os;
}

// ==================== SavingsAccount ====================

SavingsAccount::SavingsAccount(const std::string& accountHolder, const std::string& pin, double initialBalance)
    : Account(accountHolder, pin, initialBalance) {}

std::string SavingsAccount::accountType() const { return "SAVINGS"; }
double SavingsAccount::getMinimumBalance() const { return MIN_BALANCE; }
double SavingsAccount::getMaxWithdrawalPerTransaction() const { return MAX_WITHDRAWAL; }
double SavingsAccount::getMinWithdrawal() const { return MIN_WITHDRAWAL; }
double SavingsAccount::calculateWithdrawalLimit() { return DAILY_WITHDRAWAL_LIMIT; }
double SavingsAccount::calculateTransferLimit() { return DAILY_TRANSFER_LIMIT; }

double SavingsAccount::withdrawalFee(double amount) const {
    return std::max(amount * WITHDRAWAL_FEE_RATE, 0.0);
}

double SavingsAccount::transferFee(double amount) const {
    return std::max(amount * TRANSFER_FEE_RATE, 0.0);
}

// ==================== CurrentAccount ====================

CurrentAccount::CurrentAccount(const std::string& accountHolder, const std::string& pin, double initialBalance)
    : Account(accountHolder, pin, initialBalance) {}

std::string CurrentAccount::accountType() const { return "CURRENT"; }
double CurrentAccount::getMinimumBalance() const { return MIN_BALANCE; }
double CurrentAccount::getMaxWithdrawalPerTransaction() const { return MAX_WITHDRAWAL; }
double CurrentAccount::getMinWithdrawal() const { return MIN_WITHDRAWAL; }
double CurrentAccount::calculateWithdrawalLimit() { return DAILY_WITHDRAWAL_LIMIT; }
double CurrentAccount::calculateTransferLimit() { return DAILY_TRANSFER_LIMIT; }
double CurrentAccount::withdrawalFee(double amount) const { return WITHDRAWAL_FEE_FLAT; }
double CurrentAccount::transferFee(double amount) const { return TRANSFER_FEE_FLAT; }

Transaction* CurrentAccount::withdraw(double amount) {
    checkActive();
    validateAmount(amount);
    resetDailyLimitsIfNeeded();

    if (amount < getMinWithdrawal()) {
        std::ostringstream oss;
        oss << "Minimum withdrawal amount is Rs. " << std::fixed << std::setprecision(0)
            << getMinWithdrawal() << ".";
        throw InvalidAmountError(oss.str());
    }

    if (amount > getMaxWithdrawalPerTransaction()) {
        std::ostringstream oss;
        oss << "Maximum withdrawal per transaction is Rs. " << std::fixed << std::setprecision(0)
            << getMaxWithdrawalPerTransaction() << ".";
        throw InvalidAmountError(oss.str());
    }

    double dailyLimit = calculateWithdrawalLimit();
    if (_dailyWithdrawn + amount > dailyLimit) {
        std::ostringstream oss;
        oss << "Daily withdrawal limit of Rs. " << std::fixed << std::setprecision(0) << dailyLimit
            << " exceeded. Already withdrawn: Rs. " << _dailyWithdrawn << ".";
        throw DailyLimitExceededError(oss.str());
    }

    double fee = withdrawalFee(amount);
    double total = amount + fee;

    if (_balance - total < -OVERDRAFT_LIMIT) {
        std::ostringstream oss;
        oss << "Insufficient balance. Overdraft limit: Rs. " << std::fixed << std::setprecision(0)
            << OVERDRAFT_LIMIT << ". Available (with overdraft): Rs. " << _balance + OVERDRAFT_LIMIT << ".";
        throw InsufficientBalanceError(oss.str());
    }

    _balance -= total;
    _dailyWithdrawn += amount;
    auto* txn = new WithdrawalTransaction(amount, _accountNumber);
    txn->setStatus("COMPLETED");
    _transactions.push_back(txn);
    return txn;
}

std::pair<Transaction*, Transaction*> CurrentAccount::transfer(double amount, Account* targetAccount) {
    checkActive();
    targetAccount->checkActive();
    validateAmount(amount);

    if (_accountNumber == targetAccount->getAccountNumber()) {
        throw SameAccountTransferError("Cannot transfer to the same account.");
    }

    resetDailyLimitsIfNeeded();

    double dailyLimit = calculateTransferLimit();
    if (_dailyTransferred + amount > dailyLimit) {
        std::ostringstream oss;
        oss << "Daily transfer limit of Rs. " << std::fixed << std::setprecision(0) << dailyLimit
            << " exceeded. Already transferred: Rs. " << _dailyTransferred << ".";
        throw DailyLimitExceededError(oss.str());
    }

    double fee = transferFee(amount);
    double total = amount + fee;

    if (_balance - total < -OVERDRAFT_LIMIT) {
        std::ostringstream oss;
        oss << "Insufficient balance for transfer. Overdraft limit: Rs. " << std::fixed
            << std::setprecision(0) << OVERDRAFT_LIMIT << ". Available (with overdraft): Rs. "
            << _balance + OVERDRAFT_LIMIT << ".";
        throw InsufficientBalanceError(oss.str());
    }

    _balance -= total;
    targetAccount->_balance += amount;
    _dailyTransferred += amount;

    auto* senderTxn = new TransferTransaction(amount, _accountNumber,
                                              targetAccount->getAccountNumber(), true);
    senderTxn->setStatus("COMPLETED");
    _transactions.push_back(senderTxn);

    auto* receiverTxn = new TransferTransaction(amount, targetAccount->getAccountNumber(),
                                                _accountNumber, false);
    receiverTxn->setStatus("COMPLETED");
    targetAccount->_transactions.push_back(receiverTxn);

    return {senderTxn, receiverTxn};
}
