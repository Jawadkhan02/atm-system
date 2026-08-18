// ==================== DATA MODELS ====================

class Transaction {
    constructor(amount, accountNumber) {
        this._transactionId = 'TXN-' + (Transaction._counter++);
        this._amount = amount;
        this._accountNumber = accountNumber;
        this._dateTime = new Date();
        this._status = 'PENDING';
    }
    getTransactionId() { return this._transactionId; }
    getAmount() { return this._amount; }
    getAccountNumber() { return this._accountNumber; }
    getStatus() { return this._status; }
    setStatus(s) { this._status = s; }
    getFormattedDate() {
        const d = this._dateTime;
        const months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
        return d.getDate() + '-' + months[d.getMonth()];
    }
    transactionType() { return ''; }
    displayAmount() { return ''; }
    toString() {
        return `${this._transactionId.padEnd(12)} ${this.getFormattedDate().padEnd(8)} ${this.transactionType().padEnd(10)} ${this.displayAmount().padStart(10)}`;
    }
}
Transaction._counter = 1000;

class DepositTransaction extends Transaction {
    constructor(amount, acc) { super(amount, acc); }
    transactionType() { return 'DEPOSIT'; }
    displayAmount() { return '+' + this._amount; }
}

class WithdrawalTransaction extends Transaction {
    constructor(amount, acc) { super(amount, acc); }
    transactionType() { return 'WITHDRAWAL'; }
    displayAmount() { return '-' + this._amount; }
}

class TransferTransaction extends Transaction {
    constructor(amount, acc, target, isSender) {
        super(amount, acc);
        this._target = target;
        this._isSender = isSender;
    }
    transactionType() { return 'TRANSFER'; }
    displayAmount() { return (this._isSender ? '-' : '+') + this._amount; }
    getTargetAccountNumber() { return this._target; }
}

// ==================== ACCOUNT ====================

class Account {
    static _counter = 1000000;
    constructor(holder, pin, balance = 0) {
        this._accountNumber = '100' + (++Account._counter);
        this._accountHolder = holder;
        this._pin = pin;
        this._balance = balance;
        this._isActive = true;
        this._transactions = [];
        this._dailyWithdrawn = 0;
        this._dailyTransferred = 0;
        this._lastTransactionDate = new Date();
    }
    getAccountNumber() { return this._accountNumber; }
    getAccountHolder() { return this._accountHolder; }
    getBalance() { return this._balance; }
    isActive() { return this._isActive; }
    getTransactions() { return this._transactions; }

    _resetDailyLimits() {
        const now = new Date();
        const last = this._lastTransactionDate;
        if (now.getDate() !== last.getDate() || now.getMonth() !== last.getMonth() || now.getFullYear() !== last.getFullYear()) {
            this._dailyWithdrawn = 0;
            this._dailyTransferred = 0;
            this._lastTransactionDate = now;
        }
    }
    getDailyWithdrawn() { this._resetDailyLimits(); return this._dailyWithdrawn; }
    getDailyTransferred() { this._resetDailyLimits(); return this._dailyTransferred; }

    validatePin(pin) { return pin === this._pin; }
    changePin(oldPin, newPin) {
        if (oldPin !== this._pin) return false;
        if (newPin.length < 4 || newPin.length > 6) return false;
        if (!/^\d+$/.test(newPin)) return false;
        this._pin = newPin;
        return true;
    }
    activate() { this._isActive = true; }
    deactivate() { this._isActive = false; }

    _checkActive() { if (!this._isActive) throw new Error('Account ' + this._accountNumber + ' is inactive.'); }
    _validateAmount(a) { if (a <= 0) throw new Error('Amount must be positive.'); }

    accountType() { return ''; }
    getMinimumBalance() { return 0; }
    getMaxWithdrawalPerTransaction() { return 0; }
    getMinWithdrawal() { return 0; }
    calculateWithdrawalLimit() { return 0; }
    calculateTransferLimit() { return 0; }
    withdrawalFee(a) { return 0; }
    transferFee(a) { return 0; }

