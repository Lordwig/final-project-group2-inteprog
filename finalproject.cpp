#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <cctype>

using namespace std;

// Utility functions
namespace Utils {
    string getCurrentTimestamp() {
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
        return string(buffer);
    }

    string trim(const string& str) {
        size_t first = str.find_first_not_of(' ');
        if (string::npos == first) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    bool isValidNumber(const string& s) {
        if (s.empty()) return false;
        size_t i = 0;
        if (s[0] == '-') {
            if (s.length() == 1) return false;
            i = 1;
        }
        for (; i < s.size(); i++) {
            if (!isdigit(s[i]) && s[i] != '.') return false;
        }
        return true;
    }

    bool isValidDate(const string& date) {
        if (date.length() != 10 || date[4] != '-' || date[7] != '-') return false;
        
        for (int i = 0; i < 10; i++) {
            if (i == 4 || i == 7) continue;
            if (!isdigit(date[i])) return false;
        }

        int year = stoi(date.substr(0, 4));
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));

        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;

        if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
            return false;
        
        if (month == 2) {
            bool isLeap = (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
            if (day > (isLeap ? 29 : 28))
                return false;
        }

        return true;
    }

    string toLower(const string& s) {
        string result = s;
        transform(result.begin(), result.end(), result.begin(), 
                 [](unsigned char c){ return tolower(c); });
        return result;
    }

    string getInput(const string& prompt) {
        string input;
        cout << prompt;
        getline(cin, input);
        return trim(input);
    }

    float getFloatInput(const string& prompt) {
        while (true) {
            string input = getInput(prompt);
            try {
                size_t pos;
                float value = stof(input, &pos);
                if (pos == input.size()) {
                    return value;
                }
            } catch (...) {
                cout << "Invalid input. Please enter a valid number.\n";
            }
        }
    }

    int getIntInput(const string& prompt) {
        while (true) {
            string input = getInput(prompt);
            try {
                size_t pos;
                int value = stoi(input, &pos);
                if (pos == input.size()) {
                    return value;
                }
            } catch (...) {
                cout << "Invalid input. Please enter a whole number.\n";
            }
        }
    }

    string getDateInput(const string& prompt) {
        string input;
        bool valid = false;
        do {
            input = getInput(prompt + " (YYYY-MM-DD): ");
            valid = isValidDate(input);
            if (!valid) {
                cout << "Invalid date format or impossible date. Please use YYYY-MM-DD format.\n";
            }
        } while (!valid);
        return input;
    }

    void clearScreen() {
        cout << "\033[2J\033[1;1H";
    }

    void pause() {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Abstract Logger interface
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const string& action, const string& username) = 0;
    virtual void viewLogs() = 0;
};

// Concrete Logger implementation
class FileLogger : public ILogger {
private:
    static FileLogger* instance;
    int lastTransactionId;

    FileLogger() {
        ifstream idFile("last_id.txt");
        if (idFile.is_open()) {
            idFile >> lastTransactionId;
            idFile.close();
        } else {
            lastTransactionId = 0;
        }
    }

public:
    static FileLogger* getInstance() {
        if (!instance) {
            instance = new FileLogger();
        }
        return instance;
    }

    ~FileLogger() override = default;

    void log(const string& action, const string& username) override {
        ofstream logFile("transaction_log.txt", ios::app);
        if (logFile.is_open()) {
            lastTransactionId++;
            logFile << "ID: " << lastTransactionId << " | "
                    << "Time: " << Utils::getCurrentTimestamp() << " | "
                    << "User: " << username << " | "
                    << "Action: " << action << "\n";
            logFile.close();

            ofstream idFile("last_id.txt");
            if (idFile.is_open()) {
                idFile << lastTransactionId;
                idFile.close();
            }
        }
    }

    void viewLogs() override {
        ifstream logFile("transaction_log.txt");
        if (logFile.is_open()) {
            string line;
            while (getline(logFile, line)) {
                cout << line << "\n";
            }
            logFile.close();
        } else {
            cout << "No logs found.\n";
        }
    }
};

FileLogger* FileLogger::instance = nullptr;

// Abstract Billing Strategy
class IBillingStrategy {
public:
    virtual ~IBillingStrategy() = default;
    virtual string getName() const = 0;
    virtual bool processPayment(float amount) = 0;
};

// Concrete Billing Strategies
class CashBilling : public IBillingStrategy {
public:
    string getName() const override { return "Cash"; }
    
    bool processPayment(float amount) override {
        cout << "Processing cash payment of $" << fixed << setprecision(2) << amount << "\n";
        cout << "Payment received successfully.\n";
        return true;
    }
};

