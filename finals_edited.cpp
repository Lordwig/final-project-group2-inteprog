#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <limits>
#include <conio.h>
#include <atomic>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstring>
#include <climits>
#include <string>

using namespace std;

// ====================== Logger ======================
class Logger {
private:
    static Logger* instance;
    ofstream logFile;
    atomic<unsigned long long> nextLogId;

    Logger() : nextLogId(1) {
        logFile.open("pharmacy_log.txt", ios::app);
        if (!logFile.is_open()) throw runtime_error("Cannot open log file");
        
        // Find the highest existing log ID
        ifstream inFile("pharmacy_log.txt");
        if (inFile.is_open()) {
            string line;
            unsigned long long maxId = 0;
            while (getline(inFile, line)) {
                size_t pos = line.find("ID:");
                if (pos != string::npos) {
                    try {
                        unsigned long long id = stoull(line.substr(pos + 3));
                        if (id > maxId) maxId = id;
                    } catch (...) {
                        // Ignore conversion errors
                    }
                }
            }
            nextLogId = maxId + 1;
            inFile.close();
        }
    }

public:
    static Logger* getInstance() {
        if (!instance) instance = new Logger();
        return instance;
    }

    void log(const string& functionName, const string& message) {
        unsigned long long id = nextLogId++;
        time_t now = time(nullptr);
        tm tm = *localtime(&now);
        
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &tm);
        
        logFile << "[" << timeStr << "] "
                << "ID:" << id << " "
                << functionName << " - " << message << endl;
    }

    static void shutdown() {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    ~Logger() {
        if (logFile.is_open()) logFile.close();
    }
};

Logger* Logger::instance = nullptr;
    
#define LOG_FUNCTION() Logger::getInstance()->log(__FUNCTION__, "Entered")
#define LOG_MESSAGE(msg) Logger::getInstance()->log(__FUNCTION__, msg)

// ====================== Exceptions ======================
class PharmacyException : public runtime_error {
public:
    PharmacyException(const string& msg) : runtime_error(msg) {
        LOG_MESSAGE("Exception: " + msg);
    }
};

class AuthenticationException : public PharmacyException {
public:
    AuthenticationException() : PharmacyException("Invalid username or password") {}
};

class AuthorizationException : public PharmacyException {
public:
    AuthorizationException() : PharmacyException("Insufficient privileges") {}
};

class DataValidationException : public PharmacyException {
public:
    DataValidationException(const string& msg) : PharmacyException("Validation failed: " + msg) {}
};

class DatabaseFullException : public PharmacyException {
public:
    DatabaseFullException(const string& entity) 
        : PharmacyException("Maximum " + entity + " capacity reached") {}
};

class NotFoundException : public PharmacyException {
public:
    NotFoundException(const string& entity) 
        : PharmacyException(entity + " not found") {}
};

// ====================== Enums ======================
enum class UserRole {
    ADMIN,
    PHARMACIST
};

enum class PaymentType {
    CASH,
    GCASH,
    PAYMAYA
};

// ====================== Date Class ======================
class Date {
private:
    int day, month, year;

    bool isLeapYear() const {
        return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
    }

    int daysInMonth() const {
        if (month == 2) return isLeapYear() ? 29 : 28;
        if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
        return 31;
    }

public:
    Date(int d, int m, int y) : day(d), month(m), year(y) {
        LOG_FUNCTION();
        if (year < 1900 || year > 2100) throw DataValidationException("Invalid year");
        if (month < 1 || month > 12) throw DataValidationException("Invalid month");
        if (day < 1 || day > daysInMonth()) throw DataValidationException("Invalid day");
    }

    string toString() const {
        char buffer[11];
        snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d", day, month, year);
        return string(buffer);
    }

    bool isExpired() const {
        LOG_FUNCTION();
        time_t now = time(nullptr);
        tm* current = localtime(&now);
        if (year < current->tm_year + 1900) return true;
        if (year == current->tm_year + 1900 && month < current->tm_mon + 1) return true;
        if (year == current->tm_year + 1900 && month == current->tm_mon + 1 && day < current->tm_mday) return true;
        return false;
    }

    int getDay() const { return day; }
    int getMonth() const { return month; }
    int getYear() const { return year; }
};

// ====================== Core Classes ======================
class User {
private:
    string username;
    string password;
    UserRole role;

public:
    User(const string& uname, const string& pwd, UserRole r) 
        : username(uname), password(pwd), role(r) {
        LOG_FUNCTION();
        if (username.empty() || password.empty()) {
            throw DataValidationException("Username/password cannot be empty");
        }
    }

    bool authenticate(const string& uname, const string& pwd) const {
        LOG_FUNCTION();
        return username == uname && password == pwd;
    }

    UserRole getRole() const { return role; }
    const string& getUsername() const { return username; }
};

