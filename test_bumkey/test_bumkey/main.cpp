#include <iostream>
#include <vector>

using namespace std;
class Test
{
public:
	Test() { pointer = new int(num++); }
	~Test() { delete pointer; cout << "delete" << endl; }

	void print() { cout << "살아있니?" << num << endl; }
private:
	static int num;
	int* pointer;
};

int Test::num = 0;

int main()
{
	Test* same = new Test();
	vector<Test*> a;
	vector<Test*> b;

	a.push_back(same);
	b.push_back(same);

	for (auto i : a)
		i->print();

	for (auto i : b)
		i->print();

	a.clear();
	b.clear();

	for (auto i : a)
		i->print();

	for (auto i : b)
		i->print();
}