    deposit(amount) {
        this._checkActive();
        this._validateAmount(amount);
        this._balance += amount;
        const txn = new DepositTransaction(amount, this._accountNumber);
        txn.setStatus('COMPLETED');
        this._transactions.push(txn);
        return txn;
    }

    withdraw(amount) {
        this._checkActive();
        this._validateAmount(amount);
        this._resetDailyLimits();
        if (amount < this.getMinWithdrawal()) throw new Error('Minimum withdrawal amount is Rs. ' + this.getMinWithdrawal() + '.');
        if (amount > this.getMaxWithdrawalPerTransaction()) throw new Error('Maximum withdrawal per transaction is Rs. ' + this.getMaxWithdrawalPerTransaction() + '.');
        const dLimit = this.calculateWithdrawalLimit();
        if (this._dailyWithdrawn + amount > dLimit) throw new Error('Daily withdrawal limit of Rs. ' + dLimit + ' exceeded. Already withdrawn: Rs. ' + this._dailyWithdrawn + '.');
        const fee = this.withdrawalFee(amount);
        const total = amount + fee;
        if (this._balance - total < this.getMinimumBalance()) throw new Error('Insufficient balance. Minimum balance: Rs. ' + this.getMinimumBalance() + '. With fee Rs. ' + fee + ', total needed: Rs. ' + total + '.');
        this._balance -= total;
        this._dailyWithdrawn += amount;
        const txn = new WithdrawalTransaction(amount, this._accountNumber);
        txn.setStatus('COMPLETED');
        this._transactions.push(txn);
        return txn;
    }

    transfer(amount, target) {
        this._checkActive();
        target._checkActive();
        this._validateAmount(amount);
        if (this._accountNumber === target.getAccountNumber()) throw new Error('Cannot transfer to the same account.');
        this._resetDailyLimits();
        const dLimit = this.calculateTransferLimit();
        if (this._dailyTransferred + amount > dLimit) throw new Error('Daily transfer limit of Rs. ' + dLimit + ' exceeded. Already transferred: Rs. ' + this._dailyTransferred + '.');
        const fee = this.transferFee(amount);
        const total = amount + fee;
        if (this._balance - total < this.getMinimumBalance()) throw new Error('Insufficient balance for transfer. Minimum balance: Rs. ' + this.getMinimumBalance() + '. With fee Rs. ' + fee + ', total needed: Rs. ' + total + '.');
        this._balance -= total;
        target._balance += amount;
        this._dailyTransferred += amount;
        const sTxn = new TransferTransaction(amount, this._accountNumber, target.getAccountNumber(), true);
        sTxn.setStatus('COMPLETED');
        this._transactions.push(sTxn);
        const rTxn = new TransferTransaction(amount, target.getAccountNumber(), this._accountNumber, false);
        rTxn.setStatus('COMPLETED');
        target._transactions.push(rTxn);
        return [sTxn, rTxn];
    }

    getMiniStatement(count = 5) {
        const n = Math.min(count, this._transactions.length);
        return this._transactions.slice(-n);
    }
}

class SavingsAccount extends Account {
    constructor(holder, pin, balance = 0) {
        super(holder, pin, balance);
        this.MIN_BALANCE = 5000;
        this.MIN_WITHDRAWAL = 500;
        this.MAX_WITHDRAWAL = 50000;
        this.DAILY_WITHDRAWAL_LIMIT = 100000;
        this.DAILY_TRANSFER_LIMIT = 100000;
        this.WITHDRAWAL_FEE_RATE = 0.001;
        this.TRANSFER_FEE_RATE = 0.002;
    }
    accountType() { return 'SAVINGS'; }
    getMinimumBalance() { return this.MIN_BALANCE; }
    getMaxWithdrawalPerTransaction() { return this.MAX_WITHDRAWAL; }
    getMinWithdrawal() { return this.MIN_WITHDRAWAL; }
    calculateWithdrawalLimit() { return this.DAILY_WITHDRAWAL_LIMIT; }
    calculateTransferLimit() { return this.DAILY_TRANSFER_LIMIT; }
    withdrawalFee(a) { return a * this.WITHDRAWAL_FEE_RATE; }
    transferFee(a) { return a * this.TRANSFER_FEE_RATE; }
}

