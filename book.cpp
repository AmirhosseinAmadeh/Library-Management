#include <iostream>
#include "genres.cpp"
using namespace std;

class Book
{
private:
    string title;
    string author;
    string ISBN;
    bool available;
    Genres genre;
    // Constructor
    Book(string t, string a, string isbn, Genres genre)
    {
        title = t;
        author = a;
        ISBN = isbn;
        available = true; // New books are available by default
    }

};