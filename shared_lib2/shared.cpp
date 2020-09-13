#include <iostream>
#include "sharedlib.h"
using namespace std;

int  myAbstractBaseclass::invokfun1() {
        bool retVal = true;
        std::cout << "myAbstractBaseclass: invokfun1 called" << std::endl;
        if (retVal == true)
        {
            responseCallback();
        }
        else
        {
            responseErrorCallback();
        }
        return 0;
}

int  myAbstractBaseclass::invokfun2() {
        bool retVal = true;
        std::cout << "myAbstractBaseclass: invokfun2 called" << std::endl;
        return retVal;
}
