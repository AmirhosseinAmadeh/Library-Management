#include <iostream>

using namespace std;

class Book
{
private:
    string title;
    string author;
    string ISBN;
    bool available;
    // Constructor
    Book(string t, string a, string isbn)
    {
        title = t;
        author = a;
        ISBN = isbn;
        available = true; // New books are available by default
    }
};