class CurrentAccount extends Account {
    constructor(holder, pin, balance = 0) {
        super(holder, pin, balance);
        this.MIN_BALANCE = 0;
        this.OVERDRAFT_LIMIT = 50000;
        this.MIN_WITHDRAWAL = 500;
        this.MAX_WITHDRAWAL = 100000;
        this.DAILY_WITHDRAWAL_LIMIT = 200000;
        this.DAILY_TRANSFER_LIMIT = 500000;
        this.WITHDRAWAL_FEE_FLAT = 50;
        this.TRANSFER_FEE_FLAT = 100;
    }
    accountType() { return 'CURRENT'; }
    getMinimumBalance() { return this.MIN_BALANCE; }
    getMaxWithdrawalPerTransaction() { return this.MAX_WITHDRAWAL; }
    getMinWithdrawal() { return this.MIN_WITHDRAWAL; }
    calculateWithdrawalLimit() { return this.DAILY_WITHDRAWAL_LIMIT; }
    calculateTransferLimit() { return this.DAILY_TRANSFER_LIMIT; }
    withdrawalFee(a) { return this.WITHDRAWAL_FEE_FLAT; }
    transferFee(a) { return this.TRANSFER_FEE_FLAT; }

    withdraw(amount) {
        this._checkActive();
        this._validateAmount(amount);
        this._resetDailyLimits();
        if (amount < this.getMinWithdrawal()) throw new Error('Minimum withdrawal amount is Rs. ' + this.getMinWithdrawal() + '.');
        if (amount > this.getMaxWithdrawalPerTransaction()) throw new Error('Maximum withdrawal per transaction is Rs. ' + this.getMaxWithdrawalPerTransaction() + '.');
        const dLimit = this.calculateWithdrawalLimit();
        if (this._dailyWithdrawn + amount > dLimit) throw new Error('Daily withdrawal limit of Rs. ' + dLimit + ' exceeded. Already withdrawn: Rs. ' + this._dailyWithdrawn + '.');
        const fee = this.withdrawalFee(amount);
        const total = amount + fee;
        if (this._balance - total < -this.OVERDRAFT_LIMIT) throw new Error('Insufficient balance. Overdraft limit: Rs. ' + this.OVERDRAFT_LIMIT + '. Available: Rs. ' + (this._balance + this.OVERDRAFT_LIMIT) + '.');
        this._balance -= total;
        this._dailyWithdrawn += amount;
        const txn = new WithdrawalTransaction(amount, this._accountNumber);
        txn.setStatus('COMPLETED');
        this._transactions.push(txn);
        return txn;
    }

    transfer(amount, target) {
        this._checkActive();
        target._checkActive();
        this._validateAmount(amount);
        if (this._accountNumber === target.getAccountNumber()) throw new Error('Cannot transfer to the same account.');
        this._resetDailyLimits();
        const dLimit = this.calculateTransferLimit();
        if (this._dailyTransferred + amount > dLimit) throw new Error('Daily transfer limit of Rs. ' + dLimit + ' exceeded. Already transferred: Rs. ' + this._dailyTransferred + '.');
        const fee = this.transferFee(amount);
        const total = amount + fee;
        if (this._balance - total < -this.OVERDRAFT_LIMIT) throw new Error('Insufficient balance for transfer. Overdraft limit: Rs. ' + this.OVERDRAFT_LIMIT + '. Available: Rs. ' + (this._balance + this.OVERDRAFT_LIMIT) + '.');
        this._balance -= total;
        target._balance += amount;
        this._dailyTransferred += amount;
        const sTxn = new TransferTransaction(amount, this._accountNumber, target.getAccountNumber(), true);
        sTxn.setStatus('COMPLETED');
        this._transactions.push(sTxn);
        const rTxn = new TransferTransaction(amount, target.getAccountNumber(), this._accountNumber, false);
        rTxn.setStatus('COMPLETED');
        target._transactions.push(rTxn);
        return [sTxn, rTxn];
    }
}

// ==================== CARD ====================

