class myAbstractBaseclass {

  public:
        myAbstractBaseclass() { }
        int  invokfun1();
        int  invokfun2();
        /*Pure virtual function*/
        virtual void responseCallback(void) = 0;
        virtual void responseErrorCallback(void) = 0;
};
