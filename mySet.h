#include <iostream>
#include <string>
#include <memory.h>
using namespace std;
class Set
{
	int maxsize; //���ϵĵ�ǰ�������
	int count;  //���ϵĵ�ǰԪ�ظ���
	int *elem;
public:
	Set(int initsize = 10); //���캯��������һ���ռ���initsize: ���ϵĳ�ʼ����
	~Set(); //��������
	int Add(int a[], int len); //����һ����Ԫ�أ�����ֵΪ�����ӵ�Ԫ�ظ���
	int Add(int e); //����һ����Ԫ�أ�����ֵΪ�����ӵ�Ԫ�ظ���
	bool Contains(int e) const; //��鵱ǰ�������Ƿ��Ѱ���Ԫ�� e
	int GetCount() const; //��ȡ��ǰ�����е�Ԫ�ظ���
	void Print() const; //��ӡ����Ԫ��
};//ע��const ��ʾ�ú�������Ե�ǰ��������ݳ�Ա�����޸�

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
	delete[] elem;	//�ͷ�ռ�ÿռ�
}

Set::Set(int initsize)
	:maxsize(initsize), count(0)
{
	elem = new int[maxsize];
	if (!elem) throw "���뼯�Ͽռ�ʧ�ܣ�";
}


int Set::Add(int e)
{
	if (Contains(e)) return 0; //��ǰ�������Ѱ�����ָ��Ԫ�أ���������ˡ�
	if (count == maxsize)	//�ռ������������ӿռ�
	{
		int *tmp = new int[maxsize + 10];
		if (!tmp) return 0; //�����¿ռ�ʧ�ܣ��˳�
		memcpy(tmp, elem, count*sizeof(int)); //��ԭ�������ݸ��Ƶ��¿ռ���
		maxsize += 10; //�����������10�����ݵ�λ
		delete[] elem; //ɾ��ԭ�пռ�
		elem = tmp; //���¿ռ�ĵ�ַ��������
	}
	elem[count++] = e; //����Ԫ�ش������������
	return 1;
}

bool Set::Contains(int e) const
{
	for (int i = 0; i < count; i++)
		if (elem[i] == e) return true;
	return false;
}