class Card {
    static _counter = 10000000;
    constructor(pin, customerId, accountNumbers) {
        this._cardNumber = '4' + (++Card._counter);
        this._pin = pin;
        this._customerId = customerId;
        this._accountNumbers = [...accountNumbers];
        this._isActive = true;
        this._failedPinAttempts = 0;
        this._maxPinAttempts = 3;
    }
    getCardNumber() { return this._cardNumber; }
    getCustomerId() { return this._customerId; }
    getAccountNumbers() { return this._accountNumbers; }
    isActive() { return this._isActive; }

    validatePin(pin) {
        if (!this._isActive) return false;
        if (pin === this._pin) { this._failedPinAttempts = 0; return true; }
        this._failedPinAttempts++;
        if (this._failedPinAttempts >= this._maxPinAttempts) this._isActive = false;
        return false;
    }
    changePin(oldPin, newPin) {
        if (oldPin !== this._pin) return false;
        if (newPin.length < 4 || newPin.length > 6) return false;
        if (!/^\d+$/.test(newPin)) return false;
        this._pin = newPin;
        return true;
    }
    block() { this._isActive = false; }
    unblock() { this._isActive = true; this._failedPinAttempts = 0; }
    addAccount(accNum) {
        if (!this._accountNumbers.includes(accNum)) this._accountNumbers.push(accNum);
    }
    getMaskedNumber() { return '****-****-****-' + this._cardNumber.slice(-4); }
}

// ==================== CUSTOMER ====================

class Customer {
    static _counter = 1000;
    constructor(name, phone, email) {
        this._customerId = 'CUST-' + (++Customer._counter);
        this._name = name;
        this._phone = phone;
        this._email = email;
        this._accounts = {};
        this._cards = [];
    }
    getCustomerId() { return this._customerId; }
    getName() { return this._name; }
    getPhone() { return this._phone; }
    getEmail() { return this._email; }
    getAccounts() { return this._accounts; }
    getCards() { return this._cards; }
    addAccount(acc) { this._accounts[acc.getAccountNumber()] = acc; }
    getAccount(accNum) { return this._accounts[accNum] || null; }
    getAccountNumbers() { return Object.keys(this._accounts); }
    addCard(card) { this._cards.push(card); }
}

// ==================== BANK ====================

class Bank {
    constructor(name) {
        this._name = name;
        this._customers = {};
        this._accounts = {};
        this._cards = {};
    }
    getName() { return this._name; }
    createCustomer(name, phone, email) {
        const c = new Customer(name, phone, email);
        this._customers[c.getCustomerId()] = c;
        return c;
    }
    createAccount(customerId, type, pin, balance = 0) {
        const c = this._customers[customerId];
        if (!c) throw new Error('Customer not found.');
        let acc;
        if (type === 'SAVINGS') acc = new SavingsAccount(c.getName(), pin, balance);
        else if (type === 'CURRENT') acc = new CurrentAccount(c.getName(), pin, balance);
        else throw new Error('Invalid account type.');
        c.addAccount(acc);
        this._accounts[acc.getAccountNumber()] = acc;
        return acc;
    }
    issueCard(customerId, pin) {
        const c = this._customers[customerId];
        if (!c) throw new Error('Customer not found.');
        const card = new Card(pin, customerId, c.getAccountNumbers());
        c.addCard(card);
        this._cards[card.getCardNumber()] = card;
        return card;
    }
    getAccount(accNum) {
        const acc = this._accounts[accNum];
        if (!acc) throw new Error('Account ' + accNum + ' not found.');
        return acc;
    }
    findAccount(accNum) { return this._accounts[accNum] || null; }
    getCustomerByCard(cardNum) {
        const card = this._cards[cardNum];
        if (!card) return [null, null];
        const cust = this._customers[card.getCustomerId()];
        return [card, cust || null];
    }
}

// ==================== ATM UI ====================

const screen = document.getElementById('screen');
const cardSlot = document.getElementById('cardSlot');
const cashSlot = document.getElementById('cashSlot');
const keypad = document.getElementById('keypad');

let bank, currentCard, currentCustomer, selectedAccount;
let inputBuffer = '';
let inputMode = null; // 'cardNumber' | 'pin' | 'amount' | 'targetAcc' | 'oldPin' | 'newPin' | 'confirmPin' | 'menuChoice' | 'accSelect'
let inputCallback = null;
let tempData = {};