class Medicine {
private:
    int id;
    string name;
    string description;
    int quantity;
    double price;
    Date expiryDate;
    bool isControlled;

public:
    Medicine(int id, const string& name, const string& desc, int qty, 
             double price, const Date& expiry, bool controlled)
        : id(id), name(name), description(desc), quantity(qty), 
          price(price), expiryDate(expiry), isControlled(controlled) {
        LOG_FUNCTION();
        validate();
    }

    void validate() const {
        if (name.empty()) throw DataValidationException("Medicine name required");
        if (quantity < 0) throw DataValidationException("Quantity cannot be negative");
        if (price < 0) throw DataValidationException("Price cannot be negative");
    }

    void reduceQuantity(int amount) {
        LOG_FUNCTION();
        if (amount > quantity) throw DataValidationException("Insufficient stock");
        quantity -= amount;
    }

    void print() const {
        cout << "ID: " << id << "\n"
             << "Name: " << name << "\n"
             << "Description: " << description << "\n"
             << "Quantity: " << quantity << "\n"
             << "Price: P" << fixed << setprecision(2) << price << "\n"
             << "Expiry Date: " << expiryDate.toString() << "\n"
             << "Controlled: " << (isControlled ? "Yes" : "No") << "\n";
    }

    // Getters
    int getId() const { return id; }
    const string& getName() const { return name; }
    const string& getDescription() const { return description; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }
    const Date& getExpiryDate() const { return expiryDate; }
    bool getIsControlled() const { return isControlled; }

    // Setters with validation
    void setName(const string& newName) {
        if (newName.empty()) throw DataValidationException("Name cannot be empty");
        name = newName;
    }

    void setDescription(const string& desc) {
        description = desc;
    }

    void setQuantity(int newQty) {
        if (newQty < 0) throw DataValidationException("Quantity cannot be negative");
        quantity = newQty;
    }

    void setPrice(double newPrice) {
        if (newPrice < 0) throw DataValidationException("Price cannot be negative");
        price = newPrice;
    }

    void setExpiryDate(const Date& date) {
        expiryDate = date;
    }

    void setIsControlled(bool controlled) {
        isControlled = controlled;
    }
};

class PrescriptionItem {
private:
    int medicineId;
    int quantity;

public:
    PrescriptionItem(int medId, int qty) : medicineId(medId), quantity(qty) {
        LOG_FUNCTION();
        if (quantity <= 0) throw DataValidationException("Quantity must be positive");
    }

    int getMedicineId() const { return medicineId; }
    int getQuantity() const { return quantity; }
};

class Prescription {
private:
    int id;
    string patientName;
    string doctorName;
    Date date;
    vector<PrescriptionItem> items;
    bool isFilled;

public:
    Prescription(int id, const string& patient, const string& doctor, const Date& date)
        : id(id), patientName(patient), doctorName(doctor), date(date), isFilled(false) {
        LOG_FUNCTION();
        validate();
    }

    void validate() const {
        if (patientName.empty()) throw DataValidationException("Patient name required");
        if (doctorName.empty()) throw DataValidationException("Doctor name required");
    }

    void addItem(const PrescriptionItem& item) {
        LOG_FUNCTION();
        items.push_back(item);
    }

    void fill() {
        LOG_FUNCTION();
        isFilled = true;
    }

    void print() const {
        cout << "Prescription ID: " << id << "\n"
             << "Patient: " << patientName << "\n"
             << "Doctor: " << doctorName << "\n"
             << "Date: " << date.toString() << "\n"
             << "Status: " << (isFilled ? "Filled" : "Pending") << "\n"
             << "Medicines:\n";
        
        for (const auto& item : items) {
            cout << "  - Medicine ID: " << item.getMedicineId() 
                 << ", Quantity: " << item.getQuantity() << "\n";
        }
    }

    // Getters
    int getId() const { return id; }
    const string& getPatientName() const { return patientName; }
    const string& getDoctorName() const { return doctorName; }
    const Date& getDate() const { return date; }
    bool getIsFilled() const { return isFilled; }
    const vector<PrescriptionItem>& getItems() const { return items; }

    // Setters
    void setPatientName(const string& name) {
        if (name.empty()) throw DataValidationException("Patient name cannot be empty");
        patientName = name;
    }

    void setDoctorName(const string& name) {
        if (name.empty()) throw DataValidationException("Doctor name cannot be empty");
        doctorName = name;
    }

    void setDate(const Date& newDate) {
        date = newDate;
    }
};

class Transaction {
private:
    int id;
    Date date;
    int prescriptionId;
    double totalAmount;
    PaymentType paymentType;
    string paymentDetails;

public:
    Transaction(int id, const Date& date, int presId, double amount, 
                PaymentType type, const string& details)
        : id(id), date(date), prescriptionId(presId), totalAmount(amount),
          paymentType(type), paymentDetails(details) {
        LOG_FUNCTION();
        validate();
    }

    void validate() const {
        if (totalAmount < 0) throw DataValidationException("Amount cannot be negative");
        if (paymentDetails.empty() && paymentType != PaymentType::CASH) {
            throw DataValidationException("Payment details required");
        }
    }

