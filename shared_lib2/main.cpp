#include "derived.h"

using namespace std;

int main(int argc, char *argv[])
{
        int retVal;
        derivedClass m;
        m.myAbstractBaseclass::invokfun1();
        if(m.getCond() == 1)
        {
             m.myAbstractBaseclass::invokfun2();
             retVal = 0;
         }
        else
        {
             retVal = -1;
        }
        return retVal;
}
