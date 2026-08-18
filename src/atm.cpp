#include "atm.h"
#include "bank.h"
#include "card.h"
#include "customer.h"
#include "account.h"
#include "exceptions.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

// Emoji hex escape shortcuts (verified UTF-8)
#define E_ATM    "\xF0\x9F\x8F\x99"   // 🏧
#define E_MONEY  "\xF0\x9F\x92\xB0"   // 💰
#define E_CARD   "\xF0\x9F\x92\xB3"   // 💳
#define E_LOCK   "\xF0\x9F\x94\x91"   // 🔑
#define E_BANK   "\xF0\x9F\x8F\xA6"   // 🏦
#define E_PUSH   "\xF0\x9F\x92\xB5"   // 💵
#define E_CHECK  "\xE2\x9C\x85"       // ✅
#define E_CROSS  "\xE2\x9D\x8C"       // ❌
#define E_WARN   "\xE2\x9A\xA0"       // ⚠
#define E_ARROW  "\xE2\x96\xB6"       // ▶
#define E_LEFT   "\xE2\xAC\x85"       // ⬅
#define E_RIGHT  "\xE2\x9E\xA1"       // ➡
#define E_SWAP   "\xF0\x9F\x94\x84"   // 🔄
#define E_LIST   "\xF0\x9F\x93\x8B"   // 📋
#define E_EXIT   "\xE2\x9D\x8E"       // ❎
#define E_PERSON "\xF0\x9F\x91\xA4"   // 👤
#define E_BELL   "\xF0\x9F\x94\x94"   // 🔔
#define E_OK     "\xF0\x9F\x86\x94"   // 🆗
#define E_DROP   "\xF0\x9F\x92\xA7"   // 💧
#define E_THANKS "\xF0\x9F\x99\x8F"   // 🙏
#define E_DOCS   "\xF0\x9F\x93\x84"   // 📄
#define E_DOT    "\xC2\xB7"           // ·
#define E_STAR   "\xE2\xAD\x90"       // ⭐
#define E_HI     "\xF0\x9F\x91\x8B"   // 👋

const std::vector<int> ATM::SUPPORTED_DENOMINATIONS = {5000, 1000, 500};

ATM::ATM(Bank& bank, const std::string& location, int notes500, int notes1000, int notes5000)
    : _bank(bank), _location(location), _currentCard(nullptr),
      _currentCustomer(nullptr), _selectedAccount(nullptr) {
    _notes[500] = notes500;
    _notes[1000] = notes1000;
    _notes[5000] = notes5000;
}

std::string ATM::getLocation() const { return _location; }

double ATM::totalCash() const {
    double total = 0;
    for (auto& pair : _notes) {
        total += pair.first * pair.second;
    }
    return total;
}

bool ATM::canDispense(double amount) {
    double remaining = amount;
    std::map<int, int> tempNotes = _notes;

    for (int denom : SUPPORTED_DENOMINATIONS) {
        if (remaining <= 0) break;
        int needed = static_cast<int>(remaining) / denom;
        int available = tempNotes[denom];
        int take = std::min(needed, available);
        remaining -= take * denom;
        tempNotes[denom] = available - take;
    }

    return remaining <= 0.001;
}

std::map<int, int> ATM::dispenseCash(double amount) {
    std::map<int, int> dispensed;
    double remaining = amount;

    for (int denom : SUPPORTED_DENOMINATIONS) {
        if (remaining <= 0) break;
        int needed = static_cast<int>(remaining) / denom;
        int available = _notes[denom];
        int take = std::min(needed, available);
        if (take > 0) {
            dispensed[denom] = take;
            _notes[denom] -= take;
            remaining -= take * denom;
        }
    }

    return dispensed;
}

void ATM::printDenominations(const std::map<int, int>& dispensed) {
    for (auto& pair : dispensed) {
        std::cout << "    " E_PUSH " Rs. " << std::fixed << std::setprecision(0) << pair.first
                  << " x " << pair.second << "\n";
    }
}

void ATM::displayMenu() {
    std::cout << "\n============================================\n";
    std::cout << "       " E_ATM " ====== ATM ====== " E_ATM "\n";
    std::cout << "============================================\n";
    std::cout << "  1. " E_MONEY "  Check Balance\n";
    std::cout << "  2. " E_PUSH "  Deposit\n";
    std::cout << "  3. " E_DROP "  Withdraw\n";
    std::cout << "  4. " E_SWAP "  Transfer Money\n";
    std::cout << "  5. " E_LOCK "  Change PIN\n";
    std::cout << "  6. " E_DOCS "  Mini Statement\n";
    std::cout << "  7. " E_EXIT "  Exit\n";
    std::cout << "============================================\n";
    std::cout << "  Your Account: " << _selectedAccount->getAccountNumber() << "\n";
    std::cout << "============================================\n";
}

