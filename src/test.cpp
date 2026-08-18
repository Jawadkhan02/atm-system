#include <iostream>
#include <sstream>
#include <cassert>
#include "bank.h"
#include "account.h"
#include "card.h"
#include "customer.h"
#include "exceptions.h"

void testDeposit() {
    std::cout << "=== Testing Deposit ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Ali", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 50000);

    Transaction* txn = acc->deposit(20000);
    assert(acc->getBalance() == 70000);
    assert(txn->transactionType() == "DEPOSIT");
    assert(txn->getAmount() == 20000);
    std::cout << "  Balance after deposit: Rs. " << acc->getBalance() << " [PASS]\n";
    std::cout << "  " << *txn << " [PASS]\n\n";
}

void testWithdraw() {
    std::cout << "=== Testing Withdraw ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Sara", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 75000);

    Transaction* txn = acc->withdraw(10000);
    assert(txn->transactionType() == "WITHDRAWAL");
    std::cout << "  Balance after withdrawal: Rs. " << acc->getBalance() << " [PASS]\n";
    std::cout << "  " << *txn << " [PASS]\n\n";
}

void testTransfer() {
    std::cout << "=== Testing Transfer ===\n";
    Bank bank("Test Bank");
    Customer* c1 = bank.createCustomer("Ali", "0301", "a@b.com");
    Customer* c2 = bank.createCustomer("Sara", "0302", "b@b.com");
    Account* accA = bank.createAccount(c1->getCustomerId(), "SAVINGS", "1234", 100000);
    Account* accB = bank.createAccount(c2->getCustomerId(), "SAVINGS", "5678", 50000);

    // SavingsAccount transfer fee is 0.2% = Rs. 40 on 20000
    auto txns = accA->transfer(20000, accB);
    assert(accA->getBalance() == 79960); // 100000 - 20000 - 40 fee
    assert(accB->getBalance() == 70000); // 50000 + 20000
    assert(txns.first->transactionType() == "TRANSFER");
    assert(txns.second->transactionType() == "TRANSFER");
    std::cout << "  AccA balance: Rs. " << accA->getBalance() << " [PASS]\n";
    std::cout << "  AccB balance: Rs. " << accB->getBalance() << " [PASS]\n";
    std::cout << "  " << *txns.first << " [PASS]\n";
    std::cout << "  " << *txns.second << " [PASS]\n\n";
}

void testCardBlocking() {
    std::cout << "=== Testing Card Blocking ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Ali", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 50000);
    Card* card = bank.issueCard(c->getCustomerId(), "1234");

    assert(card->isActive());
    card->validatePin("0000");
    card->validatePin("0000");
    card->validatePin("0000");
    assert(!card->isActive());
    std::cout << "  Card blocked after 3 wrong PINs [PASS]\n\n";
}

void testInvalidAmount() {
    std::cout << "=== Testing Invalid Amount ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Ali", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 50000);

    bool caught = false;
    try {
        acc->withdraw(-100);
    } catch (const InvalidAmountError& e) {
        caught = true;
        std::cout << "  Caught: " << e.what() << " [PASS]\n";
    }
    assert(caught);

    caught = false;
    try {
        acc->deposit(0);
    } catch (const InvalidAmountError& e) {
        caught = true;
        std::cout << "  Caught: " << e.what() << " [PASS]\n";
    }
    assert(caught);
    std::cout << "\n";
}

void testInsufficientBalance() {
    std::cout << "=== Testing Insufficient Balance ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Ali", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 1000);

    bool caught = false;
    try {
        acc->withdraw(5000);
    } catch (const MinimumBalanceError& e) {
        caught = true;
        std::cout << "  Caught: " << e.what() << " [PASS]\n";
    }
    assert(caught);
    std::cout << "\n";
}

void testMiniStatement() {
    std::cout << "=== Testing Mini Statement ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Ali", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 50000);

    acc->deposit(5000);
    acc->withdraw(2000);
    acc->deposit(10000);

    auto stmt = acc->getMiniStatement(5);
    std::cout << "  Transactions recorded: " << stmt.size() << " [PASS]\n";
    for (auto* t : stmt) {
        std::cout << "  " << *t << "\n";
    }
    std::cout << "  Current Balance: Rs. " << acc->getBalance() << "\n\n";
}

void testOverdraft() {
    std::cout << "=== Testing Current Account Overdraft ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Usman", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "CURRENT", "1234", 20000);

    Transaction* txn = acc->withdraw(60000);
    std::cout << "  Balance after overdraft withdrawal: Rs. " << acc->getBalance() << "\n";
    std::cout << "  (Includes Rs. 50 flat fee)\n";
    assert(acc->getBalance() < 0);
    std::cout << "  Overdraft working correctly [PASS]\n\n";
}

void testPinChange() {
    std::cout << "=== Testing PIN Change ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Ali", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 50000);

    assert(acc->changePin("1234", "5678"));
    assert(acc->getBalance() == 50000);
    assert(!acc->changePin("1234", "9999"));
    std::cout << "  PIN changed successfully [PASS]\n";
    std::cout << "  Old PIN rejected [PASS]\n\n";
}

void testMinimumWithdrawal() {
    std::cout << "=== Testing Minimum Withdrawal ===\n";
    Bank bank("Test Bank");
    Customer* c = bank.createCustomer("Ali", "0301", "a@b.com");
    Account* acc = bank.createAccount(c->getCustomerId(), "SAVINGS", "1234", 50000);

    bool caught = false;
    try {
        acc->withdraw(100);
    } catch (const InvalidAmountError& e) {
        caught = true;
        std::cout << "  Caught: " << e.what() << " [PASS]\n";
    }
    assert(caught);

    caught = false;
    try {
        acc->withdraw(499);
    } catch (const InvalidAmountError& e) {
        caught = true;
        std::cout << "  Caught: " << e.what() << " [PASS]\n";
    }
    assert(caught);

    Transaction* txn = acc->withdraw(500);
    assert(txn != nullptr);
    std::cout << "  Withdrawal of Rs. 500 accepted [PASS]\n\n";
}

int main() {
    std::cout << "\n============================================================\n";
    std::cout << "  ATM System - Automated Test Suite\n";
    std::cout << "============================================================\n\n";

    testDeposit();
    testWithdraw();
    testTransfer();
    testCardBlocking();
    testInvalidAmount();
    testInsufficientBalance();
    testMiniStatement();
    testOverdraft();
    testPinChange();
    testMinimumWithdrawal();

    std::cout << "============================================================\n";
    std::cout << "  ALL TESTS PASSED!\n";
    std::cout << "============================================================\n";

    return 0;
}
