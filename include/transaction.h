#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>

class Transaction {
protected:
    static int _globalCounter;
    std::string _transactionId;
    double _amount;
    std::string _accountNumber;
    std::time_t _dateTime;
    std::string _status;

public:
    Transaction(double amount, const std::string& accountNumber);
    virtual ~Transaction() = default;

    std::string getTransactionId() const;
    double getAmount() const;
    std::string getAccountNumber() const;
    std::time_t getDateTime() const;
    std::string getStatus() const;
    void setStatus(const std::string& status);

    std::string getFormattedDate() const;

    virtual std::string transactionType() const = 0;
    virtual std::string displayAmount() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Transaction& txn);
};

class DepositTransaction : public Transaction {
public:
    DepositTransaction(double amount, const std::string& accountNumber);
    std::string transactionType() const override;
    std::string displayAmount() const override;
};

class WithdrawalTransaction : public Transaction {
public:
    WithdrawalTransaction(double amount, const std::string& accountNumber);
    std::string transactionType() const override;
    std::string displayAmount() const override;
};

class TransferTransaction : public Transaction {
private:
    std::string _targetAccountNumber;
    bool _isSender;

public:
    TransferTransaction(double amount, const std::string& accountNumber,
                        const std::string& targetAccountNumber, bool isSender = true);
    std::string getTargetAccountNumber() const;
    std::string transactionType() const override;
    std::string displayAmount() const override;
};

#endif