class GCashBilling : public IBillingStrategy {
public:
    string getName() const override { return "GCash"; }
    
    bool processPayment(float amount) override {
        string mobileNumber;
        bool valid = false;
        do {
            mobileNumber = Utils::getInput("Enter GCash mobile number (09XXXXXXXXX): ");
            valid = (mobileNumber.length() == 11 && mobileNumber.substr(0, 2) == "09");
            if (!valid) {
                cout << "Invalid GCash mobile number format.\n";
            }
        } while (!valid);

        cout << "Sending payment request of $" << fixed << setprecision(2) << amount 
             << " to " << mobileNumber << "...\n";
        cout << "Payment confirmed via GCash.\n";
        return true;
    }
};

class PayMayaBilling : public IBillingStrategy {
public:
    string getName() const override { return "PayMaya"; }
    
    bool processPayment(float amount) override {
        string cardNumber;
        bool valid = false;
        do {
            cardNumber = Utils::getInput("Enter PayMaya card number (16 digits): ");
            valid = (cardNumber.length() == 16 && all_of(cardNumber.begin(), cardNumber.end(), ::isdigit));
            if (!valid) {
                cout << "Invalid card number format.\n";
            }
        } while (!valid);

        cout << "Processing PayMaya payment of $" << fixed << setprecision(2) << amount << "...\n";
        cout << "Payment confirmed via PayMaya.\n";
        return true;
    }
};

// Medicine interface
class IMedicine {
public:
    virtual ~IMedicine() = default;
    virtual int getId() const = 0;
    virtual string getName() const = 0;
    virtual int getQuantity() const = 0;
    virtual string getExpiryDate() const = 0;
    virtual float getPrice() const = 0;
    virtual void setQuantity(int q) = 0;
    virtual void setExpiryDate(const string& e) = 0;
    virtual void setPrice(float p) = 0;
    virtual void display() const = 0;
    virtual string toFileString() const = 0;
};

// Concrete Medicine implementation
class Medicine : public IMedicine {
private:
    int id;
    string name;
    int quantity;
    string expiryDate;
    float price;
    static int nextId;

public:
     // Modified constructor to handle both new and loaded medicines
    Medicine(const string& n, int q, const string& e, float p, int existingId = -1)
        : name(Utils::trim(n)), quantity(q), expiryDate(e), price(p) {
        if (existingId == -1) {
            // New medicine - assign next ID
            id = nextId++;
        } else {
            // Loaded from file - use existing ID
            id = existingId;
            // Update nextId to avoid future conflicts
            if (id >= nextId) {
                nextId = id + 1;
            }
        }
        // Validation remains same
        if (quantity < 0) throw invalid_argument("Quantity cannot be negative");
        if (price < 0) throw invalid_argument("Price cannot be negative");
        if (!Utils::isValidDate(expiryDate)) 
            throw invalid_argument("Invalid expiry date");
    }

    ~Medicine() override = default;

    int getId() const override { return id; }
    string getName() const override { return name; }
    int getQuantity() const override { return quantity; }
    string getExpiryDate() const override { return expiryDate; }
    float getPrice() const override { return price; }

    void setQuantity(int q) override { 
        if (q < 0) throw invalid_argument("Quantity cannot be negative");
        quantity = q; 
    }
    
    void setExpiryDate(const string& e) override { 
        if (!Utils::isValidDate(e)) throw invalid_argument("Invalid expiry date");
        expiryDate = e; 
    }
    
    void setPrice(float p) override { 
        if (p < 0) throw invalid_argument("Price cannot be negative");
        price = p; 
    }

    void display() const override {
        cout << "Name: " << name << "\n"
             << "Quantity: " << quantity << "\n"
             << "Expiry Date: " << expiryDate << "\n"
             << "Price: $" << fixed << setprecision(2) << price << "\n";
    }

    string toFileString() const override {
        return name + "," + to_string(quantity) + "," + expiryDate + "," + to_string(price);
    }

    static Medicine fromFileString(const string& line) {
        stringstream ss(line);
        string name, quantityStr, expiryDate, priceStr;
        
        getline(ss, name, ',');
        getline(ss, quantityStr, ',');
        getline(ss, expiryDate, ',');
        getline(ss, priceStr, ',');

        try {
            return Medicine(name, stoi(quantityStr), expiryDate, stof(priceStr));
        } catch (...) {
            cerr << "Error parsing medicine data\n";
            return Medicine("Invalid", 0, "0000-00-00", 0.0f);
        }
    }
};

int Medicine::nextId = 1;

