#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <conio.h>
#include <climits>
#include <cctype>

using namespace std;

// Constants
const int MAX_USERS = 50;
const int MAX_MEDICINES = 500;
const int MAX_PRESCRIPTIONS = 1000;
const int MAX_TRANSACTIONS = 2000;
const int MAX_STRING_LENGTH = 100;

// Helper functions
void clearScreen() {
    system("cls");
}

void pauseScreen() {
    cout << "\nPress any key to continue...";
    getch();
}

int getSingleDigitInput(const char* prompt, int min = 1, int max = 9) {
    char buffer[10];
    int value;
    
    do {
        cout << prompt;
        cin.getline(buffer, 10);
        
        // Validate single character and is digit
        if (strlen(buffer) != 1 || !isdigit(buffer[0])) {
            cout << "Invalid input. Please enter a single digit between " 
                 << min << " and " << max << ".\n";
            continue;
        }
        
        value = buffer[0] - '0';
        
        // Validate range
        if (value < min || value > max) {
            cout << "Invalid choice. Please enter a number between " 
                 << min << " and " << max << ".\n";
        }
    } while (strlen(buffer) != 1 || !isdigit(buffer[0]) || value < min || value > max);
    
    return value;
}

int getIntInput(const char* prompt, int min = INT_MIN, int max = INT_MAX) {
    int value;
    cout << prompt;
    while (!(cin >> value) || value < min || value > max) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input. Please enter a number between " << min << " and " << max << ": ";
    }
    cin.ignore(1000, '\n');
    return value;
}

void getStringInput(const char* prompt, char* buffer, int maxLength) {
    cout << prompt;
    cin.getline(buffer, maxLength);
    while (strlen(buffer) == 0) {
        cout << "Input cannot be empty. Please try again: ";
        cin.getline(buffer, maxLength);
    }
}

// Date structure
struct Date {
    int day;
    int month;
    int year;
    
    bool isValid() const {
        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        
        int maxDays = 31;
        if (month == 4 || month == 6 || month == 9 || month == 11) {
            maxDays = 30;
        } else if (month == 2) {
            maxDays = (year % 400 == 0 || (year % 100 != 0 && year % 4 == 0)) ? 29 : 28;
        }
        
        return day >= 1 && day <= maxDays;
    }
    
    bool isExpired() const {
        time_t now = time(0);
        tm* current = localtime(&now);
        
        if (year < current->tm_year + 1900) return true;
        if (year == current->tm_year + 1900 && month < current->tm_mon + 1) return true;
        if (year == current->tm_year + 1900 && month == current->tm_mon + 1 && day < current->tm_mday) return true;
        return false;
    }
    
    void print() const {
        printf("%02d/%02d/%04d", day, month, year);
    }
};

// User roles
enum UserRole {
    ROLE_ADMIN,
    ROLE_PHARMACIST
};

// User structure
struct User {
    char username[MAX_STRING_LENGTH];
    char password[MAX_STRING_LENGTH];
    UserRole role;
    
    bool authenticate(const char* uname, const char* pwd) const {
        return strcmp(username, uname) == 0 && strcmp(password, pwd) == 0;
    }
};

// Medicine structure
struct Medicine {
    int id;
    char name[MAX_STRING_LENGTH];
    char description[MAX_STRING_LENGTH];
    int quantity;
    double price;
    Date expiryDate;
    bool isControlled;
    
    void print() const {
        cout << "ID: " << id << "\n";
        cout << "Name: " << name << "\n";
        cout << "Description: " << description << "\n";
        cout << "Quantity: " << quantity << "\n";
        cout << "Price: P" << price << "\n";
        cout << "Expiry Date: ";
        expiryDate.print();
        cout << "\n";
        cout << "Controlled Substance: " << (isControlled ? "Yes" : "No") << "\n";
    }
};

// Prescription structure
struct Prescription {
    int id;
    char patientName[MAX_STRING_LENGTH];
    char doctorName[MAX_STRING_LENGTH];
    Date date;
    int medicineIds[10];
    int quantities[10];
    int medicineCount;
    bool isFilled;
    