function fmt(n) { return n.toLocaleString('en-PK'); }

function render(html) { screen.innerHTML = html; }

function showWelcome() {
    render(`
        <div class="title">ATM System</div>
        <div class="center" style="margin-top:60px">
            <div style="font-size:2rem;margin-bottom:20px">ATM</div>
            <p class="info">Insert your card to begin</p>
            <p class="dim" style="margin-top:40px">Card numbers are generated at runtime.<br>Check the info panel below for demo credentials.</p>
        </div>
    `);
}

function promptInput(placeholder, mode, callback) {
    inputBuffer = '';
    inputMode = mode;
    inputCallback = callback;
    render(`
        <div class="title">ATM System</div>
        <p style="margin-bottom:12px">${placeholder}</p>
        <div class="input-line">
            <div class="input-display" id="inputDisplay">${mode === 'pin' || mode === 'newPin' || mode === 'confirmPin' || mode === 'oldPin' ? '****' : ''}</div>
        </div>
        <p class="dim" style="margin-top:12px">Use keypad or keyboard</p>
    `);
}

function onKey(value) {
    if (!inputMode) return;

    if (value === 'clear') {
        inputBuffer = '';
        updateInputDisplay();
        return;
    }
    if (value === 'enter') {
        if (inputBuffer.length === 0) return;
        const cb = inputCallback;
        const val = inputBuffer;
        inputBuffer = '';
        inputMode = null;
        inputCallback = null;
        cb(val);
        return;
    }
    inputBuffer += value;
    updateInputDisplay();
}

function updateInputDisplay() {
    const el = document.getElementById('inputDisplay');
    if (!el) return;
    if (inputMode === 'pin' || inputMode === 'oldPin' || inputMode === 'newPin' || inputMode === 'confirmPin') {
        el.textContent = inputBuffer ? '*'.repeat(inputBuffer.length) : '';
    } else {
        el.textContent = inputBuffer;
    }
}

// Keypad
keypad.addEventListener('click', (e) => {
    const btn = e.target.closest('.key');
    if (!btn) return;
    onKey(btn.dataset.value);
});

// Keyboard
document.addEventListener('keydown', (e) => {
    if (e.key >= '0' && e.key <= '9') onKey(e.key);
    else if (e.key === 'Enter') onKey('enter');
    else if (e.key === 'Backspace' || e.key === 'Escape') onKey('clear');
});

// Card slot click
cardSlot.addEventListener('click', () => startSession());

// ==================== SESSION FLOW ====================

function startSession() {
    inputBuffer = '';
    inputMode = null;
    currentCard = null;
    currentCustomer = null;
    selectedAccount = null;

    promptInput('Enter card number:', 'cardNumber', (cardNum) => {
        const [card, cust] = bank.getCustomerByCard(cardNum);
        if (!card) {
            render(`
                <div class="title">ATM System</div>
                <p class="error" style="margin-top:40px;text-align:center">Card not recognized.<br>Please try again.</p>
                <div class="center" style="margin-top:30px"><button class="btn" onclick="showWelcome()">Back</button></div>
            `);
            return;
        }
        if (!card.isActive()) {
            render(`
                <div class="title">ATM System</div>
                <p class="error" style="margin-top:40px;text-align:center">This card has been BLOCKED.<br>Please contact your bank.</p>
                <div class="center" style="margin-top:30px"><button class="btn" onclick="showWelcome()">Back</button></div>
            `);
            return;
        }
        currentCard = card;
        currentCustomer = cust;
        cardSlot.textContent = card.getMaskedNumber();
        cardSlot.classList.add('inserted');
        promptPin(3);
    });
}

