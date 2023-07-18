#include <iostream>
#include "book.cpp"
#include <list>
class library
{
    string libraryName;
    list<Book> booklist;

    library(string name) : libraryName(name){}
};
