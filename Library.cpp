#include <iostream>
#include "book.cpp"
#include <list>
#include <map>
#include "genres.cpp"
class library
{
private:
    string libraryName;
    list<Book> booklist;

    library(string name) : libraryName(name) {}

public:
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

    void searchByGenre(Genres genre) // need Genres to work
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

    void borrowingBooks(string isbn)
    {
        for (Book &book : booklist)
        {
            if (book.ISBN == isbn)
            {
                if (book.available)
                {
                    book.available = false;
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
        // cout << "Book not faund.\n
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
        cout << "Book not faund.\n";
    }
};