function promptPin(attemptsLeft) {
    promptInput(`Enter PIN (${attemptsLeft} attempt${attemptsLeft > 1 ? 's' : ''} remaining):`, 'pin', (pin) => {
        if (currentCard.validatePin(pin)) {
            render(`
                <div class="title">ATM System</div>
                <p class="success" style="margin-top:40px;text-align:center">PIN validated successfully!</p>
                <p style="margin-top:12px;text-align:center">Welcome, ${currentCustomer.getName()}!</p>
            `);
            setTimeout(() => selectAccount(), 1000);
        } else {
            if (!currentCard.isActive()) {
                render(`
                    <div class="title">ATM System</div>
                    <p class="error" style="margin-top:40px;text-align:center">Incorrect PIN. Card BLOCKED after 3 failed attempts.</p>
                    <div class="center" style="margin-top:30px"><button class="btn" onclick="ejectAndReset()">OK</button></div>
                `);
            } else {
                render(`
                    <div class="title">ATM System</div>
                    <p class="warning" style="margin-top:40px;text-align:center">Incorrect PIN. ${attemptsLeft - 1} attempt(s) remaining.</p>
                    <div class="center" style="margin-top:30px"><button class="btn primary" onclick="promptPin(${attemptsLeft - 1})">Try Again</button></div>
                `);
            }
        }
    });
}

function selectAccount() {
    const accNums = currentCard.getAccountNumbers();
    if (accNums.length === 1) {
        selectedAccount = bank.getAccount(accNums[0]);
        showMenu();
        return;
    }
    let html = `<div class="title">Select Account</div>`;
    accNums.forEach((num, i) => {
        const acc = bank.findAccount(num);
        if (acc) {
            html += `<p class="menu-item" onclick="pickAccount('${num}')">${i + 1}. ${acc.accountType()} | ${num} | Rs. ${fmt(acc.getBalance())}</p>`;
        }
    });
    html += `<p class="menu-item error" onclick="ejectAndReset()">${accNums.length + 1}. Cancel</p>`;
    render(html);
}

function pickAccount(accNum) {
    selectedAccount = bank.getAccount(accNum);
    showMenu();
}

function showMenu() {
    render(`
        <div class="title">ATM Menu</div>
        <p class="info">Account: ${selectedAccount.getAccountNumber()}</p>
        <hr class="divider">
        <p class="menu-item" onclick="doCheckBalance()">1. Check Balance</p>
        <p class="menu-item" onclick="doDeposit()">2. Deposit</p>
        <p class="menu-item" onclick="doWithdraw()">3. Withdraw</p>
        <p class="menu-item" onclick="doTransfer()">4. Transfer Money</p>
        <p class="menu-item" onclick="doChangePin()">5. Change PIN</p>
        <p class="menu-item" onclick="doMiniStatement()">6. Mini Statement</p>
        <p class="menu-item error" onclick="ejectCard()">7. Exit</p>
    `);
}

// ==================== OPERATIONS ====================

function doCheckBalance() {
    render(`
        <div class="title">Check Balance</div>
        <p>Account: ${selectedAccount.getAccountNumber()}</p>
        <p>Type: ${selectedAccount.accountType()}</p>
        <hr class="divider">
        <p style="font-size:1.4rem;margin-top:10px">Balance: <span class="highlight">Rs. ${fmt(selectedAccount.getBalance())}</span></p>
        <hr class="divider">
        <div style="margin-top:16px"><button class="btn" onclick="showMenu()">Back to Menu</button></div>
    `);
}

function doDeposit() {
    promptInput('Enter deposit amount (Rs.):', 'amount', (val) => {
        const amount = parseFloat(val);
        if (isNaN(amount) || amount <= 0) { showError('Invalid amount.'); return; }
        try {
            const txn = selectedAccount.deposit(amount);
            render(`
                <div class="title">Deposit Successful</div>
                <p>Amount: Rs. ${fmt(amount)}</p>
                <p>Transaction ID: ${txn.getTransactionId()}</p>
                <p>New Balance: <span class="highlight">Rs. ${fmt(selectedAccount.getBalance())}</span></p>
                <hr class="divider">
                <div style="margin-top:16px"><button class="btn" onclick="showMenu()">Back to Menu</button></div>
            `);
        } catch (e) { showError(e.message); }
    });
}

