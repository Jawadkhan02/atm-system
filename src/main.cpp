#include <iostream>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include "bank.h"
#include "atm.h"
#include "customer.h"
#include "account.h"
#include "card.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    SetConsoleTitleA("ATM System - OOP Assignment");

    std::cout << "\n============================================================\n";
    std::cout << "     \xF0\x9F\x8F\x99  Welcome to the ATM System  \xF0\x9F\x8F\x99\n";
    std::cout << "     \xF0\x9F\x93\x9A Class-Based OOP Assignment (C++)\n";
    std::cout << "============================================================\n";

    Bank bank("National Bank of Pakistan");

    Customer* ali = bank.createCustomer("Ali Khan", "0301-1234567", "ali@email.com");
    Customer* sara = bank.createCustomer("Sara Ahmed", "0321-9876543", "sara@email.com");
    Customer* usman = bank.createCustomer("Usman Ali", "0333-5551234", "usman@email.com");

    Account* aliSavings = bank.createAccount(ali->getCustomerId(), "SAVINGS", "1234", 75000);
    Account* aliCurrent = bank.createAccount(ali->getCustomerId(), "CURRENT", "5678", 150000);
    Account* saraSavings = bank.createAccount(sara->getCustomerId(), "SAVINGS", "4321", 50000);
    Account* usmanCurrent = bank.createAccount(usman->getCustomerId(), "CURRENT", "8765", 200000);

    Card* cardAli = bank.issueCard(ali->getCustomerId(), "1234");
    Card* cardSara = bank.issueCard(sara->getCustomerId(), "4321");
    Card* cardUsman = bank.issueCard(usman->getCustomerId(), "8765");

    std::cout << "\n============================================================\n";
    std::cout << "        \xF0\x9F\x93\x8B DEMO ACCOUNT INFORMATION \xF0\x9F\x93\x8B\n";
    std::cout << "============================================================\n";
    std::cout << "  \xF0\x9F\x93\x8C Use these credentials to test the ATM:\n";
    std::cout << "------------------------------------------------------------\n\n";

    std::cout << "  \xF0\x9F\x91\xA4 Customer 1: Ali Khan\n";
    std::cout << "    \xF0\x9F\x92\xB3 Card: " << cardAli->getCardNumber() << "  |  \xF0\x9F\x94\x91 PIN: 1234\n";
    std::cout << "    \xF0\x9F\x8F\xA6 Savings Account: " << aliSavings->getAccountNumber()
              << "  |  \xF0\x9F\x92\xB5 Balance: Rs. " << std::fixed << std::setprecision(0)
              << aliSavings->getBalance() << "\n";
    std::cout << "    \xF0\x9F\x8F\xA6 Current Account: " << aliCurrent->getAccountNumber()
              << "  |  \xF0\x9F\x92\xB5 Balance: Rs. " << aliCurrent->getBalance() << "\n\n";

    std::cout << "  \xF0\x9F\x91\xA4 Customer 2: Sara Ahmed\n";
    std::cout << "    \xF0\x9F\x92\xB3 Card: " << cardSara->getCardNumber() << "  |  \xF0\x9F\x94\x91 PIN: 4321\n";
    std::cout << "    \xF0\x9F\x8F\xA6 Savings Account: " << saraSavings->getAccountNumber()
              << "  |  \xF0\x9F\x92\xB5 Balance: Rs. " << saraSavings->getBalance() << "\n\n";

    std::cout << "  \xF0\x9F\x91\xA4 Customer 3: Usman Ali\n";
    std::cout << "    \xF0\x9F\x92\xB3 Card: " << cardUsman->getCardNumber() << "  |  \xF0\x9F\x94\x91 PIN: 8765\n";
    std::cout << "    \xF0\x9F\x8F\xA6 Current Account: " << usmanCurrent->getAccountNumber()
              << "  |  \xF0\x9F\x92\xB5 Balance: Rs. " << usmanCurrent->getBalance() << "\n\n";

    std::cout << "============================================================\n";

    ATM atm(bank, "Main Branch - Islamabad", 20, 30, 10);

    std::cout << "\n\xF0\x9F\x93\xCD ATM Location: " << atm.getLocation() << "\n";
    std::cout << "\xF0\x9F\x92\xB0 ATM Cash: Rs. " << std::fixed << std::setprecision(0)
              << atm.totalCash() << "\n";

    while (true) {
        try {
            std::cout << "\n------------------------------------------------------------\n";
            std::cout << "  \xE2\x96\xB6  Press ENTER to start a new session (or type 'quit' to exit): ";
            std::string input;
            if (!std::getline(std::cin, input)) {
                std::cout << "\n  \xF0\x9F\x99\x8F Thank you for using the ATM System. Goodbye!\n";
                break;
            }
            if (input == "quit" || input == "QUIT") {
                std::cout << "\n  \xF0\x9F\x99\x8F Thank you for using the ATM System. Goodbye!\n";
                break;
            }
            atm.run();
        } catch (const std::exception& e) {
            std::cout << "\n  \xE2\x9D\x8C [SYSTEM ERROR] " << e.what() << "\n";
            std::cout << "  Restarting session...\n";
        }
    }

    return 0;
}
