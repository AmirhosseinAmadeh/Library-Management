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
        booklist.push_front(b);
    }
};