    void print() const {
        cout << "Prescription ID: " << id << "\n";
        cout << "Patient: " << patientName << "\n";
        cout << "Doctor: " << doctorName << "\n";
        cout << "Date: ";
        date.print();
        cout << "\n";
        cout << "Status: " << (isFilled ? "Filled" : "Pending") << "\n";
        cout << "Medicines:\n";
        for (int i = 0; i < medicineCount; i++) {
            cout << "  - Medicine ID: " << medicineIds[i] << ", Quantity: " << quantities[i] << "\n";
        }
    }
};

// Payment types
enum PaymentType {
    PAYMENT_CASH,
    PAYMENT_GCASH,
    PAYMENT_PAYMAYA
};

// Transaction structure
struct Transaction {
    int id;
    Date date;
    int prescriptionId;
    double totalAmount;
    PaymentType paymentType;
    char paymentDetails[MAX_STRING_LENGTH];
    
    void print() const {
        cout << "Transaction ID: " << id << "\n";
        cout << "Date: ";
        date.print();
        cout << "\n";
        cout << "Prescription ID: " << prescriptionId << "\n";
        cout << "Total Amount: P" << totalAmount << "\n";
        cout << "Payment Type: ";
        switch (paymentType) {
            case PAYMENT_CASH: cout << "Cash"; break;
            case PAYMENT_GCASH: cout << "GCash"; break;
            case PAYMENT_PAYMAYA: cout << "PayMaya"; break;
        }
        cout << "\n";
        cout << "Payment Details: " << paymentDetails << "\n";
    }
};

// Database structure
struct Database {
    User users[MAX_USERS];
    int userCount;
    
    Medicine medicines[MAX_MEDICINES];
    int medicineCount;
    
    Prescription prescriptions[MAX_PRESCRIPTIONS];
    int prescriptionCount;
    
    Transaction transactions[MAX_TRANSACTIONS];
    int transactionCount;
    
    Database() : userCount(0), medicineCount(0), prescriptionCount(0), transactionCount(0) {
        // Initialize with admin user
        strcpy(users[0].username, "admin");
        strcpy(users[0].password, "admin123");
        users[0].role = ROLE_ADMIN;
        userCount++;
        
        // Initialize with pharmacist user
        strcpy(users[1].username, "pharmacist");
        strcpy(users[1].password, "pharma123");
        users[1].role = ROLE_PHARMACIST;
        userCount++;
    }
};

// Global database
Database db;

// Forward declarations
void adminMenu();
void pharmacistMenu();
void login();
void manageMedicines();
void managePrescriptions();
void processBilling();
void generateReports();
void addMedicine();
void viewMedicines();
void updateMedicine();
void deleteMedicine();
void addPrescription();
void viewPrescriptions();
void updatePrescription();
void deletePrescription();
void fillPrescription();
void saveData();
void loadData();

// Main function
int main() {
    loadData();
    login();
    saveData();
    return 0;
}

// Login system
void login() {
    char username[MAX_STRING_LENGTH];
    char password[MAX_STRING_LENGTH];
    int attempts = 0;
    const int maxAttempts = 3;
    
    do {
        clearScreen();
        cout << "PHARMACY MANAGEMENT SYSTEM\n";
        cout << "-------------------------\n\n";
        
        getStringInput("Username: ", username, MAX_STRING_LENGTH);
        
        cout << "Password: ";
        int i = 0;
        char ch;
        while ((ch = getch()) != 13 && i < MAX_STRING_LENGTH - 1) {
            if (ch == 8 && i > 0) {
                i--;
                cout << "\b \b";
            } else if (ch >= 32 && ch <= 126) {
                password[i] = ch;
                i++;
                cout << "*";
            }
        }
        password[i] = '\0';
        cout << "\n";
        
        for (int i = 0; i < db.userCount; i++) {
            if (db.users[i].authenticate(username, password)) {
                cout << "\nLogin successful!\n";
                pauseScreen();
                
                if (db.users[i].role == ROLE_ADMIN) {
                    adminMenu();
                } else {
                    pharmacistMenu();
                }
                return;
            }
        }
        
        cout << "\nInvalid username or password.\n";
        attempts++;
        if (attempts < maxAttempts) {
            cout << attempts << " of " << maxAttempts << " attempts used.\n";
            pauseScreen();
        }
    } while (attempts < maxAttempts);
    
    cout << "Maximum login attempts reached. Exiting...\n";
    pauseScreen();
}

