// Project Identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <queue>
#include <functional>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cmath>
using namespace std;

class Bank{
    private:
        bool verbose = false;
        string filename;
        uint64_t current_timestamp = 0;
        uint32_t trade_id = 0;
        bool ts_updated = false;
        bool trade_status = true;

        struct Transaction{
            uint32_t trade_id;
            uint32_t amount;
            mutable uint32_t fee;
            string sender;
            string recipient;
            uint64_t exec_date;
            char osstatus;
            Transaction() : trade_id(0), amount(0), fee(0), sender(""), recipient(""), exec_date(0), osstatus(' ') {}
            Transaction(uint32_t tid, uint32_t amt, uint32_t fe, string sndr, string rcpt, uint64_t edate, char os) : trade_id(tid), amount(amt), fee(fe), sender(sndr), recipient(rcpt), exec_date(edate), osstatus(os) {}
        };

        struct sortTran{
            bool operator()(const Transaction &left, const Transaction &right) const{
                if(left.exec_date==right.exec_date){
                    return left.trade_id > right.trade_id;
                }
                return left.exec_date>right.exec_date;
            }
        };

        struct Customer{
            uint64_t reg_timestamp;
            string user_id;
            uint32_t pin;
            uint32_t balance;
            uint32_t trans_num = 0;
            unordered_set<string> ips;
            vector<Transaction> incoming;
            vector<Transaction> outgoing;
            Customer() : reg_timestamp(0), user_id(""), pin(0), balance(0) {}
            Customer(uint64_t reg, const std::string& id, uint32_t p, uint32_t bal) : reg_timestamp(reg), user_id(id), pin(p), balance(bal){}
        };

        unordered_map<string, Customer> registration;
        priority_queue<Transaction, vector<Transaction>, sortTran> transactionQueue;
        vector<Transaction>pastTransaction;

        void printHelp(char *argv[]) {
            cout << "Usage: " << argv[0] << " [--file/-f filename] [--verbose/-v] < infile > outfile | -h\n";
            cout << "This program is to help you manage bank.\n";
        } 

        void runTransaction(){  
            // uint32_t amount;
            // uint32_t fee;
            uint32_t sender_fee;
            uint32_t recipient_fee;
            while(!transactionQueue.empty()){
                uint32_t amount;
                uint32_t fee;
                Transaction transaction  = transactionQueue.top();
                if(transaction.exec_date>current_timestamp){
                    break;
                }
                amount = transaction.amount;
                fee = (amount* 1) / 100;
                Customer &sender = registration.at(transaction.sender);
                Customer &recipient = registration.at(transaction.recipient);
                if((transaction.exec_date - sender.reg_timestamp)>50000000000){
                    fee = (fee * 3) / 4;
                    if(fee>337){
                        fee = 337;
                    }
                    else if(fee<7){
                        fee = 7;
                    }
                }
                else{
                    if(fee>450){
                        fee = 450;
                    }
                    else if(fee<10){
                        fee = 10;
                    }
                }
                if(transaction.osstatus=='s'){
                    if(fee % 2 != 0){
                        sender_fee = fee/2+1;
                    }
                    else{
                        sender_fee = fee/2;
                    }
                    recipient_fee = fee-sender_fee;
                    sender_fee += amount;
                    if(sender_fee>sender.balance||recipient_fee>recipient.balance){
                        if(verbose){
                            cout << "Insufficient funds to process transaction "<<transaction.trade_id<<".\n";
                        }
                        transactionQueue.pop();
                        continue;
                    }
                }
                else{
                    sender_fee = fee+amount;
                    recipient_fee = 0;
                    if(sender_fee>sender.balance){
                        if(verbose){
                            cout << "Insufficient funds to process transaction "<<transaction.trade_id<<".\n";
                        }
                        transactionQueue.pop();
                        continue;
                    }
                }
                sender.balance -= sender_fee;
                recipient.balance = recipient.balance - recipient_fee + amount;
                if(verbose){
                    cout << "Transaction "<<transaction.trade_id<<" executed at "<<transaction.exec_date<<": $"<<amount<<" from "<<transaction.sender<<" to "<<transaction.recipient<<".\n";
                }
                transactionQueue.top().fee = fee;
                pastTransaction.push_back(transactionQueue.top());
                registration.at(transaction.sender).outgoing.push_back(transaction);
                registration.at(transaction.recipient).incoming.push_back(transaction);
                //might be able to save some memory here by only saving 10 transacton into the pq;
                transactionQueue.pop();
            }
        }

        void parseTimestamp(uint64_t &ts){
            ts = 0;
            string t;
            for(int i = 5; i>0;i--){
                getline(cin, t, ':');
                ts+= static_cast<uint64_t>(stoi(t)*pow(100LL,i));
            }
            cin >> t;
            ts += static_cast<uint64_t>(stoi(t));
        }

