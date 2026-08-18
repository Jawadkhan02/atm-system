#include "transaction.h"
#include <sstream>
#include <iomanip>

int Transaction::_globalCounter = 1000;

Transaction::Transaction(double amount, const std::string& accountNumber)
    : _amount(amount), _accountNumber(accountNumber), _status("PENDING") {
    _globalCounter++;
    std::ostringstream oss;
    oss << "TXN-" << _globalCounter;
    _transactionId = oss.str();
    _dateTime = std::time(nullptr);
}

std::string Transaction::getTransactionId() const { return _transactionId; }
double Transaction::getAmount() const { return _amount; }
std::string Transaction::getAccountNumber() const { return _accountNumber; }
std::time_t Transaction::getDateTime() const { return _dateTime; }
std::string Transaction::getStatus() const { return _status; }
void Transaction::setStatus(const std::string& status) { _status = status; }

std::string Transaction::getFormattedDate() const {
    char buf[16];
    std::tm* tm_info = std::localtime(&_dateTime);
    std::strftime(buf, sizeof(buf), "%d-%b", tm_info);
    return std::string(buf);
}

std::ostream& operator<<(std::ostream& os, const Transaction& txn) {
    os << std::left << std::setw(14) << txn._transactionId
       << std::setw(10) << txn.getFormattedDate()
       << std::setw(12) << txn.transactionType()
       << std::right << std::setw(12) << txn.displayAmount();
    return os;
}

// DepositTransaction
DepositTransaction::DepositTransaction(double amount, const std::string& accountNumber)
    : Transaction(amount, accountNumber) {}

std::string DepositTransaction::transactionType() const { return "DEPOSIT"; }

std::string DepositTransaction::displayAmount() const {
    std::ostringstream oss;
    oss << "+" << std::fixed << std::setprecision(0) << _amount;
    return oss.str();
}

// WithdrawalTransaction
WithdrawalTransaction::WithdrawalTransaction(double amount, const std::string& accountNumber)
    : Transaction(amount, accountNumber) {}

std::string WithdrawalTransaction::transactionType() const { return "WITHDRAWAL"; }

std::string WithdrawalTransaction::displayAmount() const {
    std::ostringstream oss;
    oss << "-" << std::fixed << std::setprecision(0) << _amount;
    return oss.str();
}

// TransferTransaction
TransferTransaction::TransferTransaction(double amount, const std::string& accountNumber,
                                         const std::string& targetAccountNumber, bool isSender)
    : Transaction(amount, accountNumber), _targetAccountNumber(targetAccountNumber),
      _isSender(isSender) {}

std::string TransferTransaction::getTargetAccountNumber() const { return _targetAccountNumber; }

std::string TransferTransaction::transactionType() const { return "TRANSFER"; }

std::string TransferTransaction::displayAmount() const {
    std::ostringstream oss;
    if (_isSender)
        oss << "-" << std::fixed << std::setprecision(0) << _amount;
    else
        oss << "+" << std::fixed << std::setprecision(0) << _amount;
    return oss.str();
}