// Admin menu
void adminMenu() {
    int choice = 0;
    do {
        clearScreen();
        cout << "ADMIN MENU\n";
        cout << "----------\n";
        cout << "1. Manage Medicines\n";
        cout << "2. Generate Reports\n";
        cout << "3. Logout\n";
        
        choice = getSingleDigitInput("Enter your choice (1-3): ", 1, 3);
        
        switch (choice) {
            case 1: manageMedicines(); break;
            case 2: generateReports(); break;
            case 3: return;
        }
    } while (choice != 3);
}

// Pharmacist menu
void pharmacistMenu() {
    int choice = 0;
    do {
        clearScreen();
        cout << "PHARMACIST MENU\n";
        cout << "---------------\n";
        cout << "1. Manage Prescriptions\n";
        cout << "2. Process Billing\n";
        cout << "3. View Medicines\n";
        cout << "4. Logout\n";
        
        choice = getSingleDigitInput("Enter your choice (1-4): ", 1, 4);
        
        switch (choice) {
            case 1: managePrescriptions(); break;
            case 2: processBilling(); break;
            case 3: viewMedicines(); break;
            case 4: return;
        }
    } while (choice != 4);
}

// Medicine management
void manageMedicines() {
    int choice = 0;
    do {
        clearScreen();
        cout << "MEDICINE MANAGEMENT\n";
        cout << "-------------------\n";
        cout << "1. Add Medicine\n";
        cout << "2. View Medicines\n";
        cout << "3. Update Medicine\n";
        cout << "4. Delete Medicine\n";
        cout << "5. Back to Main Menu\n";
        
        choice = getSingleDigitInput("Enter your choice (1-5): ", 1, 5);
        
        switch (choice) {
            case 1: addMedicine(); break;
            case 2: viewMedicines(); break;
            case 3: updateMedicine(); break;
            case 4: deleteMedicine(); break;
            case 5: return;
        }
    } while (choice != 5);
}

// Add medicine
void addMedicine() {
    clearScreen();
    cout << "ADD NEW MEDICINE\n";
    cout << "----------------\n";
    
    if (db.medicineCount >= MAX_MEDICINES) {
        cout << "Maximum number of medicines reached!\n";
        pauseScreen();
        return;
    }
    
    Medicine& med = db.medicines[db.medicineCount];
    med.id = db.medicineCount > 0 ? db.medicines[db.medicineCount-1].id + 1 : 1;
    
    getStringInput("Medicine Name: ", med.name, MAX_STRING_LENGTH);
    getStringInput("Description: ", med.description, MAX_STRING_LENGTH);
    
    med.quantity = getIntInput("Quantity: ", 0);
    med.price = getIntInput("Price : ", 0);
    
    do {
        med.expiryDate.day = getIntInput("Expiry Day (1-31): ", 1, 31);
        med.expiryDate.month = getIntInput("Expiry Month (1-12): ", 1, 12);
        med.expiryDate.year = getIntInput("Expiry Year (1900-2100): ", 1900, 2100);
        
        if (!med.expiryDate.isValid()) {
            cout << "Invalid date. Please try again.\n";
        }
    } while (!med.expiryDate.isValid());
    
    char controlled;
    do {
        cout << "Is this a controlled substance? (y/n): ";
        cin >> controlled;
        cin.ignore(1000, '\n');
    } while (controlled != 'y' && controlled != 'Y' && controlled != 'n' && controlled != 'N');
    
    med.isControlled = (controlled == 'y' || controlled == 'Y');
    
    db.medicineCount++;
    cout << "\nMedicine added successfully!\n";
    pauseScreen();
}

// View medicines
void viewMedicines() {
    clearScreen();
    cout << "MEDICINE INVENTORY\n";
    cout << "------------------\n";
    
    if (db.medicineCount == 0) {
        cout << "No medicines in inventory.\n";
        pauseScreen();
        return;
    }
    
    for (int i = 0; i < db.medicineCount; i++) {
        db.medicines[i].print();
        cout << "------------------\n";
    }
    
    pauseScreen();
}