    public:
        Bank (){};
        void getMode(int argc, char * argv[]) {
            opterr = false; // Let us handle all error output for command line options
            int choice;
            int index = 0;
            option long_options[] = {
                {"help", no_argument, 0, 'h'},
                {"file", required_argument, 0, 'f'},
                {"verbose", no_argument, 0, 'v'},
                { nullptr, 0,                 nullptr,  '\0'},
            }; 
            while ((choice = getopt_long(argc, argv, "hf:v", long_options, &index)) != -1) {
                switch (choice) {
                    case 'h':
                        printHelp(argv);
                        exit(0);
                    case 'f':
                        filename = optarg;
                        break;
                    case 'v':
                        verbose = true;
                        break;
                    default:
                        cerr << "Usage: " << argv[0] << " [-v|--verbose] [-m|--median] [-i|--trader_info] [-t|--time_travelers]" << endl;
                        exit(EXIT_FAILURE);
                }
            }
        }
        void readFile(){
            if (!filename.empty()) {
                ifstream file(filename);
                if (!file) {
                    cerr << "Error: Could not open file " << filename << endl;
                    exit(EXIT_FAILURE);
                }

                string input;
                string username;

                while(getline(file, input, ':')){
                    uint64_t timeStamp = 0;
                    timeStamp+= static_cast<uint64_t>(stoi(input))*10000000000LL;
                    for(int i = 4; i>0;i--){
                        getline(file, input, ':');
                        timeStamp+= static_cast<uint64_t>(stoi(input)*pow(100LL,i));
                    }
                    getline(file, input, '|');
                    timeStamp+= static_cast<uint64_t>(stoi(input));
                    getline(file, username, '|');
                    getline(file, input, '|');
                    uint32_t pin = static_cast<uint32_t>(stoi(input));
                    getline(file, input, '\n');
                    uint32_t balance = static_cast<uint32_t>(stoi(input));
                    Customer customer(timeStamp, username, pin, balance);
                    registration[username] = customer;
                }     
                file.close();
            } 
            else {
                cerr << "Error: No file provided. Use -f or --file to specify a file." << endl;
            }  
        }

