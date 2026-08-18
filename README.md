# ATM System

An interactive ATM simulation demonstrating object-oriented programming concepts. Available in two versions: a **C++17 console application** and a **JavaScript web application**.

**Live Web Version:** [https://jawadkhan02.github.io/atm-system/](https://jawadkhan02.github.io/atm-system/)

---

## Table of Contents

- [Features](#features)
- [Demo Accounts](#demo-accounts)
- [Web Version (JavaScript)](#web-version-javascript)
- [Console Version (C++)](#console-version-c)
- [OOP Concepts](#oop-concepts)
- [Project Structure](#project-structure)
- [Running Tests](#running-tests)

---

## Features

- Card insertion with PIN authentication (3 attempts before auto-block)
- Two account types: **Savings** (percentage-based fees, min balance) and **Current** (overdraft, flat fees)
- Deposit, withdrawal, and inter-account transfers
- Fee breakdown shown before every transaction
- Daily withdrawal and transfer limits with auto-reset
- PIN change (updates both card and account)
- Mini statement (last 5 transactions)
- Real-time balance updates across all accounts

## Demo Accounts

| Customer    | Card Number       | PIN  | Account Type | Balance     |
|-------------|-------------------|------|--------------|-------------|
| Ali Khan    | Auto-generated    | 1234 | Savings      | Rs. 75,000  |
|             |                   |      | Current      | Rs. 150,000 |
| Sara Ahmed  | Auto-generated    | 4321 | Savings      | Rs. 50,000  |
| Usman Ali   | Auto-generated    | 8765 | Current      | Rs. 200,000 |

> Card numbers and account numbers are generated at runtime. All credentials are displayed in the info panel.

---

## Web Version (JavaScript)

A responsive, mobile-friendly web application built with vanilla HTML, CSS, and JavaScript. No frameworks, no dependencies.

### How to Use

1. Visit the [live link](https://jawadkhan02.github.io/atm-system/)
2. Check the **Demo Accounts** panel at the bottom for card number, PIN, and account numbers
3. Click **INSERT CARD** to start
4. Enter card number, then PIN
5. Select an account and perform operations

### Tech Stack

- **HTML5** -- Semantic markup
- **CSS3** -- CSS variables, flexbox, grid, keyframe animations, responsive design
- **Vanilla JavaScript** -- ES6 classes, no external libraries

### Design Features

- Green & white minimal theme
- Compact mini-app layout (mobile-first)
- Smooth animations (fade-in, scale, float, cash slide)
- Responsive across all screen sizes (360px - desktop)
- Terminal-style screen with JetBrains Mono font
- Fee confirmation screen before every transaction

### Files

```
docs/
├── index.html    # Main HTML structure
├── style.css     # All styling and animations
└── script.js     # Complete ATM logic (models + UI)
```

---

## Console Version (C++17)

A Windows console application built with C++17, using `<windows.h>` for console setup and UTF-8 emoji rendering.

### Requirements

- **OS:** Windows
- **Compiler:** g++ with C++17 support (MinGW) or CMake 3.14+

### Build & Run

**Option 1: Direct compilation (g++)**

```bat
compile.bat
atm_system.exe
```

**Option 2: CMake**

```bat
build.bat
build\atm_system.exe
```

### Features (Console-specific)

- Cash denomination tracking (Rs. 500 / 1000 / 5000 notes)
- Physical cash dispensing simulation
- ATM cash availability display
- Custom exception hierarchy (11 exception types)

### Files

```
include/
├── account.h        # Account, SavingsAccount, CurrentAccount
├── atm.h            # ATM machine logic
├── bank.h           # Bank entity management
├── card.h           # Card with PIN validation & blocking
├── customer.h       # Customer entity
├── exceptions.h     # Custom exception hierarchy
└── transaction.h    # Transaction types

src/
├── main.cpp         # Entry point and demo data
├── account.cpp      # Account implementations
├── atm.cpp          # ATM UI and cash dispensing
├── bank.cpp         # Bank management
├── card.cpp         # Card logic
├── customer.cpp     # Customer management
├── test.cpp         # Unit test suite (10 tests)
└── transaction.cpp  # Transaction implementations

CMakeLists.txt
compile.bat
build.bat
```

---

## OOP Concepts

Both versions implement the same object-oriented design, adapted to their respective languages.

### 1. Polymorphism

The `Account` base class defines pure virtual methods that each subclass overrides with its own fee structure, limits, and behavior.

| Method | SavingsAccount | CurrentAccount |
|--------|---------------|----------------|
| `withdrawalFee()` | 0.1% of amount | Flat Rs. 50 |
| `transferFee()` | 0.2% of amount | Flat Rs. 100 |
| `getMinimumBalance()` | Rs. 5,000 | Rs. 0 |
| `withdraw()` | Standard logic | Allows overdraft up to Rs. 50,000 |
| `transfer()` | Standard logic | Allows overdraft up to Rs. 50,000 |

**C++:** Implemented with pure virtual functions (`virtual ... = 0`)
**JavaScript:** Implemented with method overriding in ES6 classes

### 2. Inheritance

A multi-level class hierarchy with shared behavior in base classes and specialized behavior in derived classes.

```
Account (abstract)
├── SavingsAccount
└── CurrentAccount

Transaction (abstract)
├── DepositTransaction
├── WithdrawalTransaction
└── TransferTransaction
```

- `SavingsAccount` and `CurrentAccount` inherit from `Account`
- All transaction types inherit from `Transaction`
- Shared fields (accountNumber, balance, pin) defined once in the base class

### 3. Encapsulation

All class fields are private with controlled access through public getter/setter methods.

| Class | Private Fields | Public Interface |
|-------|---------------|-----------------|
| `Account` | `_balance`, `_pin`, `_transactions`, `_dailyWithdrawn` | `getBalance()`, `validatePin()`, `deposit()`, `withdraw()` |
| `Card` | `_pin`, `_isActive`, `_failedPinAttempts` | `validatePin()`, `changePin()`, `block()` |
| `Customer` | `_accounts`, `_cards` | `addAccount()`, `addCard()`, `getAccountNumbers()` |
| `Bank` | `_customers`, `_accounts`, `_cards` | `createCustomer()`, `createAccount()`, `issueCard()` |

- Direct field access is restricted
- State changes only happen through validated methods
- Internal counters (`_globalCounter`) are hidden from external code

### 4. Exception Handling

**C++ version:** 11 custom exception classes inheriting from `ATMError` (which inherits from `std::runtime_error`):

| Exception | Trigger |
|-----------|---------|
| `InvalidPINError` | Wrong PIN entered |
| `CardBlockedError` | Card blocked after 3 failed attempts |
| `InsufficientBalanceError` | Not enough funds |
| `InsufficientATMFundsError` | ATM cannot dispense amount |
| `InvalidAmountError` | Negative or zero amount |
| `AccountInactiveError` | Operating on inactive account |
| `DailyLimitExceededError` | Withdrawal/transfer limit exceeded |
| `InvalidAccountError` | Account not found |
| `MaxPINAttemptsError` | Max PIN attempts exceeded |
| `SameAccountTransferError` | Transferring to same account |
| `MinimumBalanceError` | Below minimum balance |

**JavaScript version:** Same validation logic using `try/catch` with descriptive error messages.

### 5. Abstraction

Complex banking operations are abstracted behind simple method calls:

```cpp
// C++ - complex validation, fee calculation, limit checking hidden
Transaction* txn = account->withdraw(10000);

// JavaScript - same abstraction
const txn = account.withdraw(10000);
```

The caller doesn't need to know about daily limits, fee calculation, balance validation, or transaction recording -- all handled internally.

### 6. Composition (Has-A Relationships)

- `Bank` **has** `Customer`, `Account`, and `Card` objects
- `Customer` **has** `Account` and `Card` objects
- `Account` **has** `Transaction` objects
- `Card` **has** account numbers (linked to `Account`)

```
Bank ──┬── Customer[] ──┬── Account[]
       │                └── Card[]
       ├── Account[]
       └── Card[]
```

### 7. Static Members

Auto-incrementing IDs across all instances:

| Class | Static Field | Pattern |
|-------|-------------|---------|
| `Account` | `_globalCounter` | `100` + counter (e.g., `1001000001`) |
| `Card` | `_globalCounter` | `4` + counter (e.g., `410000001`) |
| `Customer` | `_globalCounter` | `CUST-` + counter (e.g., `CUST-1001`) |
| `Transaction` | `_globalCounter` | `TXN-` + counter (e.g., `TXN-1001`) |

---

## Running Tests

The C++ version includes 10 unit tests in `src/test.cpp`:

```bat
g++ -std=c++17 -I include -o test.exe src/test.cpp src/account.cpp src/card.cpp src/customer.cpp src/bank.cpp src/transaction.cpp
test.exe
```

**Tests cover:**
- Deposit operations
- Withdrawal operations
- Transfer with fee calculation
- Card blocking after 3 wrong PINs
- Invalid amount handling
- Insufficient balance handling
- Mini statement generation
- Current account overdraft
- PIN change validation
- Minimum withdrawal limits

---

## License

This project is for educational purposes.
