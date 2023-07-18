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
    map<string, Genres> genreMap = {
        {"fiction", Genres::Fiction},
        {"adventure", Genres::Adventure},
        {"comedy", Genres::Comedy},
        {"drama", Genres::Drama},
        {"childrens", Genres::Childrens},
        {"fantasy", Genres::Fantasy},
        {"horror", Genres::Horror},
        {"non fiction", Genres::Non_Fiction},
        {"poetry", Genres::Poetry},
        {"thriller", Genres::Thriller},
        {"romance", Genres::Romance},
        {"ya", Genres::YA},
        };

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

    void searchByGenre(string genre)
    {
        bool found = false;
        for (const Book &book : booklist)
        {
            // if(book.genre == genre){
            book.displayDetails();
            found = true;
            //}
        }
        if (!found)
        {
            cout << "Book not found.\n";
        }
    }
};
