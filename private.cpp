#include <iostream>
using namespace std;

class Student
{
	private:
		int rNo;
		int mProgressPercent;
		float perc;
		//private member functions
		void updateProgress(int bytesUpdated);
		void showProgress(void);
	public:
		//public member functions
		void read(void);
		void print(void);
};

void Student::read(void)
	{
		updateProgress(30);
		showProgress();
		cout<<"read"<<endl;			
	}		

void Student::print(void)
	{
		updateProgress(20);
		showProgress();		
		cout<<"print"<<endl;		
	}

void Student::showProgress(void)
{
	cout << "" << mProgressPercent << endl;
}

void Student::updateProgress(const int bytesUpdated)
{
	mProgressPercent = bytesUpdated;
	cout << "" << mProgressPercent << endl;
}
//Main code
int main()
{
	//declaring object of class student
	Student std;	
	//reading and printing details of a student
	std.read();
	std.print();
	
	return 0;
}
