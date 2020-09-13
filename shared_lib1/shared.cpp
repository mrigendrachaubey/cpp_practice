#include <iostream>
#include "sharedlib.h"
using namespace std;

int  myAbstractBaseclass::invokfun() {
        std::cout << "myAbstractBaseclass: invokfun called" << std::endl;
        response();
        return 0;
}