// Prescription interface
class IPrescription {
public:
    virtual ~IPrescription() = default;
    virtual string getId() const = 0;
    virtual string getPatientName() const = 0;
    virtual string getMedicineName() const = 0;
    virtual int getQuantity() const = 0;
    virtual string getDate() const = 0;
    virtual string getPrescribingDoctor() const = 0;
    virtual void display() const = 0;
    virtual string toFileString() const = 0;
};

// Concrete Prescription implementation
class Prescription : public IPrescription {
private:
    string id;
    string patientName;
    string medicineName;
    int quantity;
    string date;
    string prescribingDoctor;

public:
    Prescription(const string& i, const string& pn, const string& mn, int q, 
                const string& d, const string& pd)
        : id(Utils::trim(i)), patientName(Utils::trim(pn)), 
          medicineName(Utils::trim(mn)), quantity(q), date(d), 
          prescribingDoctor(Utils::trim(pd)) {
        if (quantity <= 0) throw invalid_argument("Quantity must be positive");
        if (!Utils::isValidDate(date)) throw invalid_argument("Invalid date");
    }

    ~Prescription() override = default;

    string getId() const override { return id; }
    string getPatientName() const override { return patientName; }
    string getMedicineName() const override { return medicineName; }
    int getQuantity() const override { return quantity; }
    string getDate() const override { return date; }
    string getPrescribingDoctor() const override { return prescribingDoctor; }

    void display() const override {
        cout << "Prescription ID: " << id << "\n"
             << "Patient: " << patientName << "\n"
             << "Medicine: " << medicineName << "\n"
             << "Quantity: " << quantity << "\n"
             << "Date: " << date << "\n"
             << "Doctor: " << prescribingDoctor << "\n";
    }

    string toFileString() const override {
        return id + "," + patientName + "," + medicineName + "," + 
               to_string(quantity) + "," + date + "," + prescribingDoctor;
    }

    static Prescription fromFileString(const string& line) {
        stringstream ss(line);
        string id, patientName, medicineName, quantityStr, date, prescribingDoctor;
        
        getline(ss, id, ',');
        getline(ss, patientName, ',');
        getline(ss, medicineName, ',');
        getline(ss, quantityStr, ',');
        getline(ss, date, ',');
        getline(ss, prescribingDoctor, ',');

        try {
            return Prescription(id, patientName, medicineName, stoi(quantityStr), date, prescribingDoctor);
        } catch (...) {
            cerr << "Error parsing prescription data\n";
            return Prescription("Invalid", "Invalid", "Invalid", 0, "0000-00-00", "Invalid");
        }
    }
};

// Pharmacy System interface
class IPharmacySystem {
public:
    virtual ~IPharmacySystem() = default;
    virtual void run() = 0;
};

// Concrete Pharmacy System implementation
class PharmacySystem : public IPharmacySystem {
private:
    vector<unique_ptr<IMedicine>> medicines;
    vector<unique_ptr<IPrescription>> prescriptions;
    string currentUser;
    string currentRole;