    void print() const {
        cout << "Transaction ID: " << id << "\n"
             << "Date: " << date.toString() << "\n"
             << "Prescription ID: " << prescriptionId << "\n"
             << "Total Amount: P" << fixed << setprecision(2) << totalAmount << "\n"
             << "Payment Type: ";
        
        switch (paymentType) {
            case PaymentType::CASH: cout << "Cash"; break;
            case PaymentType::GCASH: cout << "GCash"; break;
            case PaymentType::PAYMAYA: cout << "PayMaya"; break;
        }
        cout << "\nPayment Details: " << paymentDetails << "\n";
    }

    // Getters
    int getId() const { return id; }
    const Date& getDate() const { return date; }
    int getPrescriptionId() const { return prescriptionId; }
    double getTotalAmount() const { return totalAmount; }
    PaymentType getPaymentType() const { return paymentType; }
    const string& getPaymentDetails() const { return paymentDetails; }
};

// ====================== Database Singleton ======================
class Database {
private:
    static Database* instance;
    
    vector<User> users;
    vector<Medicine> medicines;
    vector<Prescription> prescriptions;
    vector<Transaction> transactions;

    Database() {
        LOG_FUNCTION();
        // Default users
        users.emplace_back("admin", "admin123", UserRole::ADMIN);
        users.emplace_back("pharmacist", "pharma123", UserRole::PHARMACIST);
        loadData();
    }

    void loadData() {
        LOG_FUNCTION();
        ifstream inFile("pharmacy_data.dat", ios::binary);
        if (inFile) {
            // Simplified deserialization - would need proper implementation
            LOG_MESSAGE("Data loaded from file");
        }
    }

public:
    static Database* getInstance() {
        if (!instance) instance = new Database();
        return instance;
    }

    void saveData() {
        LOG_FUNCTION();
        ofstream outFile("pharmacy_data.dat", ios::binary);
        if (outFile) {
            // Simplified serialization - would need proper implementation
            LOG_MESSAGE("Data saved to file");
        }
    }

    User* authenticateUser(const string& username, const string& password) {
        LOG_FUNCTION();
        for (auto& user : users) {
            if (user.authenticate(username, password)) {
                return &user;
            }
        }
        return nullptr;
    }

    void addMedicine(const Medicine& medicine) {
        LOG_FUNCTION();
        if (medicines.size() >= 500) throw DatabaseFullException("medicines");
        medicines.push_back(medicine);
    }

  Medicine* findMedicine(int id) const {
    LOG_FUNCTION();
    for (const auto& med : medicines) {  // Changed to const reference
        if (med.getId() == id) return const_cast<Medicine*>(&med);  // Explicit const_cast
    }
    return nullptr;
}

const vector<Medicine>& getMedicines() const { return medicines; }

void addPrescription(const Prescription& prescription) {
    LOG_FUNCTION();
    if (prescriptions.size() >= 1000) throw DatabaseFullException("prescriptions");  // Fixed plural
    prescriptions.push_back(prescription);
}

Prescription* findPrescription(int id) const {
    LOG_FUNCTION();
    for (const auto& pres : prescriptions) {  // Changed to const reference
        if (pres.getId() == id) return const_cast<Prescription*>(&pres);  // Explicit const_cast
    }
    return nullptr;
}

    const vector<Prescription>& getPrescriptions() const { return prescriptions; }

    void addTransaction(const Transaction& transaction) {
        LOG_FUNCTION();
        if (transactions.size() >= 2000) throw DatabaseFullException("transactions");
        transactions.push_back(transaction);
    }

    const vector<Transaction>& getTransactions() const { return transactions; }

    static void shutdown() {
        if (instance) {
            instance->saveData();
            delete instance;
            instance = nullptr;
        }
    }

    ~Database() {
        saveData();
    }
};

Database* Database::instance = nullptr;

// ====================== UI Helpers ======================
void clearScreen() {
    system("cls");
}

void pauseScreen() {
    cout << "\nPress any key to continue...";
    _getch();
}

int getSingleDigitInput(const string& prompt, int min = 1, int max = 9) {
    char buffer[10];
    int value;
    
    do {
        cout << prompt;
        cin.getline(buffer, sizeof(buffer));
        
        if (strlen(buffer) != 1 || !isdigit(buffer[0])) {
            cout << "Invalid input. Please enter a single digit between " 
                 << min << " and " << max << ".\n";
            continue;
        }
        
        value = buffer[0] - '0';
        
        if (value < min || value > max) {
            cout << "Invalid choice. Please enter a number between " 
                 << min << " and " << max << ".\n";
        }
    } while (value < min || value > max);
    
    return value;
}

int getIntInput(const string& prompt, int min = numeric_limits<int>::min(), int max = numeric_limits<int>::max()) {
    int value;
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);
        
        try {
            size_t pos;
            value = stoi(input, &pos);
            
            if (pos != input.length()) {
                throw invalid_argument("Invalid input");
            }
            
            if (value < min || value > max) {
                cout << "Please enter a number between " << min << " and " << max << ".\n";
                continue;
            }
            
            break;
        } catch (...) {
            cout << "Invalid input. Please enter a valid integer.\n";
        }
    }
    return value;
}