function doWithdraw() {
    render(`
        <div class="title">Withdraw</div>
        <p class="info">ATM Cash Available: Rs. ${fmt(getAtmCash())}</p>
        <hr class="divider">
        <p>Enter withdrawal amount (Rs.):</p>
        <div class="input-line">
            <div class="input-display" id="inputDisplay"></div>
        </div>
        <p class="dim" style="margin-top:8px">Use keypad or keyboard</p>
    `);
    inputBuffer = '';
    inputMode = 'amount';
    inputCallback = (val) => {
        const amount = parseFloat(val);
        if (isNaN(amount) || amount <= 0) { showError('Invalid amount.'); return; }
        try {
            const txn = selectedAccount.withdraw(amount);
            showCashDispensing(amount);
            setTimeout(() => {
                render(`
                    <div class="title">Withdrawal Successful</div>
                    <p>Amount: Rs. ${fmt(amount)}</p>
                    <p>Transaction ID: ${txn.getTransactionId()}</p>
                    <p>New Balance: <span class="highlight">Rs. ${fmt(selectedAccount.getBalance())}</span></p>
                    <hr class="divider">
                    <div style="margin-top:16px"><button class="btn" onclick="showMenu()">Back to Menu</button></div>
                `);
            }, 2000);
        } catch (e) { showError(e.message); }
    };
}

function doTransfer() {
    promptInput("Enter receiver's account number:", 'targetAcc', (targetNum) => {
        const target = bank.findAccount(targetNum);
        if (!target) { showError('Account not found.'); return; }
        if (selectedAccount.getAccountNumber() === targetNum) { showError('Cannot transfer to the same account.'); return; }
        tempData.target = target;
        render(`
            <div class="title">Transfer Money</div>
            <p class="success">Receiver: ${target.getAccountHolder()}</p>
            <p>Account: ${target.getAccountNumber()} (${target.accountType()})</p>
            <hr class="divider">
            <p>Enter transfer amount (Rs.):</p>
            <div class="input-line">
                <div class="input-display" id="inputDisplay"></div>
            </div>
            <p class="dim" style="margin-top:8px">Use keypad or keyboard</p>
        `);
        inputBuffer = '';
        inputMode = 'amount';
        inputCallback = (val) => {
            const amount = parseFloat(val);
            if (isNaN(amount) || amount <= 0) { showError('Invalid amount.'); return; }
            try {
                const [sTxn, rTxn] = selectedAccount.transfer(amount, tempData.target);
                render(`
                    <div class="title">Transfer Successful</div>
                    <p>Amount: Rs. ${fmt(amount)}</p>
                    <p>From: ${selectedAccount.getAccountNumber()}</p>
                    <p>To: ${tempData.target.getAccountNumber()}</p>
                    <p>TXN (Sender): ${sTxn.getTransactionId()}</p>
                    <p>TXN (Receiver): ${rTxn.getTransactionId()}</p>
                    <p>Your New Balance: <span class="highlight">Rs. ${fmt(selectedAccount.getBalance())}</span></p>
                    <hr class="divider">
                    <div style="margin-top:16px"><button class="btn" onclick="showMenu()">Back to Menu</button></div>
                `);
            } catch (e) { showError(e.message); }
        };
    });
}

function doChangePin() {
    promptInput('Enter current PIN:', 'oldPin', (oldPin) => {
        promptInput('Enter new PIN (4-6 digits):', 'newPin', (newPin) => {
            if (newPin.length < 4 || newPin.length > 6) { showError('PIN must be 4-6 digits.'); return; }
            if (!/^\d+$/.test(newPin)) { showError('PIN must contain only digits.'); return; }
            promptInput('Confirm new PIN:', 'confirmPin', (confirmPin) => {
                if (newPin !== confirmPin) { showError('PINs do not match.'); return; }
                if (selectedAccount.changePin(oldPin, newPin)) {
                    currentCard.changePin(oldPin, newPin);
                    render(`
                        <div class="title">PIN Changed</div>
                        <p class="success" style="text-align:center;margin-top:20px">PIN changed successfully!</p>
                        <p class="info" style="text-align:center">Card PIN and Account PIN both updated.</p>
                        <div style="margin-top:20px;text-align:center"><button class="btn" onclick="showMenu()">Back to Menu</button></div>
                    `);
                } else {
                    showError('Incorrect current PIN.');
                }
            });
        });
    });
}