    void loadMedicines() {
        medicines.clear();
        ifstream file("medicines.txt");
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                medicines.push_back(make_unique<Medicine>(Medicine::fromFileString(line)));
            }
            file.close();
        }
    }

    void saveMedicines() {
        ofstream file("medicines.txt");
        if (file.is_open()) {
            for (const auto& med : medicines) {
                file << med->toFileString() << "\n";
            }
            file.close();
        }
    }

    void loadPrescriptions() {
        prescriptions.clear();
        ifstream file("prescriptions.txt");
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                prescriptions.push_back(make_unique<Prescription>(Prescription::fromFileString(line)));
            }
            file.close();
        }
    }

    void savePrescriptions() {
        ofstream file("prescriptions.txt");
        if (file.is_open()) {
            for (const auto& pres : prescriptions) {
                file << pres->toFileString() << "\n";
            }
            file.close();
        }
    }

    string getDateIn30Days(const string& currentDate) {
        int year = stoi(currentDate.substr(0, 4));
        int month = stoi(currentDate.substr(5, 2));
        int day = stoi(currentDate.substr(8, 2));

        tm timeStruct = {0};
        timeStruct.tm_year = year - 1900;
        timeStruct.tm_mon = month - 1;
        timeStruct.tm_mday = day + 30;
        mktime(&timeStruct);

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeStruct);
        return string(buffer);
    }

    void generateComplianceReport() {
        ofstream reportFile("compliance_report.txt");
        if (!reportFile.is_open()) {
            cout << "Error creating compliance report.\n";
            return;
        }

        string currentDate = Utils::getCurrentTimestamp().substr(0, 10);

        reportFile << "Compliance Report - " << currentDate << "\n";
        reportFile << "========================================\n\n";

        reportFile << "Low Stock Medicines (Quantity < 10):\n";
        bool hasLowStock = false;
        for (const auto& med : medicines) {
            if (med->getQuantity() < 10) {
                reportFile << "- " << med->getName() << ": " << med->getQuantity() << " remaining\n";
                hasLowStock = true;
            }
        }
        if (!hasLowStock) reportFile << "No low stock medicines.\n";
        reportFile << "\n";

        reportFile << "Medicines Expiring Soon (within 30 days):\n";
        bool hasExpiringSoon = false;
        for (const auto& med : medicines) {
            string expiry = med->getExpiryDate();
            if (expiry > currentDate && expiry <= getDateIn30Days(currentDate)) {
                reportFile << "- " << med->getName() << ": Expires on " << expiry << "\n";
                hasExpiringSoon = true;
            }
        }
        if (!hasExpiringSoon) reportFile << "No medicines expiring soon.\n";

        reportFile.close();
        cout << "Compliance report generated successfully.\n";
        FileLogger::getInstance()->log("Generated compliance report", currentUser);
    }

    bool authenticateUser() {
    Utils::clearScreen();
    cout << "=== PHARMACY MANAGEMENT SYSTEM ===\n";
    cout << "Please login to continue\n";

    string username = Utils::getInput("Username: ");
    string password = Utils::getInput("Password: ");

    if (username == "admin" && password == "admin123") {
        currentUser = username;
        currentRole = "Admin";
        FileLogger::getInstance()->log("Logged in as Admin", username);
        return true;
    } 
    else if (username == "pharmacist" && password == "pharma123") {
        currentUser = username;
        currentRole = "Pharmacist";
        FileLogger::getInstance()->log("Logged in as Pharmacist", username);
        return true;
    }

    cout << "Invalid username or password.\n";
    Utils::pause();
    return false;
}


    void adminMenu() {
        int choice = 0;
        bool running = true;
        
        while (running) {
            Utils::clearScreen();
            cout << "=== ADMIN MENU ===\n"
                 << "1. Medicine Management\n"
                 << "2. View Compliance Report\n"
                 << "3. View Transaction Logs\n"
                 << "4. Logout\n"
                 << "Enter your choice: ";
            choice = Utils::getIntInput("");

            switch (choice) {
                case 1: medicineManagementMenu(); break;
                case 2: {
                    generateComplianceReport();
                    cout << "\n=== Compliance Report ===\n";
                    ifstream reportFile("compliance_report.txt");
                    if (reportFile.is_open()) {
                        string line;
                        while (getline(reportFile, line)) {
                            cout << line << "\n";
                        }
                        reportFile.close();
                    }
                    Utils::pause();
                    break;
                }
                case 3: {
                    cout << "\n=== Transaction Logs ===\n";
                    FileLogger::getInstance()->viewLogs();
                    Utils::pause();
                    break;
                }
                case 4: running = false; break;
                default: cout << "Invalid choice. Please try again.\n"; Utils::pause();
            }
        }
    }

    void medicineManagementMenu() {
        int choice = 0;
        bool running = true;
        
        while (running) {
            Utils::clearScreen();
            cout << "=== MEDICINE MANAGEMENT ===\n"
                 << "1. Add Medicine\n"
                 << "2. View All Medicines\n"
                 << "3. Update Medicine\n"
                 << "4. Delete Medicine\n"
                 << "5. Back to Admin Menu\n"
                 << "Enter your choice: ";
            choice = Utils::getIntInput("");

            switch (choice) {
                case 1: addMedicine(); break;
                case 2: viewAllMedicines(); break;
                case 3: updateMedicine(); break;
                case 4: deleteMedicine(); break;
                case 5: running = false; break;
                default: cout << "Invalid choice. Please try again.\n"; Utils::pause();
            }
        }
    }

    void addMedicine() {
        Utils::clearScreen();
        cout << "=== ADD NEW MEDICINE ===\n";

        string name;
        bool nameValid = false;
        while (!nameValid) {
            name = Utils::getInput("Enter medicine name: ");
            nameValid = !name.empty();
            if (!nameValid) {
                cout << "Name cannot be empty.\n";
            }
        }

        int quantity = 0;
        bool quantityValid = false;
        while (!quantityValid) {
            quantity = Utils::getIntInput("Enter quantity: ");
            quantityValid = (quantity > 0);
            if (!quantityValid) {
                cout << "Quantity must be positive.\n";
            }
        }

        string expiryDate = Utils::getDateInput("Enter expiry date");

        float price = 0.0f;
        bool priceValid = false;
        while (!priceValid) {
            price = Utils::getFloatInput("Enter price: ");
            priceValid = (price > 0);
            if (!priceValid) {
                cout << "Price must be positive.\n";
            }
        }

        try {
            bool medicineUpdated = false;
            for (auto& med : medicines) {
                if (Utils::toLower(med->getName()) == Utils::toLower(name) &&
                    med->getExpiryDate() == expiryDate &&
                    abs(med->getPrice() - price) < 0.001f) {
                    
                    int oldQuantity = med->getQuantity();
                    med->setQuantity(oldQuantity + quantity);
                    medicineUpdated = true;
                    
                    cout << "\nMedicine already exists! Quantity updated.\n"
                         << "Previous quantity: " << oldQuantity << "\n"
                         << "Added quantity: " << quantity << "\n"
                         << "New total quantity: " << med->getQuantity() << "\n";
                    
                    FileLogger::getInstance()->log(
                        "Updated medicine quantity: " + name + 
                        " (" + to_string(oldQuantity) + "→" + to_string(med->getQuantity()) + ")", 
                        currentUser
                    );
                    break;
                }
            }

            if (!medicineUpdated) {
                medicines.push_back(make_unique<Medicine>(name, quantity, expiryDate, price));
                cout << "\nNew medicine added successfully!\n";
                FileLogger::getInstance()->log(
                    "Added new medicine: " + name + 
                    " (Qty: " + to_string(quantity) + 
                    ", Exp: " + expiryDate + 
                    ", Price: $" + to_string(price) + ")", 
                    currentUser
                );
            }

            saveMedicines();
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
        Utils::pause();
    }

    void viewAllMedicines() {
        Utils::clearScreen();
        cout << "=== ALL MEDICINES ===\n";
        
        if (medicines.empty()) {
            cout << "No medicines found.\n";
            Utils::pause();
            return;
        }

        cout << left 
             << setw(5) << "ID" 
             << setw(25) << "Medicine Name" 
             << setw(10) << "Quantity" 
             << setw(15) << "Expiry Date" 
             << setw(10) << "Price" 
             << "\n";

        cout << setfill('-') 
             << setw(5) << "" << " " 
             << setw(25) << "" << " " 
             << setw(10) << "" << " " 
             << setw(15) << "" << " " 
             << setw(10) << "" 
             << setfill(' ') << "\n";

        for (const auto& med : medicines) {
            cout << left 
                 << setw(5) << med->getId() 
                 << setw(25) << med->getName().substr(0, 24)
                 << setw(10) << med->getQuantity()
                 << setw(15) << med->getExpiryDate()
                 << "$" << fixed << setprecision(2) << med->getPrice()
                 << "\n";
        }

        cout << "\nTotal medicines: " << medicines.size() << "\n";
        Utils::pause();
    }

    void updateMedicine() {
    viewAllMedicines();
    if (medicines.empty()) {
        Utils::pause();
        return;
    }

    int medicineId = Utils::getIntInput("Enter medicine ID to update: ");
    
    auto it = find_if(medicines.begin(), medicines.end(),
        [medicineId](const unique_ptr<IMedicine>& med) {
            return med->getId() == medicineId;
        });

    if (it == medicines.end()) {
        cout << "No medicine found with ID " << medicineId << ".\n";
        Utils::pause();
        return;
    }

    auto& med = *it;
    cout << "Current details:\n";
    med->display();

        int choice;
        cout << "\nWhat would you like to update?\n"
             << "1. Quantity\n"
             << "2. Expiry Date\n"
             << "3. Price\n"
             << "4. Cancel\n"
             << "Enter your choice: ";
        choice = Utils::getIntInput("");

        try {
            switch (choice) {
                case 1: {
                    int newQty = 0;
                    bool valid = false;
                    while (!valid) {
                        newQty = Utils::getIntInput("Enter new quantity: ");
                        valid = (newQty >= 0);
                        if (!valid) {
                            cout << "Quantity cannot be negative.\n";
                        }
                    }
                    med->setQuantity(newQty);
                    break;
                }
                case 2: {
                    string newExpiry = Utils::getDateInput("Enter new expiry date");
                    med->setExpiryDate(newExpiry);
                    break;
                }
                case 3: {
                    float newPrice = 0.0f;
                    bool valid = false;
                    while (!valid) {
                        newPrice = Utils::getFloatInput("Enter new price: ");
                        valid = (newPrice > 0);
                        if (!valid) {
                            cout << "Price must be positive.\n";
                        }
                    }
                    med->setPrice(newPrice);
                    break;
                }
                case 4: return;
                default: cout << "Invalid choice.\n"; Utils::pause(); return;
            }

            saveMedicines();
            cout << "Medicine updated successfully.\n";
            FileLogger::getInstance()->log("Updated medicine: " + med->getName(), currentUser);
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
        Utils::pause();
    }

    void deleteMedicine() {
    viewAllMedicines();
    if (medicines.empty()) {
        Utils::pause();
        return;
    }

    int medicineId = Utils::getIntInput("Enter medicine ID to delete: ");
    
    auto it = find_if(medicines.begin(), medicines.end(),
        [medicineId](const unique_ptr<IMedicine>& med) {
            return med->getId() == medicineId;
        });

    if (it == medicines.end()) {
        cout << "No medicine found with ID " << medicineId << ".\n";
        Utils::pause();
        return;
    }

    string medName = (*it)->getName();
    medicines.erase(it);
    saveMedicines();
    cout << "Medicine " << medName << " (ID: " << medicineId << ") deleted successfully.\n";
    FileLogger::getInstance()->log("Deleted medicine: " + medName + " (ID: " + to_string(medicineId) + ")", currentUser);
    Utils::pause();
}

    void pharmacistMenu() {
        int choice = 0;
        bool running = true;
        
        while (running) {
            Utils::clearScreen();
            cout << "=== PHARMACIST MENU ===\n"
                 << "1. Prescription Management\n"
                 << "2. Process Billing\n"
                 << "3. View Medicines\n"
                 << "4. Logout\n"
                 << "Enter your choice: ";
            choice = Utils::getIntInput("");

            switch (choice) {
                case 1: prescriptionManagementMenu(); break;
                case 2: processBilling(); break;
                case 3: viewAllMedicines(); break;
                case 4: running = false; break;
                default: cout << "Invalid choice. Please try again.\n"; Utils::pause();
            }
        }
    }

    void prescriptionManagementMenu() {
        int choice = 0;
        bool running = true;
        
        while (running) {
            Utils::clearScreen();
            cout << "=== PRESCRIPTION MANAGEMENT ===\n"
                 << "1. Add Prescription\n"
                 << "2. View All Prescriptions\n"
                 << "3. Update Prescription\n"
                 << "4. Delete Prescription\n"
                 << "5. Back to Pharmacist Menu\n"
                 << "Enter your choice: ";
            choice = Utils::getIntInput("");

            switch (choice) {
                case 1: addPrescription(); break;
                case 2: viewAllPrescriptions(); break;
                case 3: updatePrescription(); break;
                case 4: deletePrescription(); break;
                case 5: running = false; break;
                default: cout << "Invalid choice. Please try again.\n"; Utils::pause();
            }
        }
    }

    void addPrescription() {
        Utils::clearScreen();
        cout << "=== ADD NEW PRESCRIPTION ===\n";

        string id;
        bool idValid = false;
        while (!idValid) {
            id = Utils::getInput("Enter prescription ID: ");
            idValid = !id.empty();
            if (!idValid) {
                cout << "ID cannot be empty.\n";
            }
        }

        string patientName;
        bool nameValid = false;
        while (!nameValid) {
            patientName = Utils::getInput("Enter patient name: ");
            nameValid = !patientName.empty();
            if (!nameValid) {
                cout << "Patient name cannot be empty.\n";
            }
        }

        viewAllMedicines();
        if (medicines.empty()) {
            cout << "No medicines available to prescribe.\n";
            Utils::pause();
            return;
        }

        string medicineName;
        bool medicineExists = false;
        int availableStock = 0;
        
        while (!medicineExists) {
            medicineName = Utils::getInput("Enter medicine name: ");
            for (const auto& med : medicines) {
                if (Utils::toLower(med->getName()) == Utils::toLower(medicineName)) {
                    medicineExists = true;
                    availableStock = med->getQuantity();
                    break;
                }
            }
            if (!medicineExists) {
                cout << "Medicine not found in inventory. Try again.\n";
            }
        }

        int quantity = 0;
        bool quantityValid = false;
        while (!quantityValid) {
            quantity = Utils::getIntInput("Enter quantity prescribed: ");
            quantityValid = (quantity > 0 && quantity <= availableStock);
            if (!quantityValid) {
                if (quantity <= 0) {
                    cout << "Quantity must be positive.\n";
                } else {
                    cout << "Only " << availableStock << " units available.\n";
                }
            }
        }

        string date = Utils::getDateInput("Enter prescription date");

        string prescribingDoctor;
        bool doctorValid = false;
        while (!doctorValid) {
            prescribingDoctor = Utils::getInput("Enter prescribing doctor's name: ");
            doctorValid = !prescribingDoctor.empty();
            if (!doctorValid) {
                cout << "Doctor's name cannot be empty.\n";
            }
        }

        try {
            prescriptions.push_back(make_unique<Prescription>(id, patientName, medicineName, quantity, date, prescribingDoctor));
            cout << "\nPrescription added successfully!\n";
            savePrescriptions();
            FileLogger::getInstance()->log("Added prescription ID: " + id, currentUser);
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
        Utils::pause();
    }

    void viewAllPrescriptions() {
        Utils::clearScreen();
        cout << "=== ALL PRESCRIPTIONS ===\n";
        if (prescriptions.empty()) {
            cout << "No prescriptions found.\n";
        } else {
            for (size_t i = 0; i < prescriptions.size(); i++) {
                cout << "Prescription #" << i + 1 << ":\n";
                prescriptions[i]->display();
                cout << "-----------------\n";
            }
        }
        Utils::pause();
    }

    void updatePrescription() {
        viewAllPrescriptions();
        if (prescriptions.empty()) {
            Utils::pause();
            return;
        }

        int index = Utils::getIntInput("Enter prescription number to update: ") - 1;
        if (index < 0 || index >= static_cast<int>(prescriptions.size())) {
            cout << "Invalid prescription number.\n";
            Utils::pause();
            return;
        }

        auto& pres = prescriptions[index];
        cout << "Current details:\n";
        pres->display();

        int choice;
        cout << "\nWhat would you like to update?\n"
             << "1. Patient Name\n"
             << "2. Medicine\n"
             << "3. Quantity\n"
             << "4. Date\n"
             << "5. Doctor\n"
             << "6. Cancel\n"
             << "Enter your choice: ";
        choice = Utils::getIntInput("");

        try {
            switch (choice) {
                case 1: {
                    string newName;
                    bool valid = false;
                    while (!valid) {
                        newName = Utils::getInput("Enter new patient name: ");
                        valid = !newName.empty();
                        if (!valid) {
                            cout << "Name cannot be empty.\n";
                        }
                    }
                    // Would need a setter in Prescription class
                    cout << "Update functionality not fully implemented.\n";
                    break;
                }
                case 2: {
                    viewAllMedicines();
                    string newMed;
                    bool medicineExists = false;
                    while (!medicineExists) {
                        newMed = Utils::getInput("Enter new medicine name: ");
                        for (const auto& med : medicines) {
                            if (Utils::toLower(med->getName()) == Utils::toLower(newMed)) {
                                medicineExists = true;
                                break;
                            }
                        }
                        if (!medicineExists) {
                            cout << "Medicine not found in inventory. Try again.\n";
                        }
                    }
                    cout << "Update functionality not fully implemented.\n";
                    break;
                }
                case 3: {
                    int newQty = 0;
                    bool valid = false;
                    while (!valid) {
                        newQty = Utils::getIntInput("Enter new quantity: ");
                        valid = (newQty > 0);
                        if (!valid) {
                            cout << "Quantity must be positive.\n";
                        }
                    }
                    cout << "Update functionality not fully implemented.\n";
                    break;
                }
                case 4: {
                    string newDate = Utils::getDateInput("Enter new prescription date");
                    cout << "Update functionality not fully implemented.\n";
                    break;
                }
                case 5: {
                    string newDoctor;
                    bool valid = false;
                    while (!valid) {
                        newDoctor = Utils::getInput("Enter new doctor's name: ");
                        valid = !newDoctor.empty();
                        if (!valid) {
                            cout << "Doctor's name cannot be empty.\n";
                        }
                    }
                    cout << "Update functionality not fully implemented.\n";
                    break;
                }
                case 6: return;
                default: cout << "Invalid choice.\n"; Utils::pause(); return;
            }

            savePrescriptions();
            cout << "Prescription updated successfully.\n";
            FileLogger::getInstance()->log("Updated prescription ID: " + pres->getId(), currentUser);
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
        Utils::pause();
    }

    void deletePrescription() {
        viewAllPrescriptions();
        if (prescriptions.empty()) {
            Utils::pause();
            return;
        }

        int index = Utils::getIntInput("Enter prescription number to delete: ") - 1;
        if (index < 0 || index >= static_cast<int>(prescriptions.size())) {
            cout << "Invalid prescription number.\n";
            Utils::pause();
            return;
        }

        string presId = prescriptions[index]->getId();
        prescriptions.erase(prescriptions.begin() + index);
        savePrescriptions();
        cout << "Prescription deleted successfully.\n";
        FileLogger::getInstance()->log("Deleted prescription ID: " + presId, currentUser);
        Utils::pause();
    }

    void processBilling() {
        Utils::clearScreen();
        cout << "=== PROCESS BILLING ===\n";

        viewAllPrescriptions();
        if (prescriptions.empty()) {
            Utils::pause();
            return;
        }

        int index = Utils::getIntInput("Enter prescription number to bill: ") - 1;
        if (index < 0 || index >= static_cast<int>(prescriptions.size())) {
            cout << "Invalid prescription number.\n";
            Utils::pause();
            return;
        }

        auto& pres = prescriptions[index];
        string medicineName = pres->getMedicineName();
        int quantity = pres->getQuantity();

        auto medicineIt = find_if(medicines.begin(), medicines.end(),
            [&medicineName](const unique_ptr<IMedicine>& med) {
                return Utils::toLower(med->getName()) == Utils::toLower(medicineName);
            });

        if (medicineIt == medicines.end()) {
            cout << "Medicine not found in inventory.\n";
            Utils::pause();
            return;
        }

        if ((*medicineIt)->getQuantity() < quantity) {
            cout << "Error: Only " << (*medicineIt)->getQuantity() << " units available.\n";
            Utils::pause();
            return;
        }

        float total = (*medicineIt)->getPrice() * quantity;
        cout << "\n=== BILLING DETAILS ===\n"
             << "Medicine: " << (*medicineIt)->getName() << "\n"
             << "Quantity: " << quantity << "\n"
             << "Price per unit: $" << fixed << setprecision(2) << (*medicineIt)->getPrice() << "\n"
             << "Total: $" << fixed << setprecision(2) << total << "\n\n";

        unique_ptr<IBillingStrategy> strategy;
        int method = Utils::getIntInput("Select payment method:\n1. Cash\n2. GCash\n3. PayMaya\nEnter choice: ");
        switch (method) {
            case 1: strategy = make_unique<CashBilling>(); break;
            case 2: strategy = make_unique<GCashBilling>(); break;
            case 3: strategy = make_unique<PayMayaBilling>(); break;
            default: 
                cout << "Invalid payment method.\n";
                Utils::pause();
                return;
        }

        if (strategy->processPayment(total)) {
            (*medicineIt)->setQuantity((*medicineIt)->getQuantity() - quantity);
            saveMedicines();
            
            FileLogger::getInstance()->log(
                "Billed " + (*medicineIt)->getName() + " x" + to_string(quantity) + 
                ", Remaining: " + to_string((*medicineIt)->getQuantity()) + 
                ", Method: " + strategy->getName(),
                currentUser
            );
            
            cout << "\nTransaction completed successfully!\n";
        } else {
            cout << "\nPayment failed. Transaction cancelled.\n";
        }
        Utils::pause();
    }

public:
    PharmacySystem() {
        loadMedicines();
        loadPrescriptions();
    }

    ~PharmacySystem() override = default;

    void run() override {
        bool programRunning = true;
        
        while (programRunning) {
            bool authenticated = false;
            
            // Authentication loop
            while (!authenticated) {
                if (authenticateUser()) {
                    authenticated = true;
                } else {
                    string choice = Utils::getInput("Would you like to try again? (y/n): ");
                    if (choice != "y" && choice != "Y") {
                        programRunning = false;
                        break;  // Exit both loops
                    }
                }
            }
            
            if (!programRunning) break;
            
            // Main menu loop
            bool sessionActive = true;
            while (sessionActive) {
                if (currentRole == "Admin") {
                    adminMenu();
                } else {
                    pharmacistMenu();
                }
                
                // User chose to logout
                cout << "Logging out... tip: Be sure to save your work and adhere to pharmacy policy\n";
                FileLogger::getInstance()->log("Logged out", currentUser);
                
                // Prompt for relogin or exit
                string choice;
                do {
                    choice = Utils::getInput("Would you like to log in again? (y/n): ");
                } while (choice != "y" && choice != "Y" && choice != "n" && choice != "N");
                
                if (choice == "n" || choice == "N") {
                    sessionActive = false;
                    programRunning = false;
                } else {
                    sessionActive = false;  // Break inner loop to go back to authentication
                }
            }
        }
        
        cout << "Thank you for using the Pharmacy Management System. Goodbye!\n";
    }
};

int main() {
    unique_ptr<IPharmacySystem> system = make_unique<PharmacySystem>();
    system->run();
     return 0;
}