double getDoubleInput(const string& prompt, double min = 0.0, double max = numeric_limits<double>::max()) {
    double value;
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);
        
        try {
            size_t pos;
            value = stod(input, &pos);
            
            if (pos != input.length()) {
                throw invalid_argument("Invalid input");
            }
            
            if (value < min || value > max) {
                cout << "Please enter a number between " << min << " and " << max << ".\n";
                continue;
            }
            
            break;
        } catch (...) {
            cout << "Invalid input. Please enter a valid number.\n";
        }
    }
    return value;
}

string getStringInput(const string& prompt, size_t maxLength = 255) {
    string input;
    do {
        cout << prompt;
        getline(cin, input);
        if (input.empty()) {
            cout << "Input cannot be empty. Please try again: ";
        } else if (input.length() > maxLength) {
            cout << "Input too long (max " << maxLength << " characters). Please try again: ";
            input.clear();
        }
    } while (input.empty());
    return input;
}

string getPasswordInput(const string& prompt) {
    cout << prompt;
    string password;
    char ch;
    while ((ch = _getch()) != 13) {
        if (ch == 8 && !password.empty()) {
            password.pop_back();
            cout << "\b \b";
        } else if (ch >= 32 && ch <= 126) {
            password.push_back(ch);
            cout << "*";
        }
    }
    cout << endl;
    return password;
}

// ====================== Report Strategy ======================
class ReportStrategy {
public:
    virtual ~ReportStrategy() = default;
    virtual void generate() = 0;
    virtual string getName() const = 0;
};

class LowStockReport : public ReportStrategy {
public:
    void generate() override {
        LOG_FUNCTION();
        auto db = Database::getInstance();
        cout << "LOW STOCK REPORT (Quantity < 10)\n"
             << "-------------------------------\n";
        
        bool found = false;
        for (const auto& med : db->getMedicines()) {
            if (med.getQuantity() < 10) {
                med.print();
                cout << "-------------------------------\n";
                found = true;
            }
        }
        
        if (!found) cout << "No medicines with low stock.\n";
    }

    string getName() const override { return "Low Stock Report"; }
};

class ExpiredMedicinesReport : public ReportStrategy {
public:
    void generate() override {
        LOG_FUNCTION();
        auto db = Database::getInstance();
        cout << "EXPIRED MEDICINES REPORT\n"
             << "-----------------------\n";
        
        bool found = false;
        for (const auto& med : db->getMedicines()) {
            if (med.getExpiryDate().isExpired()) {
                med.print();
                cout << "-----------------------\n";
                found = true;
            }
        }
        
        if (!found) cout << "No expired medicines.\n";
    }

    string getName() const override { return "Expired Medicines Report"; }
};

class ControlledSubstancesReport : public ReportStrategy {
public:
    void generate() override {
        LOG_FUNCTION();
        auto db = Database::getInstance();
        cout << "CONTROLLED SUBSTANCES REPORT\n"
             << "---------------------------\n";
        
        bool found = false;
        for (const auto& med : db->getMedicines()) {
            if (med.getIsControlled()) {
                med.print();
                cout << "---------------------------\n";
                found = true;
            }
        }
        
        if (!found) cout << "No controlled substances in inventory.\n";
    }

    string getName() const override { return "Controlled Substances Report"; }
};

class SalesReport : public ReportStrategy {
public:
    void generate() override {
        LOG_FUNCTION();
        auto db = Database::getInstance();
        cout << "SALES REPORT\n"
             << "------------\n";
        
        if (db->getTransactions().empty()) {
            cout << "No transactions recorded.\n";
            return;
        }
        
        double totalSales = 0.0;
        int cashCount = 0, gcashCount = 0, paymayaCount = 0;
        double cashTotal = 0.0, gcashTotal = 0.0, paymayaTotal = 0.0;
        
        for (const auto& trans : db->getTransactions()) {
            totalSales += trans.getTotalAmount();
            
            switch (trans.getPaymentType()) {
                case PaymentType::CASH:
                    cashCount++;
                    cashTotal += trans.getTotalAmount();
                    break;
                case PaymentType::GCASH:
                    gcashCount++;
                    gcashTotal += trans.getTotalAmount();
                    break;
                case PaymentType::PAYMAYA:
                    paymayaCount++;
                    paymayaTotal += trans.getTotalAmount();
                    break;
            }
        }
        
        cout << fixed << setprecision(2);
        cout << "Total Sales: P" << totalSales << "\n";
        cout << "Number of Transactions: " << db->getTransactions().size() << "\n";
        cout << "\nPayment Method Breakdown:\n";
        cout << "Cash: " << cashCount << " transactions (P" << cashTotal << ")\n";
        cout << "GCash: " << gcashCount << " transactions (P" << gcashTotal << ")\n";
        cout << "PayMaya: " << paymayaCount << " transactions (P" << paymayaTotal << ")\n";
    }

