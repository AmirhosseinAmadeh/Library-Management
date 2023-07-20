#include <iostream>
#include "genres.cpp"
#include "user.cpp"
using namespace std;

class Book
{
public:
    string title;
    string genre;
    string author;
    string ISBN;
    bool available;
    User user = User("library", 0, 0, 0);
    // Constructor

    Book(string t, string a, string isbn, string genre)
    {
        title = t;
        author = a;
        genre = genre;
        ISBN = isbn;
        available = true; // New books are available by default
    }

    // Display book details
    void displayDetails() const
    {
        cout << "Title: " << title << endl;
        cout << "genre: " << genre << endl;
        cout << "Author: " << author << endl;
        cout << "ISBN: " << ISBN << endl;
        cout << "Availability: " << (available ? "Available" : "Not Available") << endl;
        cout << "can find it by the " << user.userName << endl;
        cout << "------------------------" << endl;
    }
};