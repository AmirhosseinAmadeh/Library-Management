#include <iostream>
#include "book.cpp"
#include <list>
#include "genres.cpp"
using namespace std;
class Library
{
private:
    string libraryName;
    list<Book> booklist;
    User manager = User("library", 0, 0, 0);

public:
    Library(string name)
    {
        libraryName = name;
    }

    void addBook(Book b)
    {
        booklist.push_back(b);
    }

    void removeBook(Book b)
    {
        booklist.remove(b);
    }

    void displayAllBooks()
    {
        for (const Book &book : booklist)
        {
            if (book.user.userName == "library")
                book.displayDetails();
        }
    }

    void searchByTitle(string title)
    {
        bool found = false;
        for (const Book &book : booklist)
        {
            if (book.title == title)
            {
                book.displayDetails();
                found = true;
            }
        }
        if (!found)
        {
            cout << "Book not found.\n";
        }
    }

    void searchByAuthor(string author)
    {
        bool found = false;
        for (const Book &book : booklist)
        {
            if (book.author == author)
            {
                book.displayDetails();
                found = true;
            }
        }
        if (!found)
        {
            cout << "Book not found.\n";
        }
    }

    void searchByGenre(string genre) // need Genres to work
    {
        bool found = false;
        for (const Book &book : booklist)
        {
            if (book.genre == genre)
            {
                book.displayDetails();
                found = true;
            }
        }
        if (!found)
        {
            cout << "Book not found.\n";
        }
    }

    void borrowingBooks(string isbn, User user)
    {
        for (Book &book : booklist)
        {
            if (book.ISBN == isbn)
            {
                if (book.available)
                {
                    book.available = false;
                    book.user = user;
                    book.displayDetails();
                    return;
                }
                else
                {
                    cout << "Book not available.\n";
                    return;
                }
            }
        }
        cout << "Book with ISBN " << isbn << " not found.\n";
    }

    void returnTheBooks(string isbn)
    {
        for (Book &book : booklist)
        {
            if (book.ISBN == isbn)
            {
                if (!book.available)
                {
                    book.available = true;
                    book.user = manager;
                    book.displayDetails();
                    return;
                }
                else
                {
                    cout << "Book is already available.\n";
                    return;
                }
            }
        }
        cout << "Book with ISBN " << isbn << " not found.\n";
    }
};