        void readCommands(){
            char header;
            while(trade_status&&cin>>header){
                readCommand(header);
            }
        }
        void readCommand(char header){
            // char header;
            string comment;
            string user_id;
            uint32_t pin;
            string ip;
            string sender;
            string recipient;
            uint32_t amount;
            uint64_t exec_date;
            char osstatus;
            if(header =='#'){
                getline(cin, comment);
            }
            else if(header=='l'){
                cin >> comment;
                cin >> user_id;
                cin >> pin;   
                cin >> ip;
                if (registration.find(user_id) != registration.end()&&registration.at(user_id).pin==pin) {
                    registration[user_id].ips.insert(ip);
                    if(verbose){
                        cout << "User "<<user_id<<" logged in.\n";
                    }
                }
                else {
                    if(verbose){
                        cout << "Login failed for "<<user_id<<".\n";
                    }
                }
            }
            else if(header=='o'){
                cin >> comment;
                cin >> user_id;
                cin >> ip;  
                if (registration.find(user_id) != registration.end()&&registration.at(user_id).ips.erase(ip)) {
                    if(verbose){
                        cout << "User "<<user_id<<" logged out.\n";
                    }
                }
                else {
                    if(verbose){
                        cout << "Logout failed for "<<user_id<<".\n";
                    }
                }
            }
            else if(header=='b'){
                cin >> comment;
                cin >> user_id;
                cin >> ip;
                if (registration.find(user_id) == registration.end()) {
                    if(verbose){
                        cout << "Account "<<user_id<<" does not exist.\n";
                    }
                }
                else if(registration.at(user_id).ips.empty()){
                    if(verbose){
                        cout << "Account "<<user_id<<" is not logged in.\n";
                    }
                }
                else if(registration.at(user_id).ips.find(ip)==registration.at(user_id).ips.end()) {
                    if(verbose){
                        cout << "Fraudulent transaction detected, aborting request.\n";
                    }
                }
                else{
                    if(ts_updated){
                        cout << "As of "<< current_timestamp <<", "<<user_id<<" has a balance of $"<<registration.at(user_id).balance <<".\n";
                    }
                    else{
                        cout << "As of "<< registration.at(user_id).reg_timestamp << ", "<<user_id<<" has a balance of $"<<registration.at(user_id).balance <<".\n";
                    }
                }
            }
            else if(header=='p'){
                cin >> comment;
                uint64_t x;
                parseTimestamp(x);
                if(x<current_timestamp){
                    cerr <<"Invalid decreasing timestamp in 'place' command.\n";
                    exit(1);
                }
                current_timestamp = x;
                ts_updated = true;
                cin >> ip;
                cin >> sender;
                cin >> recipient;
                cin >> amount;
                parseTimestamp(exec_date);
                if(exec_date<current_timestamp){
                    cerr << "You cannot have an execution date before the current timestamp.\n";
                    exit(1);
                }
                cin >> osstatus;
                if(sender==recipient){
                    if(verbose){
                        cout << "Self transactions are not allowed.\n";
                    }
                }
                else if(exec_date-current_timestamp>3000000){
                    if(verbose){
                        cout << "Select a time up to three days in the future.\n";
                    }
                }
                else if(registration.find(sender) == registration.end()) {
                    if(verbose){
                        cout << "Sender "<<sender<<" does not exist.\n";
                    }
                }
                else if(registration.find(recipient) == registration.end()) {
                    if(verbose){
                        cout << "Recipient "<<recipient<<" does not exist.\n";
                    }
                }
                else if(registration.at(sender).reg_timestamp > exec_date||registration.at(recipient).reg_timestamp > exec_date) {
                    if(verbose){
                        cout << "At the time of execution, sender and/or recipient have not registered.\n";
                    }
                }
                else if(registration.at(sender).ips.empty()){
                    if(verbose){
                        cout << "Sender "<<sender<<" is not logged in.\n";
                    }
                }
                else if(registration.at(sender).ips.find(ip)==registration.at(sender).ips.end()) {
                    if(verbose){
                        cout << "Fraudulent transaction detected, aborting request.\n";
                    }
                }
                else{
                    if(!transactionQueue.empty()){
                        runTransaction();
                    }
                    if(verbose){
                        cout << "Transaction "<<trade_id<<" placed at "<<current_timestamp<<": $"<<amount<<" from " <<sender<<" to "<<recipient<<" at "<<exec_date<<".\n";
                    }
                    Transaction transaction(trade_id, amount, 0, sender, recipient, exec_date, osstatus);
                    transactionQueue.push(transaction);
                    trade_id+=1;
                }
            }
            else if(header=='$'){
                cin >> comment;
                current_timestamp = UINT64_MAX;
                runTransaction();
                trade_status = false;
            }
        }
        void queryList(){
            char header;
            uint64_t x;
            uint64_t y;
            while(cin>>header){
                if(header == 'l'){
                    parseTimestamp(x);
                    parseTimestamp(y);
                    if(y==x){
                        cout << "List Transactions requires a non-empty time interval.\n";
                    }
                    else{
                        int counter = 0;
                        for (const auto& transaction : pastTransaction) {
                            if(transaction.exec_date>=y){
                                break;
                            }
                            else if(transaction.exec_date>=x){
                                cout << transaction.trade_id <<": "<<transaction.sender<<" sent ";
                                if(transaction.amount==1){
                                    cout <<"1 dollar ";
                                }
                                else{
                                    cout <<transaction.amount<< " dollars ";
                                }
                                cout<<"to "<<transaction.recipient<<" at "<<transaction.exec_date<<".\n";
                                counter++;
                            }
                        }
                        if(counter==1){
                            cout << "There was 1 transaction that was placed between time "<<x<<" to "<<y<<".\n";
                        }
                        else{
                            cout << "There were "<<counter<<" transactions that were placed between time "<<x<<" to "<<y<<".\n";
                        }
                    }
                }
                else if(header == 'r'){
                    parseTimestamp(x);
                    parseTimestamp(y);
                    if(y==x){
                        cout << "Bank Revenue requires a non-empty time interval.\n";
                    }
                    else{
                        uint32_t total = 0;
                        for (const auto& transaction : pastTransaction) {
                            if(transaction.exec_date>=y){
                                break;
                            }
                            else if(transaction.exec_date>=x){
                                total+=transaction.fee;
                            }
                        }
                        cout<<"281Bank has collected "<<total<<" dollars in fees over";
                        uint64_t period = y-x;
                        uint64_t seconds = period % 100;
                        period /= 100;
                        uint64_t minutes = period % 100;
                        period /= 100;
                        uint64_t hours = period % 100;
                        period /= 100;
                        uint64_t days = period % 100;
                        period /= 100;
                        uint64_t months = period % 100;
                        period /= 100;
                        uint64_t years = period % 100;
                        if(years!=0){
                            if(years==1){
                                cout << " 1 year";
                            }
                            else{
                                cout << " " << years << " years";
                            }
                        }
                        if(months!=0){
                            if(months==1){
                                cout << " 1 month";
                            }
                            else{
                                cout << " " << months << " months";
                            }
                        }
                        if(days!=0){
                            if(days==1){
                                cout << " 1 day";
                            }
                            else{
                                cout << " " << days << " days";
                            }
                        }
                        if(hours!=0){
                            if(hours==1){
                                cout << " 1 hour";
                            }
                            else{
                                cout << " " << hours << " hours";
                            }
                        }
                        if(minutes!=0){
                            if(minutes==1){
                                cout << " 1 minute";
                            }
                            else{
                                cout << " " << minutes << " minutes";
                            }
                        }
                        if(seconds!=0){
                            if(seconds==1){
                                cout << " 1 second";
                            }
                            else{
                                cout << " " << seconds << " seconds";
                            }
                        }
                        cout << ".\n";
                    }
                }
                else if(header == 'h'){
                    string user_id;
                    cin >> user_id;
                    if(registration.find(user_id)==registration.end()){
                        cout << "User "<<user_id<<" does not exist.\n";
                        continue;
                    }
                    Customer customer = registration.at(user_id);
                    size_t incoming_size = customer.incoming.size();
                    size_t outgoing_size = customer.outgoing.size();
                    cout << "Customer "<<user_id<<" account summary:\n";
                    cout << "Balance: $" << customer.balance << "\n";
                    cout << "Total # of transactions: " << incoming_size + outgoing_size << "\n";
                    cout << "Incoming " << incoming_size << ":\n";
                    if(incoming_size<=10){
                        for(const auto& transaction : customer.incoming){
                            cout << transaction.trade_id <<": "<<transaction.sender<<" sent ";
                            if(transaction.amount==1){
                                cout <<1<<" dollar "; 
                            }
                            else{
                                cout <<transaction.amount<< " dollars ";
                            }
                            cout << "to "<<transaction.recipient<<" at "<<transaction.exec_date<<".\n";
                        }
                    }
                    else{
                        for(size_t i = incoming_size-10; i<incoming_size; i++){
                            Transaction &transaction = customer.incoming[i];
                            cout << transaction.trade_id <<": "<<transaction.sender<<" sent ";
                            if(transaction.amount==1){
                                cout <<1<<" dollar ";
                            }
                            else{
                                cout <<transaction.amount<< " dollars ";
                            }
                            cout << "to "<<transaction.recipient<<" at "<<transaction.exec_date<<".\n";
                        }
                    }
                    cout << "Outgoing " << outgoing_size << ":\n";
                    if(outgoing_size<=10){
                        for(const auto& transaction : customer.outgoing){
                            cout << transaction.trade_id <<": "<<transaction.sender<<" sent ";
                            if(transaction.amount==1){
                                cout <<1<<" dollar "; 
                            }
                            else{
                                cout <<transaction.amount<< " dollars ";
                            }
                            cout << "to "<<transaction.recipient<<" at "<<transaction.exec_date<<".\n";
                        }
                    }
                    else{
                        for(size_t i = outgoing_size-10; i<outgoing_size; i++){
                            Transaction &transaction = customer.outgoing[i];
                            cout << transaction.trade_id <<": "<<transaction.sender<<" sent ";
                            if(transaction.amount==1){
                                cout <<1<<" dollar ";
                            }
                            else{
                                cout <<transaction.amount<< " dollars ";
                            }
                            cout << "to "<<transaction.recipient<<" at "<<transaction.exec_date<<".\n";
                        }
                    }
                }
                else if(header == 's'){
                    parseTimestamp(x);
                    x = (x/1000000)*1000000;
                    uint64_t y = x+1000000;
                    cout << "Summary of ["<<x<<", "<<y<<"):\n";
                    int counter = 0;
                    uint64_t fee = 0;
                    for (const auto& transaction : pastTransaction) {
                        if(transaction.exec_date>=y){
                            break;
                        }
                        else if(transaction.exec_date>=x){
                            cout << transaction.trade_id <<": "<<transaction.sender<<" sent ";
                            if(transaction.amount==1){
                                cout <<"1 dollar ";
                            }
                            else{
                                cout <<transaction.amount<< " dollars ";
                            }
                            cout<<"to "<<transaction.recipient<<" at "<<transaction.exec_date<<".\n";
                            fee += transaction.fee;
                            counter++;
                        }
                        
                    }
                    if(counter==1){
                        cout << "There was a total of 1 transaction, 281Bank has collected "<<fee<<" dollars in fees.\n";
                    }
                    else{
                        cout << "There were a total of "<<counter<<" transactions, 281Bank has collected "<<fee<<" dollars in fees.\n";
                    }
                }
            }
        }
};

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    Bank bank;
    bank.getMode(argc, argv);
    bank.readFile();
    bank.readCommands();
    bank.queryList();
}