    string getName() const override { return "Sales Report"; }
};

// ====================== Menu System ======================
class Menu {
public:
    virtual ~Menu() = default;
    virtual void show() = 0;
protected:
    void showMedicines() {
        clearScreen();
        cout << "MEDICINE INVENTORY\n"
             << "------------------\n";
        
        auto db = Database::getInstance();
        const auto& medicines = db->getMedicines();
        
        if (medicines.empty()) {
            cout << "No medicines in inventory.\n";
        } else {
            for (const auto& med : medicines) {
                med.print();
                cout << "------------------\n";
            }
        }
        pauseScreen();
    }
};

class AdminMenu : public Menu {
public:
    void show() override {
        int choice;
        do {
            clearScreen();
            cout << "ADMIN MENU\n"
                 << "----------\n"
                 << "1. Manage Medicines\n"
                 << "2. Generate Reports\n"
                 << "3. Logout\n";
            
            choice = getSingleDigitInput("Enter your choice (1-3): ", 1, 3);
            
            switch (choice) {
                case 1: manageMedicines(); break;
                case 2: generateReports(); break;
                case 3: return;
            }
        } while (true);
    }

private:
    void manageMedicines() {
        int choice;
        do {
            clearScreen();
            cout << "MEDICINE MANAGEMENT\n"
                 << "-------------------\n"
                 << "1. Add Medicine\n"
                 << "2. View Medicines\n"
                 << "3. Update Medicine\n"
                 << "4. Delete Medicine\n"
                 << "5. Back\n";
            
            choice = getSingleDigitInput("Enter your choice (1-5): ", 1, 5);
            
            switch (choice) {
                case 1: addMedicine(); break;
                case 2: showMedicines(); break;
                case 3: updateMedicine(); break;
                case 4: deleteMedicine(); break;
                case 5: return;
            }
        } while (true);
    }

