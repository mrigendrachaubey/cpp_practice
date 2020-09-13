#include <iostream>
#include "sharedlib.h"

using namespace std;

class derivedClass:public myAbstractBaseclass
{
   public:
        void response(void);
};

void derivedClass::response(void)
{
        std::cout << "derivedClass: response called" << std::endl;
}

int main(int argc, char *argv[])
{

        derivedClass m;
        m.myAbstractBaseclass::invokfun();
        return 0;
}
