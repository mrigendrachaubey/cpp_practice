#include <iostream>
#include "sharedlib.h"

class derivedClass:public myAbstractBaseclass
{
   public:
        void responseCallback(void);
        void responseErrorCallback(void);
        void setCond(int cond);
        int getCond();

   private:
        int pcond;
};
