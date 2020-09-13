#include "derived.h"

using namespace std;

void derivedClass::setCond(int cond)
{
    pcond = cond;
}

int derivedClass::getCond()
{
    return pcond;
}

void derivedClass::responseCallback(void)
{
    std::cout << "derivedClass: response called" << std::endl;
    setCond(1);
}

void derivedClass::responseErrorCallback(void)
{
    std::cout << "derivedClass: response called" << std::endl;
    setCond(2);
}