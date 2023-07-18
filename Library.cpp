#include <iostream>
#include "book.cpp"
#include <list>
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

    void searchByTitle(string title) {
        bool found = false;
        for (const Book& book : booklist) {
            if (book.title == title) {
                book.displayDetails();
                found = true;
            }
        }
        if (!found) {
            cout << "Book not found.\n";
        }
    }

    void searchByAuthor(string author) {
        bool found = false;
        for (const Book& book: booklist){
            if(book.author == author){
                book.displayDetails();
                found = true;
            }
        }
        if(!found){
            cout << "Book not found.\n";
        }
    }
};
