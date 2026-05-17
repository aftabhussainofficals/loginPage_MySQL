#include<iostream>
#include <string>
#include <mysql.h>
#include <mysqld_error.h>

using namespace std;
const char* DB_HOST ="localhost";
const char* DB_USER ="root";
const char* DB_PASSWORD= "Aft@bhussain00";    //my local db pasword 
const char* DB_NAME= "login";
const int DB_PORT= 3306;

bool loginUser(MYSQL* connection, string username,string password){
    string query = "SELECT id FROM users WHERE username = '"+username + "' AND password = '" + password +"'";
    mysql_query(connection, query.c_str());
    MYSQL_RES* result = mysql_store_result(connection);
    bool foundUser= (mysql_num_rows(result)>0);
    mysql_free_result(result);
    return foundUser;
}

bool signupUser(MYSQL* connection, string username, string password,string email){
    string check_query="SELECT id FROM users WHERE username= '"+ username+"'";
    mysql_query(connection,check_query.c_str());
    MYSQL_RES* result = mysql_store_result(connection);

    if(mysql_num_rows(result)>0){
        cout<<"\nUsername already exists!"<<endl;
        mysql_free_result(result);
        return false;
    }
    string insert_query = "INSERT INTO users(username, password, email) VALUES ('"+username+"','"+password+"','"+email+"')";
    if(mysql_query(connection, insert_query.c_str())==0){
        cout<<"User registered successfully!"<<endl;
        return true;
    }
    cout<<"\nError registering user: "<<mysql_error(connection)<<endl;
    return false;
}


void forgetPassword(MYSQL* connection){
    string email;
    cout<<"\nEnter your email: ";
    cin>>email;

    string pass_query="SELECT username, password, email FROM users WHERE email = '"+email+"'";
    mysql_query(connection,pass_query.c_str());
    MYSQL_RES* result = mysql_store_result(connection);
    
    if (mysql_num_rows(result)==0){
        cout<<"\nNo account found with that email!"<<endl;
        mysql_free_result(result);
        return;
    }

   MYSQL_ROW row = mysql_fetch_row(result);
   cout<<"Username : "<<row[0]<<endl;
   cout<<"Password : "<<row[1]<<endl;
   cout<<"Email : "<<row[2]<<endl;
   mysql_free_result(result);

   char YN;
    cout<<"\nDo you want to reset your password? (y/n): ";
    cin>>YN;
    if(YN=='y' || YN=='Y'){
        string new_password;
        cout<<"Enter new password: ";
        cin>>new_password;
        string update_query="UPDATE users SET password ='"+new_password+"' WHERE email = '"+email+"'";
        if(mysql_query(connection,update_query.c_str())==0){
            cout<<"Password updated successfully!"<<endl;
        }else{
            cout<<"\nError updating password: "<<mysql_error(connection)<<endl;
        }
    }
}





int main(){
   MYSQL* connection;
   connection = mysql_init(nullptr);

   if(!mysql_real_connect(connection,DB_HOST,DB_USER,DB_PASSWORD,DB_NAME,DB_PORT,nullptr,0)){
    system("cls");
    cout<<"Connection error: "<< mysql_error(connection) <<endl;
    mysql_close(connection); //optional
    return 1;
   }else{
    system("cls");
    cout<<"Connecting to database......\n";
    #ifdef _WIN32
    Sleep(2000);
    #else
        sleep(2);
    #endif
    system("cls");
    cout<<"Be Patient for few seconds..........\n";
    #ifdef _WIN32
    Sleep(2000);
    #else
        sleep(2);
    #endif
    system("cls");
    cout<<"Connected to database successfully!"<<endl;
   }
#ifdef _WIN32
    Sleep(2000);
#else
    sleep(2);
#endif

    while(true){
        int choice;
        system("cls");
        cout<<"\n1. Login"<<endl;
        cout<<"2. Sign Up"<<endl;
        cout<<"3. Forget Password"<<endl;
        cout<<"4. Exit"<<endl;
        cout<<"\nEnter your choice: ";
        cin>>choice;
        if(cin.fail()){cin.clear();cin.ignore();}
        switch(choice){
        case 1:
            {
                system("cls");
                cout<<"========================\n";
                string username, password;
                cout<<"Enter username: ";
                cin>>username;
                cout<<"Enter password: ";
                cin>>password;

                if(loginUser(connection, username, password)){
                    cout<<"\nLogin successful!"<<endl;
                }else{
                    cout<<"\nInvalid Credentials!"<<endl;
                }
                break;
            }
        case 2:
            {
                system("cls");
                string username, password, email;
                cout<<"Enter username: ";
                cin>>username;
                cout<<"Enter password: ";
                cin>>password;
                cout<<"Enter email: ";
                cin>>email;

                signupUser(connection, username, password, email);
                break;
            }
        case 3:
            system("cls");
            forgetPassword(connection);
            break;
        case 4:
            system("cls");
            cout<<"GOODBYE! Database has been closed......"<<endl;
            mysql_close(connection);
            #ifdef _WIN32
               Sleep(2000);
            #else
              sleep(2);
            #endif
            system("cls");
            exit(0);
        default:
            cout<<"\nInvalid choice!"<<endl;
        }
    }


    return 0;
}