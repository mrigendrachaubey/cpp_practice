#include <iostream>
#include <cstring>
#include <stdio.h>
#include <errno.h>

using namespace std;
int main(void)
{
	int err = EAGAIN;
	std::cout << "error: "<< strerror(err) << std::endl;
	return 0;
}
