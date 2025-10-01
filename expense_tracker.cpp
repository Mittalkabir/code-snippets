// expense_tracker.cpp
// Simple CLI Expense Tracker - single file C++ project
// Compile: g++ -std=c++17 expense_tracker.cpp -o expense_tracker

#include <bits/stdc++.h>
using namespace std;

struct Record {
    string date;      // YYYY-MM-DD
    double amount;    // positive for income, negative for expense
    string category;
    string note;
};

const string DATA_FILE = "data.csv";

vector<string> split_csv_line(const string &line) {
    vector<string> parts;
    string cur;
    bool in_quote = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"' ) {
            in_quote = !in_quote;
        } else if (c == ',' && !in_quote) {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);
    return parts;
}

string escape_csv(const string &s) {
    if (s.find(',') == string::npos && s.find('"') == string::npos) return s;
    string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += "\"";
    return out;
}

vector<Record> load_data() {
    vector<Record> records;
    ifstream fin(DATA_FILE);
    if (!fin) return records;
    string line;
    // skip header if present
    if (!getline(fin, line)) return records;
    // If header is not the header (e.g., real record), we still parse it:
    if (line.find("date,amount,category,note") == string::npos) {
        // treat it as a data line
        auto parts = split_csv_line(line);
        if (parts.size() >= 4) {
            records.push_back({parts[0], stod(parts[1]), parts[2], parts[3]});
        }
    }
    while (getline(fin, line)) {
        if (line.size() == 0) continue;
        auto parts = split_csv_line(line);
        if (parts.size() < 4) continue;
        Record r{parts[0], 0.0, parts[2], parts[3]};
        try { r.amount = stod(parts[1]); } catch(...) { r.amount = 0.0; }
        records.push_back(r);
    }
    return records;
}

void save_data(const vector<Record> &records) {
    ofstream fout(DATA_FILE);
    fout << "date,amount,category,note\n";
    for (const auto &r : records) {
        fout << escape_csv(r.date) << "," << r.amount << "," << escape_csv(r.category) << "," << escape_csv(r.note) << "\n";
    }
}

void add_record(vector<Record> &records) {
    Record r;
    cout << "Enter date (YYYY-MM-DD) [leave blank for today]: ";
    string s;
    getline(cin, s);
    if (s.empty()) {
        // get today's date
        time_t t = time(nullptr);
        tm *lt = localtime(&t);
        char buf[11];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
        r.date = string(buf);
    } else r.date = s;

    cout << "Enter amount (positive for income, negative for expense): ";
    string amt;
    getline(cin, amt);
    try { r.amount = stod(amt); } catch(...) { cout << "Invalid amount. Aborting add.\n"; return; }

    cout << "Enter category (e.g., Food, Salary, Rent): ";
    getline(cin, r.category);
    if (r.category.empty()) r.category = "Uncategorized";

    cout << "Enter note (optional): ";
    getline(cin, r.note);

    records.push_back(r);
    save_data(records);
    cout << "Record added.\n";
}

void list_records(const vector<Record> &records) {
    if (records.empty()) { cout << "No records found.\n"; return; }
    cout << left << setw(12) << "Date" << setw(12) << "Amount" << setw(15) << "Category" << "Note\n";
    cout << string(60,'-') << "\n";
    for (const auto &r : records) {
        cout << setw(12) << r.date << setw(12) << r.amount << setw(15) << r.category << r.note << "\n";
    }
}

vector<Record> filter_by_date(const vector<Record> &recs, const string &from, const string &to) {
    vector<Record> out;
    for (const auto &r : recs) {
        if (!from.empty() && r.date < from) continue;
        if (!to.empty() && r.date > to) continue;
        out.push_back(r);
    }
    return out;
}

vector<Record> filter_by_category(const vector<Record> &recs, const string &cat) {
    vector<Record> out;
    for (const auto &r : recs) if (r.category == cat) out.push_back(r);
    return out;
}

void show_summary(const vector<Record> &records) {
    double income = 0, expense = 0;
    for (const auto &r : records) {
        if (r.amount >= 0) income += r.amount;
        else expense += -r.amount;
    }
    cout << fixed << setprecision(2);
    cout << "Total income : " << income << "\n";
    cout << "Total expense: " << expense << "\n";
    cout << "Net balance  : " << (income - expense) << "\n";
}

void monthly_summary(const vector<Record> &records) {
    // map YYYY-MM -> pair(income, expense)
    map<string, pair<double,double>> mm;
    for (const auto &r : records) {
        if (r.date.size() < 7) continue;
        string ym = r.date.substr(0,7); // YYYY-MM
        if (r.amount >= 0) mm[ym].first += r.amount;
        else mm[ym].second += -r.amount;
    }
    cout << left << setw(10) << "Month" << setw(12) << "Income" << setw(12) << "Expense" << "Net\n";
    cout << string(50,'-') << "\n";
    for (auto &p : mm) {
        double inc = p.second.first, exp = p.second.second;
        cout << setw(10) << p.first << setw(12) << inc << setw(12) << exp << (inc - exp) << "\n";
    }
}

void export_to_csv(const vector<Record> &records) {
    cout << "Enter filename to export to (e.g., export.csv): ";
    string name;
    getline(cin, name);
    if (name.empty()) { cout << "Invalid filename.\n"; return; }
    ofstream fout(name);
    fout << "date,amount,category,note\n";
    for (const auto &r : records) {
        fout << escape_csv(r.date) << "," << r.amount << "," << escape_csv(r.category) << "," << escape_csv(r.note) << "\n";
    }
    cout << "Exported " << records.size() << " records to " << name << "\n";
}

void prompt_filters_and_list(const vector<Record> &all) {
    cout << "Filter by date range? (y/n): ";
    string choice;
    getline(cin, choice);
    string from, to;
    if (!choice.empty() && (choice[0]=='y' || choice[0]=='Y')) {
        cout << "From (YYYY-MM-DD) [leave blank for no lower bound]: ";
        getline(cin, from);
        cout << "To (YYYY-MM-DD)   [leave blank for no upper bound]: ";
        getline(cin, to);
    }
    auto filtered = filter_by_date(all, from, to);

    cout << "Filter by category? (y/n): ";
    getline(cin, choice);
    if (!choice.empty() && (choice[0]=='y' || choice[0]=='Y')) {
        cout << "Enter category: ";
        string cat; getline(cin, cat);
        if (!cat.empty()) filtered = filter_by_category(filtered, cat);
    }

    list_records(filtered);
    cout << "\nShow summary for these records? (y/n): ";
    getline(cin, choice);
