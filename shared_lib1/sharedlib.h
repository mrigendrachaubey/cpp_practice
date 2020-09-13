class myAbstractBaseclass {

  public:
        myAbstractBaseclass() { }
        int  invokfun();
        /*Pure virtual function*/
        virtual void response(void) = 0;
};