bool ATM::selectAccount() {
    std::vector<std::string> accNums = _currentCard->getAccountNumbers();

    if (accNums.size() == 1) {
        _selectedAccount = _bank.getAccount(accNums[0]);
        std::cout << "\n" E_CARD " Using account: " << *_selectedAccount << "\n";
        return true;
    }

    std::cout << "\n" E_CARD " --- Select Account ---\n";
    for (size_t i = 0; i < accNums.size(); i++) {
        Account* acc = _bank.findAccount(accNums[i]);
        if (acc) {
            std::cout << "  " << (i + 1) << ". " << *acc << "\n";
        }
    }
    std::cout << "  " << (accNums.size() + 1) << ". " E_CROSS " Cancel\n";

    std::cout << "\n" E_ARROW " Select account number: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "\n  " E_WARN " Invalid input.\n";
        return false;
    }

    if (choice == static_cast<int>(accNums.size() + 1)) return false;

    if (choice >= 1 && choice <= static_cast<int>(accNums.size())) {
        _selectedAccount = _bank.getAccount(accNums[choice - 1]);
        std::cout << "\n" E_CARD " Using account: " << *_selectedAccount << "\n";
        return true;
    }

    std::cout << "  " E_WARN " Invalid choice.\n";
    return false;
}

bool ATM::insertCard() {
    std::cout << "\n============================================\n";
    std::cout << "       " E_ATM " ====== ATM ====== " E_ATM "\n";
    std::cout << "============================================\n";

    std::cout << "\n" E_CARD " Enter card number: ";
    std::string cardNumber;
    std::cin >> cardNumber;
    std::cin.ignore(10000, '\n');

    std::pair<Card*, Customer*> result = _bank.getCustomerByCard(cardNumber);
    Card* card = result.first;
    Customer* customer = result.second;

    if (!card) {
        std::cout << "\n  " E_CROSS " Card not recognized. Please try again.\n";
        return false;
    }

    if (!card->isActive()) {
        std::cout << "\n  " E_CROSS " This card has been BLOCKED.\n";
        std::cout << "  " E_WARN " Please contact your bank for assistance.\n";
        _currentCard = nullptr;
        _currentCustomer = nullptr;
        return false;
    }

    std::cout << "\n" E_HI " Welcome, " << customer->getName() << "!\n";

    int maxAttempts = 3;
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        int remaining = maxAttempts - attempt;
        std::cout << E_LOCK " Enter PIN (" << remaining << " attempt"
                  << (remaining > 1 ? "s" : "") << " remaining): ";
        std::string pin;
        std::cin >> pin;
        std::cin.ignore(10000, '\n');

        if (card->validatePin(pin)) {
            std::cout << "\n  " E_CHECK " PIN validated successfully!\n";
            _currentCard = card;
            _currentCustomer = customer;
            return true;
        }

        if (!card->isActive()) {
            std::cout << "\n  " E_CROSS " Incorrect PIN. Card has been BLOCKED after "
                      << maxAttempts << " failed attempts.\n";
            std::cout << "  " E_WARN " Please contact your bank for assistance.\n";
            _currentCard = nullptr;
            _currentCustomer = nullptr;
            return false;
        }
        std::cout << "\n  " E_WARN " Incorrect PIN. " << (remaining - 1) << " attempt(s) remaining.\n";
    }

    return false;
}

void ATM::checkBalance() {
    std::cout << "\n--------------------------------------------\n";
    std::cout << "  " E_MONEY " CHECK BALANCE\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "  " E_ARROW " Account: " << _selectedAccount->getAccountNumber() << "\n";
    std::cout << "  " E_DOT " Type:    " << _selectedAccount->accountType() << "\n";
    std::cout << "  " E_DOT " Balance: Rs. " << std::fixed << std::setprecision(0)
              << _selectedAccount->checkBalance() << "\n";
    std::cout << "--------------------------------------------\n";
}

