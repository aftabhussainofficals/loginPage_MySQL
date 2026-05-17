#include "headers.h"
static string trim(string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' '))
        s.pop_back();
    return s;
}
map<string, string> loadEnv(const string& filepath = ".env") {
    map<string, string> env;
    ifstream file(filepath);
    if (!file.is_open()) return env;
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == string::npos) continue;
        env[trim(line.substr(0, pos))] = trim(line.substr(pos + 1));
    }
    return env;
}
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
void pauseMs(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}
void pressAnyKey() {
    cout << "\n  [Press ENTER to return to menu...]" << flush;
    cin.clear();
    cin.ignore(1000, '\n');
}
void printHeader(const string& title) {
    clearScreen();
    cout << "\n  ===== " << title << " =====\n\n" << flush;
}
void printSuccess(const string& msg) { cout << "\n  [ OK ]  " << msg << "\n" << flush; }
void printError  (const string& msg) { cout << "\n  [FAIL]  " << msg << "\n" << flush; }
void printInfo   (const string& msg) { cout << "\n  [ i  ]  " << msg << "\n" << flush; }
static string esc(MYSQL* conn, const string& s) {
    string buf(s.size() * 2 + 1, '\0');
    buf.resize(mysql_real_escape_string(conn, &buf[0], s.c_str(), (unsigned long)s.size()));
    return buf;
}
static MYSQL_RES* runSelect(MYSQL* conn, const string& sql) {
    if (mysql_query(conn, sql.c_str())) return nullptr;
    return mysql_store_result(conn);
}
static bool runExec(MYSQL* conn, const string& sql) {
    return mysql_query(conn, sql.c_str()) == 0;
}
bool loginUser(MYSQL* conn, const string& username, const string& password) {
    string q = "SELECT id FROM users WHERE username='" + esc(conn, username) +
               "' AND password=SHA2('" + esc(conn, password) + "',256)";
    MYSQL_RES* res = runSelect(conn, q);
    if (!res) return false;
    bool found = (mysql_num_rows(res) > 0);
    mysql_free_result(res);
    return found;
}
bool usernameExists(MYSQL* conn, const string& username) {
    string q = "SELECT id FROM users WHERE username='" + esc(conn, username) + "'";
    MYSQL_RES* res = runSelect(conn, q);
    if (!res) return false;
    bool found = (mysql_num_rows(res) > 0);
    mysql_free_result(res);
    return found;
}
bool emailExists(MYSQL* conn, const string& email) {
    string q = "SELECT id FROM users WHERE email='" + esc(conn, email) + "'";
    MYSQL_RES* res = runSelect(conn, q);
    if (!res) return false;
    bool found = (mysql_num_rows(res) > 0);
    mysql_free_result(res);
    return found;
}
bool signupUser(MYSQL* conn, const string& username, const string& password, const string& email) {
    if (usernameExists(conn, username)) { printError("Username '" + username + "' is already taken."); return false; }
    if (emailExists(conn, email))       { printError("Email '" + email + "' is already registered."); return false; }
    string q = "INSERT INTO users(username,password,email) VALUES('" +
               esc(conn, username) + "',SHA2('" + esc(conn, password) + "',256),'" + esc(conn, email) + "')";
    if (!runExec(conn, q)) { printError("Signup failed: " + string(mysql_error(conn))); return false; }
    printSuccess("Account created for '" + username + "'! You can now log in.");
    return true;
}
bool deleteAccount(MYSQL* conn, const string& username, const string& password) {
    if (!loginUser(conn, username, password)) { printError("Invalid credentials. Cannot delete account."); return false; }
    string q = "DELETE FROM users WHERE username='" + esc(conn, username) + "'";
    if (!runExec(conn, q)) { printError("Delete failed: " + string(mysql_error(conn))); return false; }
    printSuccess("Account '" + username + "' deleted successfully.");
    return true;
}
bool resetPassword(MYSQL* conn, const string& email, const string& newPassword) {
    if (!emailExists(conn, email)) { printError("No account found for '" + email + "'."); return false; }
    string q = "UPDATE users SET password=SHA2('" + esc(conn, newPassword) + "',256) WHERE email='" + esc(conn, email) + "'";
    if (!runExec(conn, q)) { printError("Reset failed: " + string(mysql_error(conn))); return false; }
    printSuccess("Password reset for '" + email + "'.");
    return true;
}
string getPassword(const string& prompt) {
    cout << prompt << flush;
    string pwd;
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b' && !pwd.empty()) { pwd.pop_back(); cout << "\b \b" << flush; }
        else if ((unsigned char)ch >= 32) { pwd += ch; cout << '*' << flush; }
    }
    cout << '\n';
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    getline(cin, pwd);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << '\n';
#endif
    return trim(pwd);
}
string getInput(const string& prompt) {
    cout << prompt << flush;
    string s;
    getline(cin, s);
    return trim(s);
}
bool isValidEmail(const string& e) {
    size_t at = e.find('@'), dot = e.rfind('.');
    return at != string::npos && dot != string::npos && at > 0 && dot > at + 1 && dot < e.size() - 1;
}
void handleLogin(MYSQL* conn, int& loginAttempts) {
    const int MAX = 3;
    printHeader("Login");
    if (loginAttempts >= MAX) {
        printError("Too many failed attempts. Account temporarily locked.");
        printInfo("Use 'Forgot Password' to regain access.");
        pressAnyKey();
        return;
    }
    string username = getInput("  Username: ");
    string password = getPassword("  Password: ");
    if (username.empty() || password.empty()) { printError("Username and password cannot be empty."); pressAnyKey(); return; }
    cout << "\n  Authenticating..." << flush;
    pauseMs(500);
    if (!usernameExists(conn, username)) { printError("No account found for '" + username + "'. Please sign up first."); pressAnyKey(); return; }
    if (loginUser(conn, username, password)) {
        loginAttempts = 0;
        printSuccess("LOGIN SUCCESSFUL!  Welcome back, " + username + "!");
    } else {
        ++loginAttempts;
        int rem = MAX - loginAttempts;
        if (rem > 0) printError("Wrong password. " + to_string(rem) + " attempt(s) remaining.");
        else         printError("Wrong password. Account is now temporarily locked.");
    }
    pressAnyKey();
}
void handleSignup(MYSQL* conn) {
    printHeader("Sign Up");
    string username = getInput("  Username (max 30): ");
    string password = getPassword("  Password (max 50): ");
    string email    = getInput("  Email:              ");
    if (username.empty() || password.empty() || email.empty()) { printError("All fields are required."); pressAnyKey(); return; }
    if (username.size() > 30) { printError("Username max 30 characters."); pressAnyKey(); return; }
    if (password.size() > 50) { printError("Password max 50 characters."); pressAnyKey(); return; }
    if (!isValidEmail(email))  { printError("Invalid email address.");      pressAnyKey(); return; }
    cout << "\n  Creating account..." << flush;
    pauseMs(500);
    signupUser(conn, username, password, email);
    pressAnyKey();
}
void handleForgotPassword(MYSQL* conn) {
    printHeader("Forgot Password");
    string email = getInput("  Registered email: ");
    if (!isValidEmail(email))        { printError("Invalid email address.");                pressAnyKey(); return; }
    if (!emailExists(conn, email))   { printError("No account found for '" + email + "'."); pressAnyKey(); return; }
    string newPassword = getPassword("  New password: ");
    if (newPassword.empty() || newPassword.size() > 50) { printError("Password must be 1-50 characters."); pressAnyKey(); return; }
    cout << "\n  Resetting password..." << flush;
    pauseMs(500);
    resetPassword(conn, email, newPassword);
    pressAnyKey();
}
void handleDeleteAccount(MYSQL* conn) {
    printHeader("Delete Account");
    string username = getInput("  Username: ");
    string password = getPassword("  Password: ");
    if (username.empty() || password.empty()) { printError("Username and password cannot be empty."); pressAnyKey(); return; }
    string confirm = getInput("\n  Are you sure you want to delete '" + username + "'? (yes/no): ");
    if (confirm != "yes") { printInfo("Deletion cancelled."); pressAnyKey(); return; }
    cout << "\n  Deleting account..." << flush;
    pauseMs(500);
    deleteAccount(conn, username, password);
    pressAnyKey();
}
int main() {
    auto env = loadEnv();
    const string host = env.count("DB_HOST")     ? env["DB_HOST"]       : "localhost";
    const string user = env.count("DB_USER")     ? env["DB_USER"]       : "root";
    const string pass = env.count("DB_PASSWORD") ? env["DB_PASSWORD"]   : "";
    const string name = env.count("DB_NAME")     ? env["DB_NAME"]       : "login";
    const int    port = env.count("DB_PORT")     ? stoi(env["DB_PORT"]) : 3306;
    clearScreen();
    cout << "\n  Connecting to " << user << "@" << host << ":" << port << "/" << name << " ...\n" << flush;
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) { cerr << "\n  [FAIL] mysql_init() failed.\n"; return 1; }
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), name.c_str(), port, nullptr, 0)) {
        cerr << "\n  [FAIL] " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return 1;
    }
    mysql_set_character_set(conn, "utf8mb4");
    printSuccess("Connected to database successfully!");
    pauseMs(800);
    int loginAttempts = 0;
    while (true) {
        clearScreen();
        cout << "\n  ===== User Authentication =====\n\n"
             << "    1.  Login\n"
             << "    2.  Sign Up\n"
             << "    3.  Forgot Password\n"
             << "    4.  Delete Account\n"
             << "    5.  Exit\n\n"
             << "  Choice (1-5): " << flush;
        string input;
        if (!getline(cin, input)) { mysql_close(conn); return 0; }
        input = trim(input);
        if (input.empty()) continue;
        int choice = 0;
        try { choice = stoi(input); } catch (...) { choice = -1; }
        switch (choice) {
            case 1: handleLogin(conn, loginAttempts); break;
            case 2: handleSignup(conn);               break;
            case 3: handleForgotPassword(conn);       break;
            case 4: handleDeleteAccount(conn);        break;
            case 5:
                clearScreen();
                cout << "\n  Closing connection...\n" << flush;
                mysql_close(conn);
                pauseMs(400);
                printSuccess("Goodbye!");
                pauseMs(600);
                return 0;
            default:
                printError("Invalid choice. Enter 1-5.");
                pauseMs(600);
        }
    }
}
