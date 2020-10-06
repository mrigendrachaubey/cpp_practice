#include <iostream>
#include <string>

using namespace std;
void func(const std::string& bootInfo);

int main(void) {
	func("learn c++");
	return 0;
}

void func(const std::string& bootInfo) {
	const char s = bootInfo[3];
	std::cout <<"character at index 3 is :" << s << std::endl;	
}
