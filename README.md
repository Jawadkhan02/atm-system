# ATM System

An interactive console-based ATM simulation built in C++17, demonstrating object-oriented programming concepts including polymorphism, encapsulation, inheritance, and exception handling.

## Features

- Card insertion with PIN authentication (3 attempts before auto-block)
- Two account types: **Savings** (percentage-based fees, min balance) and **Current** (overdraft, flat fees)
- Deposit, withdrawal, and inter-account transfers
- Cash denomination tracking (Rs. 500 / 1000 / 5000 notes)
- Daily withdrawal and transfer limits with auto-reset
- PIN change (updates both card and account)
- Mini statement (last 5 transactions)
- Custom exception hierarchy for detailed error handling

## Demo Accounts

| Customer    | Card Number       | PIN  | Account Type | Balance   |
|-------------|-------------------|------|--------------|-----------|
| Ali Khan    | Auto-generated    | 1234 | Savings      | Rs. 75,000|
|             |                   |      | Current      | Rs. 150,000|
| Sara Ahmed  | Auto-generated    | 4321 | Savings      | Rs. 50,000|
| Usman Ali   | Auto-generated    | 8765 | Current      | Rs. 200,000|

> Card numbers are generated at runtime. The program displays all credentials on startup.

## Requirements

- **OS:** Windows (uses `<windows.h>` for console setup)
- **Compiler:** g++ with C++17 support (MinGW) or CMake 3.14+

## Build & Run

### Option 1: Direct compilation (g++)

```bat
compile.bat
atm_system.exe
```

### Option 2: CMake

```bat
build.bat
build\atm_system.exe
```

## Project Structure

```
ATM System/
├── include/
│   ├── account.h        # Account, SavingsAccount, CurrentAccount
│   ├── atm.h            # ATM machine logic
│   ├── bank.h           # Bank entity management
│   ├── card.h           # Card with PIN validation & blocking
│   ├── customer.h       # Customer entity
│   ├── exceptions.h     # Custom exception hierarchy
│   └── transaction.h    # Transaction types (deposit, withdrawal, transfer)
├── src/
│   ├── main.cpp         # Entry point and demo data
│   ├── account.cpp      # Account implementations
│   ├── atm.cpp          # ATM UI and cash dispensing
│   ├── bank.cpp         # Bank management
│   ├── card.cpp         # Card logic
│   ├── customer.cpp     # Customer management
│   ├── test.cpp         # Unit test suite (10 tests)
│   └── transaction.cpp  # Transaction implementations
├── CMakeLists.txt
├── compile.bat
├── build.bat
└── README.md
```

## OOP Concepts Demonstrated

- **Polymorphism** -- `Account` base class with `SavingsAccount` / `CurrentAccount` overrides for fees, limits, and overdraft behavior
- **Inheritance** -- `Transaction` hierarchy (`DepositTransaction`, `WithdrawalTransaction`, `TransferTransaction`)
- **Encapsulation** -- Private members with controlled access via getters
- **RAII** -- Destructors manage dynamically allocated transactions and entities
- **Exception Handling** -- 11 custom exception types for domain-specific error handling

## Running Tests

Compile `src/test.cpp` separately:

```bat
g++ -std=c++17 -I include -o test.exe src/test.cpp src/account.cpp src/card.cpp src/customer.cpp src/bank.cpp src/transaction.cpp
test.exe
```

## License

This project is for educational purposes.