function doMiniStatement() {
    const txns = selectedAccount.getMiniStatement(5);
    let rows = '';
    txns.forEach(t => {
        rows += `<tr><td>${t.getTransactionId()}</td><td>${t.getFormattedDate()}</td><td>${t.transactionType()}</td><td>${t.displayAmount()}</td></tr>`;
    });
    render(`
        <div class="title">Mini Statement</div>
        <p>Account: ${selectedAccount.getAccountNumber()}</p>
        <p>Type: ${selectedAccount.accountType()}</p>
        <hr class="divider">
        <table class="stmt-table">
            <tr><th>TXN ID</th><th>Date</th><th>Type</th><th>Amount</th></tr>
            ${rows || '<tr><td colspan="4" class="info">No transactions found.</td></tr>'}
        </table>
        <hr class="divider">
        <p>Current Balance: <span class="highlight">Rs. ${fmt(selectedAccount.getBalance())}</span></p>
        <div style="margin-top:16px"><button class="btn" onclick="showMenu()">Back to Menu</button></div>
    `);
}

// ==================== HELPERS ====================

function showError(msg) {
    render(`
        <div class="title">Error</div>
        <p class="error" style="margin-top:30px;text-align:center">${msg}</p>
        <div class="center" style="margin-top:30px"><button class="btn primary" onclick="showMenu()">Back to Menu</button></div>
    `);
}

function ejectCard() {
    render(`
        <div class="title">Thank You</div>
        <p class="success" style="text-align:center;margin-top:40px">Thank you for using the ATM!</p>
        <p class="info" style="text-align:center">Please take your card. Goodbye!</p>
    `);
    currentCard = null;
    currentCustomer = null;
    selectedAccount = null;
    cardSlot.textContent = 'INSERT CARD';
    cardSlot.classList.remove('inserted');
    setTimeout(showWelcome, 3000);
}

function ejectAndReset() {
    currentCard = null;
    currentCustomer = null;
    selectedAccount = null;
    cardSlot.textContent = 'INSERT CARD';
    cardSlot.classList.remove('inserted');
    showWelcome();
}

function getAtmCash() {
    return 5000 * 10 + 1000 * 30 + 500 * 20; // Rs. 100,000
}

function showCashDispensing(amount) {
    cashSlot.textContent = 'DISPENSING CASH...';
    cashSlot.classList.add('dispensing');
    setTimeout(() => {
        cashSlot.textContent = 'Rs. ' + fmt(amount) + ' dispensed';
        setTimeout(() => { cashSlot.textContent = ''; cashSlot.classList.remove('dispensing'); }, 3000);
    }, 1500);
}

// ==================== INIT ====================

function init() {
    bank = new Bank('National Bank of Pakistan');

    const ali = bank.createCustomer('Ali Khan', '0301-1234567', 'ali@email.com');
    const sara = bank.createCustomer('Sara Ahmed', '0321-9876543', 'sara@email.com');
    const usman = bank.createCustomer('Usman Ali', '0333-5551234', 'usman@email.com');

    bank.createAccount(ali.getCustomerId(), 'SAVINGS', '1234', 75000);
    bank.createAccount(ali.getCustomerId(), 'CURRENT', '5678', 150000);
    bank.createAccount(sara.getCustomerId(), 'SAVINGS', '4321', 50000);
    bank.createAccount(usman.getCustomerId(), 'CURRENT', '8765', 200000);

    const cardAli = bank.issueCard(ali.getCustomerId(), '1234');
    const cardSara = bank.issueCard(sara.getCustomerId(), '4321');
    const cardUsman = bank.issueCard(usman.getCustomerId(), '8765');

    // Update info panel with actual card numbers
    const cards = document.querySelectorAll('.account-card');
    const cardNums = [cardAli, cardSara, cardUsman];
    cards.forEach((card, i) => {
        const numSpan = document.createElement('span');
        numSpan.textContent = 'Card: ' + cardNums[i].getCardNumber();
        numSpan.style.color = '#00d4ff';
        card.insertBefore(numSpan, card.children[1]);
    });

    showWelcome();
}

init();
