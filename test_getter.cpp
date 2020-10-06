#include <iostream>
using namespace std;

class B {
   private:
      int age;
   public:
      void setAge(int _age);
      int getAge();
};

void B::setAge(int _age) {
   age = _age;
}

int B::getAge() {
	return age;
}

int main(){
	B fun;
	fun.setAge(21);
	std::cout << "" << fun.getAge() << std::endl;
	return 0;
}