void ATM::deposit() {
    std::cout << "\n--------------------------------------------\n";
    std::cout << "  " E_PUSH " DEPOSIT\n";
    std::cout << "--------------------------------------------\n";

    std::cout << E_ARROW " Enter deposit amount: Rs. ";
    double amount;
    std::cin >> amount;
    std::cin.ignore(10000, '\n');

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        throw InvalidAmountError("Invalid amount entered.");
    }

    Transaction* txn = _selectedAccount->deposit(amount);

    std::cout << "\n  " E_CHECK " Deposit Successful!\n";
    std::cout << "  " E_ARROW " Amount:         Rs. " << std::fixed << std::setprecision(0) << amount << "\n";
    std::cout << "  " E_DOT " Transaction ID: " << txn->getTransactionId() << "\n";
    std::cout << "  " E_DOT " New Balance:    Rs. " << _selectedAccount->getBalance() << "\n";
    std::cout << "--------------------------------------------\n";
}

void ATM::withdraw() {
    std::cout << "\n--------------------------------------------\n";
    std::cout << "  " E_DROP " WITHDRAW\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "  " E_ARROW " ATM Cash Available: Rs. " << std::fixed << std::setprecision(0)
              << totalCash() << "\n";

    std::cout << E_ARROW " Enter withdrawal amount: Rs. ";
    double amount;
    std::cin >> amount;
    std::cin.ignore(10000, '\n');

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        throw InvalidAmountError("Invalid amount entered.");
    }

    if (!canDispense(amount)) {
        throw InsufficientATMFundsError(
            "ATM cannot dispense this exact amount with available denominations.\n"
            "  " E_ARROW " Please try a different amount.");
    }

    Transaction* txn = _selectedAccount->withdraw(amount);
    std::map<int, int> dispensed = dispenseCash(amount);

    std::cout << "\n  " E_CHECK " Withdrawal Successful!\n";
    std::cout << "  " E_ARROW " Amount:         Rs. " << std::fixed << std::setprecision(0) << amount << "\n";
    std::cout << "  " E_DOT " Transaction ID: " << txn->getTransactionId() << "\n";
    std::cout << "  " E_DOT " Please collect your cash:\n";
    printDenominations(dispensed);
    std::cout << "  " E_DOT " New Balance:    Rs. " << _selectedAccount->getBalance() << "\n";
    std::cout << "--------------------------------------------\n";
}

void ATM::transfer() {
    std::cout << "\n--------------------------------------------\n";
    std::cout << "  " E_SWAP " TRANSFER MONEY\n";
    std::cout << "--------------------------------------------\n";

    std::cout << "  (Enter the receiver's ACCOUNT number, not card number)\n";
    std::cout << E_ARROW " Enter receiver account number: ";
    std::string targetAccNum;
    std::cin >> targetAccNum;
    std::cin.ignore(10000, '\n');

    Account* targetAccount = _bank.findAccount(targetAccNum);
    if (!targetAccount) {
        throw InvalidAccountError("Account " + targetAccNum + " not found.");
    }

    if (_selectedAccount->getAccountNumber() == targetAccNum) {
        throw SameAccountTransferError("Cannot transfer to the same account.");
    }

    std::cout << "\n  " E_PERSON " Receiver: " << targetAccount->getAccountHolder() << "\n";
    std::cout << "  " E_BANK " Account:  " << targetAccount->getAccountNumber() << "\n";
    std::cout << "  " E_DOT " Type:     " << targetAccount->accountType() << "\n";

    std::cout << "\n" E_ARROW " Enter transfer amount: Rs. ";
    double amount;
    std::cin >> amount;
    std::cin.ignore(10000, '\n');

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        throw InvalidAmountError("Invalid amount entered.");
    }

    std::pair<Transaction*, Transaction*> txnPair = _selectedAccount->transfer(amount, targetAccount);
    Transaction* senderTxn = txnPair.first;
    Transaction* receiverTxn = txnPair.second;

    std::cout << "\n  " E_CHECK " Transfer Successful!\n";
    std::cout << "  " E_ARROW " Amount:                Rs. " << std::fixed << std::setprecision(0) << amount << "\n";
    std::cout << "  " E_LEFT " From:                  " << _selectedAccount->getAccountNumber() << "\n";
    std::cout << "  " E_RIGHT " To:                    " << targetAccount->getAccountNumber() << "\n";
    std::cout << "  " E_DOT " Transaction ID (Send): " << senderTxn->getTransactionId() << "\n";
    std::cout << "  " E_DOT " Transaction ID (Recv): " << receiverTxn->getTransactionId() << "\n";
    std::cout << "  " E_DOT " Your New Balance:      Rs. " << _selectedAccount->getBalance() << "\n";
    std::cout << "--------------------------------------------\n";
}

