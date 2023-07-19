#include <iostream>
using namespace std;

class User
{
public://difalt user is library
    string userName;// for library user name its book location
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