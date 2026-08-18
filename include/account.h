#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>
#include <ctime>
#include "transaction.h"

class Account {
protected:
    static int _globalCounter;
    std::string _accountNumber;
    std::string _accountHolder;
    std::string _pin;
    double _balance;
    bool _isActive;
    std::vector<Transaction*> _transactions;
    double _dailyWithdrawn;
    double _dailyTransferred;
    std::time_t _lastTransactionDate;

    void resetDailyLimitsIfNeeded();
    void checkActive() const;
    void validateAmount(double amount) const;

public:
    Account(const std::string& accountHolder, const std::string& pin, double initialBalance = 0);
    virtual ~Account();

    std::string getAccountNumber() const;
    std::string getAccountHolder() const;
    double getBalance() const;
    bool isActive() const;
    std::vector<Transaction*> getTransactions() const;
    double getDailyWithdrawn();
    double getDailyTransferred();

    bool validatePin(const std::string& pin) const;
    bool changePin(const std::string& oldPin, const std::string& newPin);
    void activate();
    void deactivate();

    virtual std::string accountType() const = 0;
    virtual double getMinimumBalance() const = 0;
    virtual double getMaxWithdrawalPerTransaction() const = 0;
    virtual double getMinWithdrawal() const = 0;
    virtual double calculateWithdrawalLimit() = 0;
    virtual double calculateTransferLimit() = 0;
    virtual double withdrawalFee(double amount) const = 0;
    virtual double transferFee(double amount) const = 0;

    double checkBalance() const;
    virtual Transaction* deposit(double amount);
    virtual Transaction* withdraw(double amount);
    virtual std::pair<Transaction*, Transaction*> transfer(double amount, Account* targetAccount);
    std::vector<Transaction*> getMiniStatement(int count = 5) const;

    friend class CurrentAccount;
    friend std::ostream& operator<<(std::ostream& os, const Account& acc);
};

class SavingsAccount : public Account {
private:
    static constexpr double MIN_BALANCE = 5000;
    static constexpr double MIN_WITHDRAWAL = 500;
    static constexpr double MAX_WITHDRAWAL = 50000;
    static constexpr double DAILY_WITHDRAWAL_LIMIT = 100000;
    static constexpr double DAILY_TRANSFER_LIMIT = 100000;
    static constexpr double WITHDRAWAL_FEE_RATE = 0.001;
    static constexpr double TRANSFER_FEE_RATE = 0.002;

public:
    SavingsAccount(const std::string& accountHolder, const std::string& pin, double initialBalance = 0);

    std::string accountType() const override;
    double getMinimumBalance() const override;
    double getMaxWithdrawalPerTransaction() const override;
    double getMinWithdrawal() const override;
    double calculateWithdrawalLimit() override;
    double calculateTransferLimit() override;
    double withdrawalFee(double amount) const override;
    double transferFee(double amount) const override;
};

class CurrentAccount : public Account {
private:
    static constexpr double MIN_BALANCE = 0;
    static constexpr double OVERDRAFT_LIMIT = 50000;
    static constexpr double MIN_WITHDRAWAL = 500;
    static constexpr double MAX_WITHDRAWAL = 100000;
    static constexpr double DAILY_WITHDRAWAL_LIMIT = 200000;
    static constexpr double DAILY_TRANSFER_LIMIT = 500000;
    static constexpr double WITHDRAWAL_FEE_FLAT = 50;
    static constexpr double TRANSFER_FEE_FLAT = 100;

public:
    CurrentAccount(const std::string& accountHolder, const std::string& pin, double initialBalance = 0);

    std::string accountType() const override;
    double getMinimumBalance() const override;
    double getMaxWithdrawalPerTransaction() const override;
    double getMinWithdrawal() const override;
    double calculateWithdrawalLimit() override;
    double calculateTransferLimit() override;
    double withdrawalFee(double amount) const override;
    double transferFee(double amount) const override;

    Transaction* withdraw(double amount) override;
    std::pair<Transaction*, Transaction*> transfer(double amount, Account* targetAccount) override;
};

#endif