void ATM::changePin() {
    std::cout << "\n--------------------------------------------\n";
    std::cout << "  " E_LOCK " CHANGE PIN\n";
    std::cout << "--------------------------------------------\n";

    std::cout << E_ARROW " Enter current PIN: ";
    std::string oldPin;
    std::cin >> oldPin;
    std::cin.ignore(10000, '\n');

    std::cout << E_ARROW " Enter new PIN (4-6 digits): ";
    std::string newPin;
    std::cin >> newPin;
    std::cin.ignore(10000, '\n');

    std::cout << E_ARROW " Confirm new PIN: ";
    std::string confirmPin;
    std::cin >> confirmPin;
    std::cin.ignore(10000, '\n');

    if (newPin != confirmPin) {
        std::cout << "\n  " E_CROSS " [ERROR] PINs do not match.\n";
        std::cout << "--------------------------------------------\n";
        return;
    }

    if (newPin.length() < 4 || newPin.length() > 6) {
        std::cout << "\n  " E_CROSS " [ERROR] PIN must be 4-6 digits.\n";
        std::cout << "--------------------------------------------\n";
        return;
    }

    for (char c : newPin) {
        if (!std::isdigit(c)) {
            std::cout << "\n  " E_CROSS " [ERROR] PIN must contain only digits.\n";
            std::cout << "--------------------------------------------\n";
            return;
        }
    }

    if (_selectedAccount->changePin(oldPin, newPin)) {
        _currentCard->changePin(oldPin, newPin);
        std::cout << "\n  " E_CHECK " PIN changed successfully!\n";
        std::cout << "  " E_DOT " Card PIN and Account PIN both updated.\n";
    } else {
        std::cout << "\n  " E_CROSS " [ERROR] Incorrect current PIN.\n";
    }
    std::cout << "--------------------------------------------\n";
}

void ATM::miniStatement() {
    std::cout << "\n==========================================================\n";
    std::cout << "       " E_DOCS " MINI STATEMENT " E_DOCS "\n";
    std::cout << "==========================================================\n";
    std::cout << "  " E_ARROW " Account: " << _selectedAccount->getAccountNumber() << "\n";
    std::cout << "  " E_DOT " Type:    " << _selectedAccount->accountType() << "\n";
    std::cout << "----------------------------------------------------------\n";
    std::cout << "  " << std::left << std::setw(14) << "TXN ID"
              << std::setw(10) << "Date"
              << std::setw(12) << "Type"
              << std::right << std::setw(12) << "Amount" << "\n";
    std::cout << "----------------------------------------------------------\n";

    std::vector<Transaction*> transactions = _selectedAccount->getMiniStatement(5);

    if (transactions.empty()) {
        std::cout << "  No transactions found.\n";
    } else {
        for (auto* txn : transactions) {
            std::cout << "  " << *txn << "\n";
        }
    }

    std::cout << "----------------------------------------------------------\n";
    std::cout << "  " E_DOT " Current Balance: Rs. " << std::fixed << std::setprecision(0)
              << _selectedAccount->getBalance() << "\n";
    std::cout << "==========================================================\n";
}

void ATM::run() {
    if (!insertCard()) return;

    while (true) {
        if (!selectAccount()) {
            ejectCard();
            return;
        }

        displayMenu();

        std::cout << "\n" E_ARROW " Select an option (1-7): ";
        std::string choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        try {
            if (choice == "1") checkBalance();
            else if (choice == "2") deposit();
            else if (choice == "3") withdraw();
            else if (choice == "4") transfer();
            else if (choice == "5") changePin();
            else if (choice == "6") miniStatement();
            else if (choice == "7") { ejectCard(); return; }
            else std::cout << "\n  " E_WARN " Invalid option. Please select 1-7.\n";
        } catch (const ATMError& e) {
            std::cout << "\n  " E_CROSS " [ERROR] " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cout << "\n  " E_CROSS " [ERROR] Invalid input. Please try again.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }
}

void ATM::ejectCard() {
    std::cout << "\n  " E_THANKS " Thank you for using the ATM!\n";
    std::cout << "  " E_CARD " Please take your card. Goodbye!\n";
    _currentCard = nullptr;
    _currentCustomer = nullptr;
    _selectedAccount = nullptr;
}