    void addMedicine() {
        try {
            clearScreen();
            cout << "ADD NEW MEDICINE\n"
                 << "----------------\n";
            
            auto db = Database::getInstance();
            
            string name = getStringInput("Medicine Name: ");
            string desc = getStringInput("Description: ");
            int qty = getIntInput("Quantity: ", 0);
            double price = getDoubleInput("Price: ", 0.0);
            
            int day, month, year;
            do {
                day = getIntInput("Expiry Day (1-31): ", 1, 31);
                month = getIntInput("Expiry Month (1-12): ", 1, 12);
                year = getIntInput("Expiry Year (1900-2100): ", 1900, 2100);
                try {
                    Date testDate(day, month, year);
                    break;
                } catch (const exception& e) {
                    cout << "Invalid date: " << e.what() << ". Please try again.\n";
                }
            } while (true);
            
            char controlled;
            do {
                cout << "Is this a controlled substance? (y/n): ";
                cin >> controlled;
                cin.ignore(1000, '\n');
            } while (controlled != 'y' && controlled != 'Y' && controlled != 'n' && controlled != 'N');
            
            int newId = db->getMedicines().empty() ? 1 : db->getMedicines().back().getId() + 1;
            Medicine med(newId, name, desc, qty, price, Date(day, month, year), (controlled == 'y' || controlled == 'Y'));
            
            db->addMedicine(med);
            cout << "\nMedicine added successfully! ID: " << med.getId() << "\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }

    void updateMedicine() {
        try {
            clearScreen();
            cout << "UPDATE MEDICINE\n"
                 << "---------------\n";
            
            auto db = Database::getInstance();
            if (db->getMedicines().empty()) {
                cout << "No medicines to update.\n";
                pauseScreen();
                return;
            }
            
            int id = getIntInput("Enter Medicine ID to update: ", 1);
            Medicine* med = db->findMedicine(id);
            
            if (!med) {
                throw NotFoundException("Medicine");
            }
            
            // Store original values in case user cancels
            Medicine original = *med;
            
            med->print();
            cout << "\nEnter new details (leave blank to keep current):\n";
            
            string input = getStringInput("Medicine Name: ");
            if (!input.empty()) med->setName(input);
            
            input = getStringInput("Description: ");
            if (!input.empty()) med->setDescription(input);
            
            int qty = getIntInput("Quantity (-1 to keep current): ", -1);
            if (qty >= 0) med->setQuantity(qty);
            
            double price = getDoubleInput("Price (-1 to keep current): ", -1);
            if (price >= 0) med->setPrice(price);
            
            char choice;
            cout << "Update expiry date? (y/n): ";
            cin >> choice;
            cin.ignore(1000, '\n');
            if (choice == 'y' || choice == 'Y') {
                int day, month, year;
                do {
                    day = getIntInput("Expiry Day (1-31): ", 1, 31);
                    month = getIntInput("Expiry Month (1-12): ", 1, 12);
                    year = getIntInput("Expiry Year (1900-2100): ", 1900, 2100);
                    try {
                        Date testDate(day, month, year);
                        break;
                    } catch (const exception& e) {
                        cout << "Invalid date: " << e.what() << ". Please try again.\n";
                    }
                } while (true);
                med->setExpiryDate(Date(day, month, year));
            }
            
            cout << "Update controlled status? (y/n): ";
            cin >> choice;
            cin.ignore(1000, '\n');
            if (choice == 'y' || choice == 'Y') {
                do {
                    cout << "Is this a controlled substance? (y/n): ";
                    cin >> choice;
                    cin.ignore(1000, '\n');
                } while (choice != 'y' && choice != 'Y' && choice != 'n' && choice != 'N');
                med->setIsControlled(choice == 'y' || choice == 'Y');
            }
            
            // Ask if user wants to save changes
            cout << "\nSave changes? (y/n): ";
            cin >> choice;
            cin.ignore(1000, '\n');
            
            if (choice == 'y' || choice == 'Y') {
                cout << "\nMedicine updated successfully!\n";
            } else {
                // Restore original values
                *med = original;
                cout << "\nChanges discarded.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }

    void deleteMedicine() {
        try {
            clearScreen();
            cout << "DELETE MEDICINE\n"
                 << "---------------\n";
            
            auto db = Database::getInstance();
            if (db->getMedicines().empty()) {
                cout << "No medicines to delete.\n";
                pauseScreen();
                return;
            }
            
            int id = getIntInput("Enter Medicine ID to delete: ", 1);
            auto& medicines = const_cast<vector<Medicine>&>(db->getMedicines());
            
            auto it = find_if(medicines.begin(), medicines.end(), 
                [id](const Medicine& m) { return m.getId() == id; });
            
            if (it == medicines.end()) {
                throw NotFoundException("Medicine");
            }
            
            char confirm;
            cout << "Are you sure you want to delete this medicine? (y/n): ";
            cin >> confirm;
            cin.ignore(1000, '\n');
            
            if (confirm == 'y' || confirm == 'Y') {
                medicines.erase(it);
                cout << "Medicine deleted successfully!\n";
            } else {
                cout << "Deletion canceled.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }

    void generateReports() {
        vector<unique_ptr<ReportStrategy>> reports;
        reports.push_back(make_unique<LowStockReport>());
        reports.push_back(make_unique<ExpiredMedicinesReport>());
        reports.push_back(make_unique<ControlledSubstancesReport>());
        reports.push_back(make_unique<SalesReport>());

        int choice;
        do {
            clearScreen();
            cout << "GENERATE REPORTS\n"
                 << "----------------\n";
            
            for (size_t i = 0; i < reports.size(); i++) {
                cout << (i + 1) << ". " << reports[i]->getName() << "\n";
            }
            cout << (reports.size() + 1) << ". Back\n";
            
            choice = getSingleDigitInput("Enter your choice (1-" + to_string(reports.size() + 1) + "): ", 
                                       1, reports.size() + 1);
            
            if (choice <= reports.size()) {
                clearScreen();
                reports[choice - 1]->generate();
                pauseScreen();
            }
        } while (choice != reports.size() + 1);
    }
};

class PharmacistMenu : public Menu {
public:
    void show() override {
        int choice;
        do {
            clearScreen();
            cout << "PHARMACIST MENU\n"
                 << "---------------\n"
                 << "1. Manage Prescriptions\n"
                 << "2. Process Billing\n"
                 << "3. View Medicines\n"
                 << "4. Logout\n";
            
            choice = getSingleDigitInput("Enter your choice (1-4): ", 1, 4);
            
            switch (choice) {
                case 1: managePrescriptions(); break;
                case 2: processBilling(); break;
                case 3: showMedicines(); break;
                case 4: return;
            }
        } while (true);
    }

private:
    void managePrescriptions() {
        int choice;
        do {
            clearScreen();
            cout << "PRESCRIPTION MANAGEMENT\n"
                 << "-----------------------\n"
                 << "1. Add Prescription\n"
                 << "2. View Prescriptions\n"
                 << "3. Update Prescription\n"
                 << "4. Delete Prescription\n"
                 << "5. Fill Prescription\n"
                 << "6. Back\n";
            
            choice = getSingleDigitInput("Enter your choice (1-6): ", 1, 6);
            
            switch (choice) {
                case 1: addPrescription(); break;
                case 2: viewPrescriptions(); break;
                case 3: updatePrescription(); break;
                case 4: deletePrescription(); break;
                case 5: fillPrescription(); break;
                case 6: return;
            }
        } while (true);
    }

    void addPrescription() {
        try {
            clearScreen();
            cout << "ADD NEW PRESCRIPTION\n"
                 << "--------------------\n";
            
            auto db = Database::getInstance();
            if (db->getMedicines().empty()) {
                cout << "Cannot add prescription - no medicines available.\n";
                pauseScreen();
                return;
            }
            
            string patient = getStringInput("Patient Name: ");
            string doctor = getStringInput("Doctor Name: ");
            
            int day, month, year;
            do {
                day = getIntInput("Date Day (1-31): ", 1, 31);
                month = getIntInput("Date Month (1-12): ", 1, 12);
                year = getIntInput("Date Year (1900-2100): ", 1900, 2100);
                try {
                    Date testDate(day, month, year);
                    break;
                } catch (const exception& e) {
                    cout << "Invalid date: " << e.what() << ". Please try again.\n";
                }
            } while (true);
            
            int itemCount = getIntInput("Number of medicines (1-10): ", 1, 10);
            
            int newId = db->getPrescriptions().empty() ? 1 : db->getPrescriptions().back().getId() + 1;
            Prescription pres(newId, patient, doctor, Date(day, month, year));
            
            for (int i = 0; i < itemCount; i++) {
                cout << "\nMedicine #" << (i + 1) << ":\n";
                int medId = getIntInput("Medicine ID: ", 1);
                Medicine* med = db->findMedicine(medId);
                
                if (!med) {
                    cout << "Medicine not found. Please try again.\n";
                    i--;
                    continue;
                }
                
                int maxQty = med->getQuantity();
                int qty = getIntInput("Quantity (1-" + to_string(maxQty) + "): ", 1, maxQty);
                pres.addItem(PrescriptionItem(medId, qty));
            }
            
            db->addPrescription(pres);
            cout << "\nPrescription added successfully! ID: " << pres.getId() << "\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }

    void viewPrescriptions() {
        clearScreen();
        cout << "PRESCRIPTION LIST\n"
             << "-----------------\n";
        
        auto db = Database::getInstance();
        const auto& prescriptions = db->getPrescriptions();
        
        if (prescriptions.empty()) {
            cout << "No prescriptions available.\n";
        } else {
            for (const auto& pres : prescriptions) {
                pres.print();
                cout << "-----------------\n";
            }
        }
        pauseScreen();
    }

    void updatePrescription() {
        try {
            clearScreen();
            cout << "UPDATE PRESCRIPTION\n"
                 << "------------------\n";
            
            auto db = Database::getInstance();
            if (db->getPrescriptions().empty()) {
                cout << "No prescriptions to update.\n";
                pauseScreen();
                return;
            }
            
            int id = getIntInput("Enter Prescription ID to update: ", 1);
            Prescription* pres = db->findPrescription(id);
            
            if (!pres) {
                throw NotFoundException("Prescription");
            }
            
            if (pres->getIsFilled()) {
                cout << "Cannot update a filled prescription.\n";
                pauseScreen();
                return;
            }
            
            pres->print();
            cout << "\nEnter new details (leave blank to keep current):\n";
            
            string input = getStringInput("Patient Name: ");
            if (!input.empty()) pres->setPatientName(input);
            
            input = getStringInput("Doctor Name: ");
            if (!input.empty()) pres->setDoctorName(input);
            
            char choice;
            cout << "Update date? (y/n): ";
            cin >> choice;
            cin.ignore(1000, '\n');
            if (choice == 'y' || choice == 'Y') {
                int day, month, year;
                do {
                    day = getIntInput("Date Day (1-31): ", 1, 31);
                    month = getIntInput("Date Month (1-12): ", 1, 12);
                    year = getIntInput("Date Year (1900-2100): ", 1900, 2100);
                    try {
                        Date testDate(day, month, year);
                        break;
                    } catch (const exception& e) {
                        cout << "Invalid date: " << e.what() << ". Please try again.\n";
                    }
                } while (true);
                pres->setDate(Date(day, month, year));
            }
            
            cout << "\nPrescription updated successfully!\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }

    void deletePrescription() {
        try {
            clearScreen();
            cout << "DELETE PRESCRIPTION\n"
                 << "------------------\n";
            
            auto db = Database::getInstance();
            if (db->getPrescriptions().empty()) {
                cout << "No prescriptions to delete.\n";
                pauseScreen();
                return;
            }
            
            int id = getIntInput("Enter Prescription ID to delete: ", 1);
            auto& prescriptions = const_cast<vector<Prescription>&>(db->getPrescriptions());
            
            auto it = find_if(prescriptions.begin(), prescriptions.end(), 
                [id](const Prescription& p) { return p.getId() == id; });
            
            if (it == prescriptions.end()) {
                throw NotFoundException("Prescription");
            }
            
            char confirm;
            cout << "Are you sure you want to delete this prescription? (y/n): ";
            cin >> confirm;
            cin.ignore(1000, '\n');
            
            if (confirm == 'y' || confirm == 'Y') {
                prescriptions.erase(it);
                cout << "Prescription deleted successfully!\n";
            } else {
                cout << "Deletion canceled.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }

    void fillPrescription() {
        try {
            clearScreen();
            cout << "FILL PRESCRIPTION\n"
                 << "----------------\n";
            
            auto db = Database::getInstance();
            if (db->getPrescriptions().empty()) {
                cout << "No prescriptions available.\n";
                pauseScreen();
                return;
            }
            
            int id = getIntInput("Enter Prescription ID to fill: ", 1);
            Prescription* pres = db->findPrescription(id);
            
            if (!pres) {
                throw NotFoundException("Prescription");
            }
            
            if (pres->getIsFilled()) {
                cout << "This prescription has already been filled.\n";
                pauseScreen();
                return;
            }
            
            pres->print();
            cout << "\n";
            
            // Check if all medicines are available
            for (const auto& item : pres->getItems()) {
                Medicine* med = db->findMedicine(item.getMedicineId());
                if (!med) {
                    throw NotFoundException("Medicine in prescription");
                }
                if (med->getQuantity() < item.getQuantity()) {
                    throw DataValidationException("Not enough stock for " + med->getName());
                }
            }
            
            char confirm;
            cout << "Confirm filling this prescription? (y/n): ";
            cin >> confirm;
            cin.ignore(1000, '\n');
            
            if (confirm == 'y' || confirm == 'Y') {
                // Reduce stock quantities
                for (const auto& item : pres->getItems()) {
                    Medicine* med = db->findMedicine(item.getMedicineId());
                    med->reduceQuantity(item.getQuantity());
                }
                
                pres->fill();
                cout << "Prescription filled successfully!\n";
            } else {
                cout << "Filling canceled.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }

    void processBilling() {
        try {
            clearScreen();
            cout << "PROCESS BILLING\n"
                 << "---------------\n";
            
            auto db = Database::getInstance();
            if (db->getPrescriptions().empty()) {
                cout << "No prescriptions available for billing.\n";
                pauseScreen();
                return;
            }
            
            int id = getIntInput("Enter Prescription ID to bill: ", 1);
            Prescription* pres = db->findPrescription(id);
            
            if (!pres) {
                throw NotFoundException("Prescription");
            }
            
            if (!pres->getIsFilled()) {
                cout << "Cannot bill an unfilled prescription.\n";
                pauseScreen();
                return;
            }
            
            // Check if already billed
            for (const auto& trans : db->getTransactions()) {
                if (trans.getPrescriptionId() == id) {
                    cout << "This prescription has already been billed.\n";
                    pauseScreen();
                    return;
                }
            }
            
            pres->print();
            cout << "\n";
            
            // Calculate total amount
            double total = 0.0;
            for (const auto& item : pres->getItems()) {
                Medicine* med = db->findMedicine(item.getMedicineId());
                total += med->getPrice() * item.getQuantity();
            }
            
            cout << fixed << setprecision(2);
            cout << "Total Amount: P" << total << "\n";
            
            // Get payment method
            cout << "\nPayment Methods:\n"
                 << "1. Cash\n"
                 << "2. GCash\n"
                 << "3. PayMaya\n";
            
            int choice = getSingleDigitInput("Select payment method (1-3): ", 1, 3);
            PaymentType paymentType = static_cast<PaymentType>(choice - 1);
            
            string details;
            if (paymentType != PaymentType::CASH) {
                details = getStringInput("Enter transaction/reference number: ");
            } else {
                details = "Cash payment";
            }
            
            // Create transaction
            int newId = db->getTransactions().empty() ? 1 : db->getTransactions().back().getId() + 1;
            time_t now = time(nullptr);
            tm* current = localtime(&now);
            Date today(current->tm_mday, current->tm_mon + 1, current->tm_year + 1900);
            
            Transaction trans(newId, today, id, total, paymentType, details);
            db->addTransaction(trans);
            
            cout << "\nTransaction completed successfully! Transaction ID: " << trans.getId() << "\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        pauseScreen();
    }
};

// ====================== Main Application ======================
class PharmacySystem {
public:
    void run() {
        LOG_FUNCTION();
        try {
            while (true) {
                clearScreen();
                cout << "PHARMACY MANAGEMENT SYSTEM\n"
                     << "-------------------------\n"
                     << "1. Login\n"
                     << "2. Exit\n";
                
                int choice = getSingleDigitInput("Enter your choice (1-2): ", 1, 2);
                
                switch (choice) {
                    case 1: login(); break;
                    case 2: return;
                }
            }
        } catch (const exception& e) {
            cout << "Fatal error: " << e.what() << endl;
            pauseScreen();
        }
    }

private:
    void login() {
        LOG_FUNCTION();
        clearScreen();
        cout << "LOGIN\n"
             << "-----\n";
        
        string username = getStringInput("Username: ");
        string password = getPasswordInput("Password: ");
        
        try {
            auto db = Database::getInstance();
            User* user = db->authenticateUser(username, password);
            
            if (!user) {
                throw AuthenticationException();
            }
            
            cout << "\nLogin successful! Welcome, " << username << ".\n";
            pauseScreen();
            
            unique_ptr<Menu> menu;
            if (user->getRole() == UserRole::ADMIN) {
                menu = make_unique<AdminMenu>();
            } else {
                menu = make_unique<PharmacistMenu>();
            }
            
            menu->show();
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
            pauseScreen();
        }
    }
};

// ====================== Main Function ======================
int main() {
    PharmacySystem system;
    system.run();
    
    // Clean up singletons
    Database::shutdown();
    Logger::shutdown();
    
    return 0;
}