// Update medicine
void updateMedicine() {
    clearScreen();
    cout << "UPDATE MEDICINE\n";
    cout << "---------------\n";
    
    if (db.medicineCount == 0) {
        cout << "No medicines to update.\n";
        pauseScreen();
        return;
    }
    
    int id = getIntInput("Enter Medicine ID to update: ", 1);
    
    int index = -1;
    for (int i = 0; i < db.medicineCount; i++) {
        if (db.medicines[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        cout << "Medicine not found.\n";
        pauseScreen();
        return;
    }
    
    Medicine& med = db.medicines[index];
    med.print();
    cout << "\n";
    
    cout << "Enter new details (leave blank to keep current value):\n";
    
    char buffer[MAX_STRING_LENGTH];
    getStringInput("Medicine Name: ", buffer, MAX_STRING_LENGTH);
    if (strlen(buffer) > 0) {
        strcpy(med.name, buffer);
    }
    
    getStringInput("Description: ", buffer, MAX_STRING_LENGTH);
    if (strlen(buffer) > 0) {
        strcpy(med.description, buffer);
    }
    
    int quantity = getIntInput("Quantity (-1 to keep current): ", -1);
    if (quantity >= 0) {
        med.quantity = quantity;
    }
    
    int price = getIntInput("Price in cents (-1 to keep current): ", -1);
    if (price >= 0) {
        med.price = price / 100.0;
    }
    
    char choice;
    cout << "Update expiry date? (y/n): ";
    cin >> choice;
    cin.ignore(1000, '\n');
    if (choice == 'y' || choice == 'Y') {
        do {
            med.expiryDate.day = getIntInput("Expiry Day (1-31): ", 1, 31);
            med.expiryDate.month = getIntInput("Expiry Month (1-12): ", 1, 12);
            med.expiryDate.year = getIntInput("Expiry Year (1900-2100): ", 1900, 2100);
            
            if (!med.expiryDate.isValid()) {
                cout << "Invalid date. Please try again.\n";
            }
        } while (!med.expiryDate.isValid());
    }
    
    cout << "Update controlled substance status? (y/n): ";
    cin >> choice;
    cin.ignore(1000, '\n');
    if (choice == 'y' || choice == 'Y') {
        do {
            cout << "Is this a controlled substance? (y/n): ";
            cin >> choice;
            cin.ignore(1000, '\n');
        } while (choice != 'y' && choice != 'Y' && choice != 'n' && choice != 'N');
        med.isControlled = (choice == 'y' || choice == 'Y');
    }
    
    cout << "\nMedicine updated successfully!\n";
    pauseScreen();
}

// Delete medicine
void deleteMedicine() {
    clearScreen();
    cout << "DELETE MEDICINE\n";
    cout << "---------------\n";
    
    if (db.medicineCount == 0) {
        cout << "No medicines to delete.\n";
        pauseScreen();
        return;
    }
    
    int id = getIntInput("Enter Medicine ID to delete: ", 1);
    
    int index = -1;
    for (int i = 0; i < db.medicineCount; i++) {
        if (db.medicines[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        cout << "Medicine not found.\n";
        pauseScreen();
        return;
    }
    
    // Confirm deletion
    char confirm;
    cout << "Are you sure you want to delete this medicine? (y/n): ";
    cin >> confirm;
    cin.ignore(1000, '\n');
    
    if (confirm == 'y' || confirm == 'Y') {
        // Shift all elements after index one position left
        for (int i = index; i < db.medicineCount - 1; i++) {
            db.medicines[i] = db.medicines[i + 1];
        }
        db.medicineCount--;
        cout << "Medicine deleted successfully!\n";
    } else {
        cout << "Deletion canceled.\n";
    }
    
    pauseScreen();
}

// Prescription management
void managePrescriptions() {
    int choice = 0;
    do {
        clearScreen();
        cout << "PRESCRIPTION MANAGEMENT\n";
        cout << "-----------------------\n";
        cout << "1. Add Prescription\n";
        cout << "2. View Prescriptions\n";
        cout << "3. Update Prescription\n";
        cout << "4. Delete Prescription\n";
        cout << "5. Fill Prescription\n";
        cout << "6. Back to Main Menu\n";
        
        choice = getSingleDigitInput("Enter your choice (1-6): ", 1, 6);
        
        switch (choice) {
            case 1: addPrescription(); break;
            case 2: viewPrescriptions(); break;
            case 3: updatePrescription(); break;
            case 4: deletePrescription(); break;
            case 5: fillPrescription(); break;
            case 6: return;
        }
    } while (choice != 6);
}

// Add prescription
void addPrescription() {
    clearScreen();
    cout << "ADD NEW PRESCRIPTION\n";
    cout << "--------------------\n";
    
    if (db.medicineCount == 0) {
        cout << "Cannot add prescription - no medicines available in system.\n";
        cout << "Please add medicines first.\n";
        pauseScreen();
        return;
    }
    
    if (db.prescriptionCount >= MAX_PRESCRIPTIONS) {
        cout << "Maximum number of prescriptions reached!\n";
        pauseScreen();
        return;
    }
    
    Prescription& pres = db.prescriptions[db.prescriptionCount];
    pres.id = db.prescriptionCount > 0 ? db.prescriptions[db.prescriptionCount-1].id + 1 : 1;
    
    getStringInput("Patient Name: ", pres.patientName, MAX_STRING_LENGTH);
    getStringInput("Doctor Name: ", pres.doctorName, MAX_STRING_LENGTH);
    
    do {
        pres.date.day = getIntInput("Date Day (1-31): ", 1, 31);
        pres.date.month = getIntInput("Date Month (1-12): ", 1, 12);
        pres.date.year = getIntInput("Date Year (1900-2100): ", 1900, 2100);
        
        if (!pres.date.isValid()) {
            cout << "Invalid date. Please try again.\n";
        }
    } while (!pres.date.isValid());
    
    pres.medicineCount = getIntInput("Number of medicines in prescription (1-10): ", 1, 10);
    
    for (int i = 0; i < pres.medicineCount; i++) {
        cout << "\nMedicine #" << i + 1 << ":\n";
        int medicineId = getIntInput("Medicine ID: ", 1);
        
        // Check if medicine exists
        int medIndex = -1;
        for (int j = 0; j < db.medicineCount; j++) {
            if (db.medicines[j].id == medicineId) {
                medIndex = j;
                break;
            }
        }
        
        if (medIndex == -1) {
            cout << "Medicine not found. Please try again.\n";
            i--;
            continue;
        }
        
        pres.medicineIds[i] = medicineId;
        pres.quantities[i] = getIntInput("Quantity: ", 1, db.medicines[medIndex].quantity);
    }
    
    pres.isFilled = false;
    db.prescriptionCount++;
    cout << "\nPrescription added successfully!\n";
    pauseScreen();
}

// View prescriptions
void viewPrescriptions() {
    clearScreen();
    cout << "PRESCRIPTION LIST\n";
    cout << "-----------------\n";
    
    if (db.prescriptionCount == 0) {
        cout << "No prescriptions available.\n";
        pauseScreen();
        return;
    }
    
    for (int i = 0; i < db.prescriptionCount; i++) {
        db.prescriptions[i].print();
        cout << "-----------------\n";
    }
    
    pauseScreen();
}

// Update prescription
void updatePrescription() {
    clearScreen();
    cout << "UPDATE PRESCRIPTION\n";
    cout << "------------------\n";
    
    if (db.prescriptionCount == 0) {
        cout << "No prescriptions to update.\n";
        pauseScreen();
        return;
    }
    
    int id = getIntInput("Enter Prescription ID to update: ", 1);
    
    int index = -1;
    for (int i = 0; i < db.prescriptionCount; i++) {
        if (db.prescriptions[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        cout << "Prescription not found.\n";
        pauseScreen();
        return;
    }
    
    Prescription& pres = db.prescriptions[index];
    if (pres.isFilled) {
        cout << "Cannot update a filled prescription.\n";
        pauseScreen();
        return;
    }
    
    pres.print();
    cout << "\n";
    
    cout << "Enter new details (leave blank to keep current value):\n";
    
    char buffer[MAX_STRING_LENGTH];
    getStringInput("Patient Name: ", buffer, MAX_STRING_LENGTH);
    if (strlen(buffer) > 0) {
        strcpy(pres.patientName, buffer);
    }
    
    getStringInput("Doctor Name: ", buffer, MAX_STRING_LENGTH);
    if (strlen(buffer) > 0) {
        strcpy(pres.doctorName, buffer);
    }
    
    char choice;
    cout << "Update date? (y/n): ";
    cin >> choice;
    cin.ignore(1000, '\n');
    if (choice == 'y' || choice == 'Y') {
        do {
            pres.date.day = getIntInput("Date Day (1-31): ", 1, 31);
            pres.date.month = getIntInput("Date Month (1-12): ", 1, 12);
            pres.date.year = getIntInput("Date Year (1900-2100): ", 1900, 2100);
            
            if (!pres.date.isValid()) {
                cout << "Invalid date. Please try again.\n";
            }
        } while (!pres.date.isValid());
    }
    
    cout << "Update medicines? (y/n): ";
    cin >> choice;
    cin.ignore(1000, '\n');
    if (choice == 'y' || choice == 'Y') {
        pres.medicineCount = getIntInput("Number of medicines in prescription (1-10): ", 1, 10);
        
        for (int i = 0; i < pres.medicineCount; i++) {
            cout << "\nMedicine #" << i + 1 << ":\n";
            int medicineId = getIntInput("Medicine ID: ", 1);
            
            // Check if medicine exists
            int medIndex = -1;
            for (int j = 0; j < db.medicineCount; j++) {
                if (db.medicines[j].id == medicineId) {
                    medIndex = j;
                    break;
                }
            }
            
            if (medIndex == -1) {
                cout << "Medicine not found. Please try again.\n";
                i--;
                continue;
            }
            
            pres.medicineIds[i] = medicineId;
            pres.quantities[i] = getIntInput("Quantity: ", 1, db.medicines[medIndex].quantity);
        }
    }
    
    cout << "\nPrescription updated successfully!\n";
    pauseScreen();
}

// Delete prescription
void deletePrescription() {
    clearScreen();
    cout << "DELETE PRESCRIPTION\n";
    cout << "------------------\n";
    
    if (db.prescriptionCount == 0) {
        cout << "No prescriptions to delete.\n";
        pauseScreen();
        return;
    }
    
    int id = getIntInput("Enter Prescription ID to delete: ", 1);
    
    int index = -1;
    for (int i = 0; i < db.prescriptionCount; i++) {
        if (db.prescriptions[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        cout << "Prescription not found.\n";
        pauseScreen();
        return;
    }
    
    // Confirm deletion
    char confirm;
    cout << "Are you sure you want to delete this prescription? (y/n): ";
    cin >> confirm;
    cin.ignore(1000, '\n');
    
    if (confirm == 'y' || confirm == 'Y') {
        // Shift all elements after index one position left
        for (int i = index; i < db.prescriptionCount - 1; i++) {
            db.prescriptions[i] = db.prescriptions[i + 1];
        }
        db.prescriptionCount--;
        cout << "Prescription deleted successfully!\n";
    } else {
        cout << "Deletion canceled.\n";
    }
    
    pauseScreen();
}

// Fill prescription
void fillPrescription() {
    clearScreen();
    cout << "FILL PRESCRIPTION\n";
    cout << "----------------\n";
    
    if (db.medicineCount == 0) {
        cout << "Cannot fill prescriptions - no medicines available in system.\n";
        cout << "Please add medicines first.\n";
        pauseScreen();
        return;
    }
    
    if (db.prescriptionCount == 0) {
        cout << "No prescriptions available.\n";
        pauseScreen();
        return;
    }
    
    int id = getIntInput("Enter Prescription ID to fill: ", 1);
    
    int index = -1;
    for (int i = 0; i < db.prescriptionCount; i++) {
        if (db.prescriptions[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        cout << "Prescription not found.\n";
        pauseScreen();
        return;
    }
    
    Prescription& pres = db.prescriptions[index];
    if (pres.isFilled) {
        cout << "Prescription already filled.\n";
        pauseScreen();
        return;
    }
    
    // Check if all medicines are available
    bool canFill = true;
    for (int i = 0; i < pres.medicineCount; i++) {
        int medId = pres.medicineIds[i];
        int reqQty = pres.quantities[i];
        
        int medIndex = -1;
        for (int j = 0; j < db.medicineCount; j++) {
            if (db.medicines[j].id == medId) {
                medIndex = j;
                break;
            }
        }
        
        if (medIndex == -1 || db.medicines[medIndex].quantity < reqQty) {
            cout << "Not enough stock for medicine ID " << medId << " (required: " << reqQty << ")\n";
            canFill = false;
        }
    }
    
    if (!canFill) {
        cout << "Cannot fill prescription due to insufficient stock.\n";
        pauseScreen();
        return;
    }
    
    // Deduct quantities from inventory
    for (int i = 0; i < pres.medicineCount; i++) {
        int medId = pres.medicineIds[i];
        int reqQty = pres.quantities[i];
        
        for (int j = 0; j < db.medicineCount; j++) {
            if (db.medicines[j].id == medId) {
                db.medicines[j].quantity -= reqQty;
                break;
            }
        }
    }
    
    pres.isFilled = true;
    cout << "Prescription filled successfully!\n";
    pauseScreen();
}

// Process billing
void processBilling() {
    clearScreen();
    cout << "PROCESS BILLING\n";
    cout << "---------------\n";
    
    if (db.medicineCount == 0) {
        cout << "Cannot process billing - no medicines available in system.\n";
        cout << "Please add medicines first.\n";
        pauseScreen();
        return;
    }
    
    if (db.prescriptionCount == 0) {
        cout << "No prescriptions available.\n";
        pauseScreen();
        return;
    }
    
    int id = getIntInput("Enter Prescription ID to bill: ", 1);
    
    int presIndex = -1;
    for (int i = 0; i < db.prescriptionCount; i++) {
        if (db.prescriptions[i].id == id) {
            presIndex = i;
            break;
        }
    }
    
    if (presIndex == -1) {
        cout << "Prescription not found.\n";
        pauseScreen();
        return;
    }
    
    Prescription& pres = db.prescriptions[presIndex];
    if (!pres.isFilled) {
        cout << "Prescription not yet filled. Please fill it first.\n";
        pauseScreen();
        return;
    }
    
    // Calculate total amount
    double total = 0.0;
    for (int i = 0; i < pres.medicineCount; i++) {
        int medId = pres.medicineIds[i];
        int qty = pres.quantities[i];
        
        for (int j = 0; j < db.medicineCount; j++) {
            if (db.medicines[j].id == medId) {
                total += db.medicines[j].price * qty;
                break;
            }
        }
    }
    
    cout << "\nPrescription Details:\n";
    pres.print();
    cout << "\nTotal Amount: P" << total << "\n";
    
    // Select payment method
    cout << "\nSelect Payment Method:\n";
    cout << "1. Cash\n";
    cout << "2. GCash\n";
    cout << "3. PayMaya\n";
    
    int paymentChoice = getSingleDigitInput("Enter your choice (1-3): ", 1, 3);
    PaymentType paymentType;
    char paymentDetails[MAX_STRING_LENGTH] = "";
    
    switch (paymentChoice) {
        case 1:
            paymentType = PAYMENT_CASH;
            strcpy(paymentDetails, "Cash payment");
            break;
        case 2:
            paymentType = PAYMENT_GCASH;
            getStringInput("Enter GCash reference number: ", paymentDetails, MAX_STRING_LENGTH);
            break;
        case 3:
            paymentType = PAYMENT_PAYMAYA;
            getStringInput("Enter PayMaya reference number: ", paymentDetails, MAX_STRING_LENGTH);
            break;
    }
    
    // Create transaction
    if (db.transactionCount >= MAX_TRANSACTIONS) {
        cout << "Maximum number of transactions reached!\n";
        pauseScreen();
        return;
    }
    
    Transaction& trans = db.transactions[db.transactionCount];
    trans.id = db.transactionCount > 0 ? db.transactions[db.transactionCount-1].id + 1 : 1;
    
    time_t now = time(0);
    tm* current = localtime(&now);
    trans.date.day = current->tm_mday;
    trans.date.month = current->tm_mon + 1;
    trans.date.year = current->tm_year + 1900;
    
    trans.prescriptionId = pres.id;
    trans.totalAmount = total;
    trans.paymentType = paymentType;
    strcpy(trans.paymentDetails, paymentDetails);
    
    db.transactionCount++;
    
    cout << "\nTransaction completed successfully!\n";
    cout << "Transaction ID: " << trans.id << "\n";
    pauseScreen();
}

// Generate reports
void generateReports() {
    int choice = 0;
    do {
        clearScreen();
        cout << "GENERATE REPORTS\n";
        cout << "----------------\n";
        cout << "1. Low Stock Report\n";
        cout << "2. Expired Medicines Report\n";
        cout << "3. Controlled Substances Report\n";
        cout << "4. Sales Report\n";
        cout << "5. Back to Main Menu\n";
        
        choice = getSingleDigitInput("Enter your choice (1-5): ", 1, 5);
        
        switch (choice) {
            case 1: {
                clearScreen();
                cout << "LOW STOCK REPORT (Quantity < 10)\n";
                cout << "-------------------------------\n";
                
                bool found = false;
                for (int i = 0; i < db.medicineCount; i++) {
                    if (db.medicines[i].quantity < 10) {
                        db.medicines[i].print();
                        cout << "-------------------------------\n";
                        found = true;
                    }
                }
                
                if (!found) {
                    cout << "No medicines with low stock.\n";
                }
                pauseScreen();
                break;
            }
            case 2: {
                clearScreen();
                cout << "EXPIRED MEDICINES REPORT\n";
                cout << "-----------------------\n";
                
                bool found = false;
                for (int i = 0; i < db.medicineCount; i++) {
                    if (db.medicines[i].expiryDate.isExpired()) {
                        db.medicines[i].print();
                        cout << "-----------------------\n";
                        found = true;
                    }
                }
                
                if (!found) {
                    cout << "No expired medicines.\n";
                }
                pauseScreen();
                break;
            }
            case 3: {
                clearScreen();
                cout << "CONTROLLED SUBSTANCES REPORT\n";
                cout << "---------------------------\n";
                
                bool found = false;
                for (int i = 0; i < db.medicineCount; i++) {
                    if (db.medicines[i].isControlled) {
                        db.medicines[i].print();
                        cout << "---------------------------\n";
                        found = true;
                    }
                }
                
                if (!found) {
                    cout << "No controlled substances in inventory.\n";
                }
                pauseScreen();
                break;
            }
            case 4: {
                clearScreen();
                cout << "SALES REPORT\n";
                cout << "------------\n";
                
                if (db.transactionCount == 0) {
                    cout << "No transactions recorded.\n";
                    pauseScreen();
                    break;
                }
                
                double totalSales = 0.0;
                int cashCount = 0, gcashCount = 0, paymayaCount = 0;
                double cashTotal = 0.0, gcashTotal = 0.0, paymayaTotal = 0.0;
                
                for (int i = 0; i < db.transactionCount; i++) {
                    totalSales += db.transactions[i].totalAmount;
                    
                    switch (db.transactions[i].paymentType) {
                        case PAYMENT_CASH:
                            cashCount++;
                            cashTotal += db.transactions[i].totalAmount;
                            break;
                        case PAYMENT_GCASH:
                            gcashCount++;
                            gcashTotal += db.transactions[i].totalAmount;
                            break;
                        case PAYMENT_PAYMAYA:
                            paymayaCount++;
                            paymayaTotal += db.transactions[i].totalAmount;
                            break;
                    }
                }
                
                cout << "Total Sales: P" << totalSales << "\n";
                cout << "Number of Transactions: " << db.transactionCount << "\n";
                cout << "\nPayment Method Breakdown:\n";
                cout << "Cash: " << cashCount << " transactions (P" << cashTotal << ")\n";
                cout << "GCash: " << gcashCount << " transactions (P" << gcashTotal << ")\n";
                cout << "PayMaya: " << paymayaCount << " transactions (P" << paymayaTotal << ")\n";
                
                pauseScreen();
                break;
            }
            case 5:
                return;
        }
    } while (choice != 5);
}

// Save data to file
void saveData() {
    ofstream outFile("pharmacy_data.dat", ios::binary);
    if (!outFile) {
        cout << "Error saving data to file.\n";
        return;
    }
    
    outFile.write((char*)&db, sizeof(Database));
    outFile.close();
}

// Load data from file
void loadData() {
    ifstream inFile("pharmacy_data.dat", ios::binary);
    if (!inFile) {
        cout << "No existing data file found. Starting with default data.\n";
        return;
    }
    
    inFile.read((char*)&db, sizeof(Database));
    inFile.close();
}