#include <list>
#include <string>
#include <iostream>
#include <iomanip>

int main()
{
    std::list<std::string> strings {
        "the",
        "cat",
        "sat",
        "on",
        "the",
        "mat"
    };

    auto current = strings.begin();
    auto last = strings.end();

    while (current != last)
    {
        const std::string& ref = *current;   // take a reference
        std::string copy = *current;   // take a copy  
        copy += " - modified";   // modify the copy

        // prove that modifying the copy does not change the string
        // in the list
        std::cout << std::quoted(ref) << " - " << std::quoted(copy) << std::endl;

        // move the iterator to the next in the list
        current = std::next(current, 1);
        // or simply ++current;
    }

    return 0;
}
