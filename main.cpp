#include "headers.h"
static string trimWhitespace(string str){
    while(!str.empty()&&(str.back()=='\r'||str.back()=='\n'||str.back()==' '))
        str.pop_back();
    return str;
}
map<string,string> loadEnvFile(const string& filePath=".env"){
    map<string,string> envMap;
    ifstream envFile(filePath);
    if(!envFile.is_open()) return envMap;
    string line;
    while(getline(envFile,line)){
        line=trimWhitespace(line);
        if(line.empty()||line[0]=='#') continue;
        auto delimPos=line.find('=');
        if(delimPos==string::npos) continue;
        envMap[trimWhitespace(line.substr(0,delimPos))]=trimWhitespace(line.substr(delimPos+1));
    }
    return envMap;
}
void clearScreen(){
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
void pauseMilliseconds(int milliseconds){
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds*1000);
#endif
}
void pressEnterToContinue(){
    cout<<"\n  [Press ENTER to return to menu...]"<<flush;
    cin.clear();
    cin.ignore(1000,'\n');
}
void printHeader(const string& title){
    clearScreen();
    cout<<"\n  ===== "<<title<<" =====\n\n"<<flush;
}
void printSuccess(const string& message){cout<<"\n  [ OK ]  "<<message<<"\n"<<flush;}
void printError(const string& message){cout<<"\n  [FAIL]  "<<message<<"\n"<<flush;}
void printInfo(const string& message){cout<<"\n  [ i  ]  "<<message<<"\n"<<flush;}
static string escapeString(MYSQL* dbConnection,const string& rawValue){
    string escapedBuffer(rawValue.size()*2+1,'\0');
    escapedBuffer.resize(mysql_real_escape_string(dbConnection,&escapedBuffer[0],rawValue.c_str(),(unsigned long)rawValue.size()));
    return escapedBuffer;
}
static MYSQL_RES* runSelectQuery(MYSQL* dbConnection,const string& query){
    if(mysql_query(dbConnection,query.c_str())) return nullptr;
    return mysql_store_result(dbConnection);
}
static bool runExecuteQuery(MYSQL* dbConnection,const string& query){
    return mysql_query(dbConnection,query.c_str())==0;
}
bool loginUser(MYSQL* dbConnection,const string& username,const string& password){
    string query="SELECT id FROM users WHERE username='"+escapeString(dbConnection,username)+
                 "' AND password=SHA2('"+escapeString(dbConnection,password)+"',256)";
    MYSQL_RES* result=runSelectQuery(dbConnection,query);
    if(!result) return false;
    bool isFound=(mysql_num_rows(result)>0);
    mysql_free_result(result);
    return isFound;
}
bool usernameExists(MYSQL* dbConnection,const string& username){
    string query="SELECT id FROM users WHERE username='"+escapeString(dbConnection,username)+"'";
    MYSQL_RES* result=runSelectQuery(dbConnection,query);
    if(!result) return false;
    bool isFound=(mysql_num_rows(result)>0);
    mysql_free_result(result);
    return isFound;
}
bool emailExists(MYSQL* dbConnection,const string& email){
    string query="SELECT id FROM users WHERE email='"+escapeString(dbConnection,email)+"'";
    MYSQL_RES* result=runSelectQuery(dbConnection,query);
    if(!result) return false;
    bool isFound=(mysql_num_rows(result)>0);
    mysql_free_result(result);
    return isFound;
}
bool signupUser(MYSQL* dbConnection,const string& username,const string& password,const string& email){
    if(usernameExists(dbConnection,username)){printError("Username '"+username+"' is already taken.");return false;}
    if(emailExists(dbConnection,email)){printError("Email '"+email+"' is already registered.");return false;}
    string query="INSERT INTO users(username,password,email) VALUES('"+
                 escapeString(dbConnection,username)+"',SHA2('"+escapeString(dbConnection,password)+"',256),'"+escapeString(dbConnection,email)+"')";
    if(!runExecuteQuery(dbConnection,query)){printError("Signup failed: "+string(mysql_error(dbConnection)));return false;}
    printSuccess("Account created for '"+username+"'! You can now log in.");
    return true;
}
bool deleteAccount(MYSQL* dbConnection,const string& username,const string& password){
    if(!loginUser(dbConnection,username,password)){printError("Invalid credentials. Cannot delete account.");return false;}
    string query="DELETE FROM users WHERE username='"+escapeString(dbConnection,username)+"'";
    if(!runExecuteQuery(dbConnection,query)){printError("Delete failed: "+string(mysql_error(dbConnection)));return false;}
    printSuccess("Account '"+username+"' deleted successfully.");
    return true;
}
bool resetPassword(MYSQL* dbConnection,const string& email,const string& newPassword){
    if(!emailExists(dbConnection,email)){printError("No account found for '"+email+"'.");return false;}
    string query="UPDATE users SET password=SHA2('"+escapeString(dbConnection,newPassword)+"',256) WHERE email='"+escapeString(dbConnection,email)+"'";
    if(!runExecuteQuery(dbConnection,query)){printError("Reset failed: "+string(mysql_error(dbConnection)));return false;}
    printSuccess("Password reset for '"+email+"'.");
    return true;
}
string readPasswordMasked(const string& prompt){
    cout<<prompt<<flush;
    string password;
#ifdef _WIN32
    char keyChar;
    while((keyChar=_getch())!='\r'&&keyChar!='\n'){
        if(keyChar=='\b'&&!password.empty()){password.pop_back();cout<<"\b \b"<<flush;}
        else if((unsigned char)keyChar>=32){password+=keyChar;cout<<'*'<<flush;}
    }
    cout<<'\n';
#else
    struct termios oldTermSettings,newTermSettings;
    tcgetattr(STDIN_FILENO,&oldTermSettings);
    newTermSettings=oldTermSettings;
    newTermSettings.c_lflag&=~ECHO;
    tcsetattr(STDIN_FILENO,TCSANOW,&newTermSettings);
    getline(cin,password);
    tcsetattr(STDIN_FILENO,TCSANOW,&oldTermSettings);
    cout<<'\n';
#endif
    return trimWhitespace(password);
}
string readTextInput(const string& prompt){
    cout<<prompt<<flush;
    string userInput;
    getline(cin,userInput);
    return trimWhitespace(userInput);
}
bool isValidEmail(const string& email){
    size_t atPos=email.find('@'),dotPos=email.rfind('.');
    return atPos!=string::npos&&dotPos!=string::npos&&atPos>0&&dotPos>atPos+1&&dotPos<email.size()-1;
}
void handleLogin(MYSQL* dbConnection,int& failedAttempts){
    const int maxAttempts=3;
    printHeader("Login");
    if(failedAttempts>=maxAttempts){
        printError("Too many failed attempts. Account temporarily locked.");
        printInfo("Use 'Forgot Password' to regain access.");
        pressEnterToContinue();
        return;
    }
    string username=readTextInput("  Username: ");
    string password=readPasswordMasked("  Password: ");
    if(username.empty()||password.empty()){printError("Username and password cannot be empty.");pressEnterToContinue();return;}
    cout<<"\n  Authenticating..."<<flush;
    pauseMilliseconds(500);
    if(!usernameExists(dbConnection,username)){printError("No account found for '"+username+"'. Please sign up first.");pressEnterToContinue();return;}
    if(loginUser(dbConnection,username,password)){
        failedAttempts=0;
        printSuccess("LOGIN SUCCESSFUL! Welcome back, "+username+"!");
    }else{
        ++failedAttempts;
        int remainingAttempts=maxAttempts-failedAttempts;
        if(remainingAttempts>0) printError("Wrong password. "+to_string(remainingAttempts)+" attempt(s) remaining.");
        else printError("Wrong password. Account is now temporarily locked.");
    }
    pressEnterToContinue();
}
void handleSignup(MYSQL* dbConnection){
    printHeader("Sign Up");
    string username=readTextInput("  Username (max 30): ");
    string password=readPasswordMasked("  Password (max 50): ");
    string email=readTextInput("  Email: ");
    if(username.empty()||password.empty()||email.empty()){printError("All fields are required.");pressEnterToContinue();return;}
    if(username.size()>30){printError("Username max 30 characters.");pressEnterToContinue();return;}
    if(password.size()>50){printError("Password max 50 characters.");pressEnterToContinue();return;}
    if(!isValidEmail(email)){printError("Invalid email address.");pressEnterToContinue();return;}
    cout<<"\n  Creating account..."<<flush;
    pauseMilliseconds(500);
    signupUser(dbConnection,username,password,email);
    pressEnterToContinue();
}
void handleForgotPassword(MYSQL* dbConnection){
    printHeader("Forgot Password");
    string email=readTextInput("  Registered email: ");
    if(!isValidEmail(email)){printError("Invalid email address.");pressEnterToContinue();return;}
    if(!emailExists(dbConnection,email)){printError("No account found for '"+email+"'.");pressEnterToContinue();return;}
    string newPassword=readPasswordMasked("  New password: ");
    if(newPassword.empty()||newPassword.size()>50){printError("Password must be 1-50 characters.");pressEnterToContinue();return;}
    cout<<"\n  Resetting password..."<<flush;
    pauseMilliseconds(500);
    resetPassword(dbConnection,email,newPassword);
    pressEnterToContinue();
}
void handleDeleteAccount(MYSQL* dbConnection){
    printHeader("Delete Account");
    string username=readTextInput("  Username: ");
    string password=readPasswordMasked("  Password: ");
    if(username.empty()||password.empty()){printError("Username and password cannot be empty.");pressEnterToContinue();return;}
    string confirmation=readTextInput("\n  Are you sure you want to delete '"+username+"'? (yes/no): ");
    if(confirmation!="yes"){printInfo("Deletion cancelled.");pressEnterToContinue();return;}
    cout<<"\n  Deleting account..."<<flush;
    pauseMilliseconds(500);
    deleteAccount(dbConnection,username,password);
    pressEnterToContinue();
}
int main(){
    map<string,string> envConfig=loadEnvFile();
    const string dbHost=envConfig.count("DB_HOST")?envConfig["DB_HOST"]:"localhost";
    const string dbUser=envConfig.count("DB_USER")?envConfig["DB_USER"]:"root";
    const string dbPass=envConfig.count("DB_PASSWORD")?envConfig["DB_PASSWORD"]:"";
    const string dbName=envConfig.count("DB_NAME")?envConfig["DB_NAME"]:"login";
    const int dbPort=envConfig.count("DB_PORT")?stoi(envConfig["DB_PORT"]):3306;
    clearScreen();
    cout<<"\n  Connecting to "<<dbUser<<"@"<<dbHost<<":"<<dbPort<<"/"<<dbName<<" ...\n"<<flush;
    MYSQL* dbConnection=mysql_init(nullptr);
    if(!dbConnection){cerr<<"\n  [FAIL] mysql_init() failed.\n";return 1;}
    if(!mysql_real_connect(dbConnection,dbHost.c_str(),dbUser.c_str(),dbPass.c_str(),dbName.c_str(),dbPort,nullptr,0)){
        cerr<<"\n  [FAIL] "<<mysql_error(dbConnection)<<"\n";
        mysql_close(dbConnection);
        return 1;
    }
    mysql_set_character_set(dbConnection,"utf8mb4");
    printSuccess("Connected to database successfully!");
    pauseMilliseconds(800);
    int failedLoginAttempts=0;
    while(true){
        clearScreen();
        cout<<"\n  ===== User Authentication =====\n\n"
            <<"    1.  Login\n"
            <<"    2.  Sign Up\n"
            <<"    3.  Forgot Password\n"
            <<"    4.  Delete Account\n"
            <<"    5.  Exit\n\n"
            <<"  Choice (1-5): "<<flush;
        string menuInput;
        if(!getline(cin,menuInput)){mysql_close(dbConnection);return 0;}
        menuInput=trimWhitespace(menuInput);
        if(menuInput.empty()) continue;
        int menuChoice=0;
        try{menuChoice=stoi(menuInput);}catch(...){menuChoice=-1;}
        switch(menuChoice){
            case 1: handleLogin(dbConnection,failedLoginAttempts); break;
            case 2: handleSignup(dbConnection); break;
            case 3: handleForgotPassword(dbConnection); break;
            case 4: handleDeleteAccount(dbConnection); break;
            case 5:
                clearScreen();
                cout<<"\n  Closing connection...\n"<<flush;
                mysql_close(dbConnection);
                pauseMilliseconds(400);
                printSuccess("Goodbye!");
                pauseMilliseconds(600);
                return 0;
            default:
                printError("Invalid choice. Enter 1-5.");
                pauseMilliseconds(600);
        }
    }
}
