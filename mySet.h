#include <iostream>
#include <string>
#include <memory.h>
using namespace std;
class Set
{
	int maxsize; //集合的当前最大容量
	int count;  //集合的当前元素个数
	int *elem;
public:
	Set(int initsize = 10); //构造函数，创建一个空集，initsize: 集合的初始容量
	~Set(); //析构函数
	int Add(int a[], int len); //增加一组新元素，返回值为新增加的元素个数
	int Add(int e); //增加一个新元素，返回值为新增加的元素个数
	bool Contains(int e) const; //检查当前集合中是否已包含元素 e
	int GetCount() const; //获取当前集合中的元素个数
	void Print() const; //打印所有元素
};//注：const 表示该函数不会对当前对象的数据成员做出修改

void Set::Print() const
{
	for (int i = 0; i < count; i++) cout << elem[i] << " ";
	cout << endl;
}

inline int Set::GetCount() const
{
	return count;
}


Set::~Set()
{
	delete[] elem;	//释放占用空间
}

Set::Set(int initsize)
	:maxsize(initsize), count(0)
{
	elem = new int[maxsize];
	if (!elem) throw "申请集合空间失败！";
}


int Set::Add(int e)
{
	if (Contains(e)) return 0; //当前集合中已包含了指定元素，不用添加了。
	if (count == maxsize)	//空间已满，再增加空间
	{
		int *tmp = new int[maxsize + 10];
		if (!tmp) return 0; //申请新空间失败，退出
		memcpy(tmp, elem, count*sizeof(int)); //将原来的内容复制到新空间中
		maxsize += 10; //最大容量增长10个数据单位
		delete[] elem; //删除原有空间
		elem = tmp; //将新空间的地址保存下来
	}
	elem[count++] = e; //将新元素存放在数组的最后
	return 1;
}

bool Set::Contains(int e) const
{
	for (int i = 0; i < count; i++)
		if (elem[i] == e) return true;
	return false;
}