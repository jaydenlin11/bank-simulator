# Bank Simulator 🏦💰

## Overview 📌

I built this banking simulator (`bank`) to tackle the challenge of real-time wire transfers. It processes customer registrations, handles transactions, and answers queries—all with an eye on efficiency. The idea was to create a system that stays responsive over time, using smart data structures to manage a growing transaction log. It’s been a great way to flex my C++ skills and figure out what works best for different tasks.


## Features ✨

- ⚡ **Real-time** transaction processing with fraud checks.
- 📥 **Input:** Registration file and commands via stdin.
- ⚙️ **Flags:** `--help` (usage), `--file` (registrations), `--verbose` (logs).

### Usage 🖥️
```bash
./bank --file regs.txt < commands.txt > output.txt
./bank -vf spec-reg.txt < spec-commands.txt
```

## Banking Logic 🔄

The simulator mimics a **real-time gross settlement system**—credits and debits apply as they arrive, not batched. Here’s how it ticks:

- 📝 **Registrations:** Load users with a timestamp, ID, PIN, and balance  
  _(e.g., `23:05:17:09:30:00|alice|123456|1000`)_.  
- 🏦 **Commands:** Two phases—operations (e.g., login, transactions) and queries (e.g., history)—split by `$$$`.

### Operations ⚙️
- 🔑 **Login:** `login alice 123456 192.168.1.1`  
  ✅ Authenticates users, tracks valid IPs. _(Verbose: “User alice logged in.”)_  
- 🚪 **Logout:** `out alice 192.168.1.1`  
  ✅ Ends session, drops IP. _(Verbose: “User alice logged out.”)_  
- 💵 **Balance Check:** `balance alice 192.168.1.1`  
  ✅ Shows funds if logged in, using the latest transaction timestamp.  
- 💸 **Place Transaction:**  
  ```
  place 23:05:17:09:30:00 192.168.1.1 alice bob 500 23:05:20:09:30:00 o
  ```
  ✅ Schedules a transfer with the following checks:
  - Sender ≠ recipient.
  - Execution date ≤ 3 days from timestamp.
  - Both users exist and are registered by execution.
  - Sender’s logged in with a valid IP.
  - Timestamps don’t decrease.

### Transaction Flow 🔄
- 📌 **Queueing:** A `std::priority_queue` sorts transactions by execution date (earliest first).  
- ⚡ **Execution:** At execution time, funds are checked:
  - Sender covers amount + fee (`o`) or amount + half fee (`s`).
  - Recipient covers half fee (`s`).
  - **Fee Calculation:**  
    - `min(450, max(10, amount / 100))`  
    - 25% discount if sender’s >5 years old.  
  - ❌ **Insufficient Funds?**  
    - `"Insufficient funds to process transaction <ID>."` _(Transaction is discarded.)_  

**Example:** Alice (10-year member) sends Bob **$1000**, shared fee (`s`):  
- Fee = max(10, 1000/100) = **10** → Discounted to **7**  
- Alice pays **$1004** (1000 + 4), Bob pays **$3**, receives **$1000**.  
- _(Verbose: “Transaction 1 executed at <date>: $1000 from alice to bob.”)_

### Queries 🔍
- 📜 **List Transactions:** `l x y` _(Shows transactions in [x, y)_)  
- 💰 **Revenue Report:** `r x y` _(Sums fees in [x, y)_)  
- 📖 **History:** `h alice` _(Latest 10 in/out transactions)_  
- 📊 **Daily Summary:** `s timestamp` _(Shows day's transactions & fees)_  


## Input 📂

📄 **Registrations:**  
```bash
23:05:17:09:30:00|alice|123456|1000
```
📄 **Commands:**  
```bash
login alice 123456 192.168.1.1
place 23:05:17:09:30:00 192.168.1.1 alice bob 500 23:05:20:09:30:00 o
$$$
h alice
```

## Output 📤

🔍 **Verbose Mode:**  
- `"Transaction 1 placed at <time>: $500 from alice to bob at <date>."`  
- `"Transaction 1 executed at <date>: $500 from alice to bob."`  

✅ **Standard Output:**  
- `"As of <time>, alice has a balance of $490."`  
- `"2: alice sent 500 dollars to bob at <time>."`  


## Build 🏗️

- 🔧 **Requirements:** `g++`, STL, `-O3` in `Makefile`  
- 📂 **Files:** `bank.cpp`, `Makefile`, `test-*-reg.txt`, `test-*-commands.txt`  
- 🚀 **Run:** `make -R -r`  



## Takeaways 📌

- ✅ **Chose `priority_queue`** for transaction scheduling.  
- ✅ **Balanced runtime vs. storage** with STL.  
- ✅ **Nailed real-time logic** with fraud detection twists.  

📆 **Updated:** March 16, 2025  

