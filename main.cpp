#include <iostream>
#include <stdio.h>
#include "book.cpp"
#include "Library.cpp"
#include "sqlite_modern_cpp-dev\hdr\sqlite_modern_cpp.h"
using namespace std;
int main()
{
    Library library("center");

    // Sample books
    Book book1("The C++ Programming Language", "Bjarne Stroustrup", "Non_Fiction", "978-0201889543");
    Book book2("Effective Modern C++", "Scott Meyers", "Fantasy", "978-1491903995");

    // Add books to the library
    library.addBook(book1);
    library.addBook(book2);

    // Display all books in the library
    cout << "All Books in the Library:\n";
    library.displayAllBooks();

    // Search for books by title
    cout << "Search by Title: The C++ Programming Language\n";
    library.searchByTitle("The C++ Programming Language");

    // Search for books by author
    cout << "Search by Author: Scott Meyers\n";
    library.searchByAuthor("Scott Meyers");

    // Update book availability
    cout << "Update book availability (ISBN: 978-0201889543)\n";
    library.borrowingBooks("978-0201889543", User("ali"));

    // Display all books after updating availability
    cout << "All Books in the Library:\n";
    library.displayAllBooks();

    return 0;
}
