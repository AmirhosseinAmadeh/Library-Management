#include <iostream>
using namespace std;

class User
{
public:
    string userName;
    string password; // National Id
    int age;
    int phoneNumber;
    User(string userName, string password, int age, int phoneNumber){
        userName = userName.c_str();
        password = password.c_str();
        age = age;
        phoneNumber = phoneNumber;
    }
    User(string userName):userName(userName){
        userName = userName.c_str();
    }
};