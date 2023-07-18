#include <iostream>
#include "genres.cpp"
using namespace std;

class Book
{
private:
    string title;
    Genres genre;
    string author;
    string ISBN;
    bool available;
    // Constructor

public:
    Book(string t, string a, string isbn, Genres genre)
    {
        title = t;
        author = a;
        ISBN = isbn;
        available = true; // New books are available by default
    }

     // Display book details
    void displayDetails() {
        cout << "Title: " << title << endl;
        cout << "genre: " << genre << endl;
        cout << "Author: " << author << endl;
        cout << "ISBN: " << ISBN << endl;
        cout << "Availability: " << (available ? "Available" : "Not Available") << endl;
        cout << "------------------------" << endl;
    }
};