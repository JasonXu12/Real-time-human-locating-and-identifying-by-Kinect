
// Client.cpp : Defines the entry point for the console application.
//
#include "winsock2.h"
#include "myKinect.h"
#include<iomanip>
#include <sstream>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
BOOL	RecvLine(SOCKET s, char* buf);	//��ȡһ������
string IntToString(int & i);
#define PORT 4000
#define IP_ADDRESS "192.168.0.107"


// FPS info.
// FPS��ر���
int g_fps = 0;             // FPS֡��ֵ
char g_fpsStr[16] = { 0 };   // ���֡��ֵ
float g_time = 0.0f;       // ϵͳ����ʱ��
float g_lastTime = 0.0f;   // ������ʱ��


// ����֡��
void GetFPS()
{

	// Get the second in millisecond then multiply it to convert to seconds.
	// g_time��ȡ����ϵͳ�����������������ĺ�����������0.001f�õ�����
	g_time = GetTickCount() * 0.001f;


	// If time - last time is > than 1, save fpt.
	// ����ʱ���Ƿ����1��
	if (g_time - g_lastTime > 1.0f)
	{
		// Record last time.
		// ��¼�µĳ���ʱ��
		g_lastTime = g_time;

		// Save FPS.
		// ������g_fps��ʽ��Ϊһ���ַ���������g_fpsStr�У���������ַ���
		//sprintf(g_fpsStr, "FPS: %d", g_fps);
		cout << g_fps << endl;
		// Reset the FPS counter.
		// ����֡��ֵΪ0
		g_fps = 0;
	}
	else
	{
		// Add to the frames per second.
		// ֡��ֵ����
		g_fps++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	const int BUF_SIZE = 1024;
	WSADATA			wsd;			//WSADATA����
	SOCKET			sHost;			//�������׽���
	SOCKADDR_IN		servAddr;		//��������ַ
	char			buf[BUF_SIZE];	//�������ݻ�����
	char			bufRecv[BUF_SIZE];
	int				retVal;			//����ֵ

	//��ʼ���׽��ֶ�̬��
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		cout << "WSAStartup failed!" << endl;
		return -1;
	}

	//�����׽���
	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sHost)
	{
		cout << "socket failed!" << endl;
		WSACleanup();//�ͷ��׽�����Դ
		return  -1;
	}
	//���÷�������ַ
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	servAddr.sin_port = htons(PORT);
	int	nServAddlen = sizeof(servAddr);
	//���ӷ�����
	retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
	if (SOCKET_ERROR == retVal)
	{
		cout << "connect failed!" << endl;
		closesocket(sHost);	//�ر��׽���
		WSACleanup();		//�ͷ��׽�����Դ
		return -1;
	}

	CBodyBasics myKinect;
	HRESULT hr = myKinect.InitializeDefaultSensor();

	if (SUCCEEDED(hr))
	{

		while (true){
			myKinect.Update();
			//GetFPS();
			string strs;
			int count = 0;
			for (int i = 0; i<BODY_COUNT; i++)
			{
				if (myKinect.myState[i] == 1)
				{
					//float angle = float(abs(myKinect.X[i] - 350)) / float(abs(myKinect.Z[i] - 73));
					//if (myKinect.Z[i] > 40 && myKinect.Z[i] < 300 && (myKinect.Z[i]<255 || myKinect.X[i]<444) && (myKinect.Z[i]<255 || myKinect.X[i]>244))
					{
						strs = strs + '@' + IntToString(i);
						strs = strs + 'X' + IntToString(myKinect.X[i]);
						strs = strs + 'Y' + IntToString(myKinect.Y[i]);
						strs = strs + 'Z' + IntToString(myKinect.Z[i]);
						//strs = strs + 'H' + IntToString(myKinect.bodylength[i]);
						//strs = strs + 'B' + IntToString(myKinect.avgB[i]);
						//strs = strs + 'G' + IntToString(myKinect.avgG[i]);
						//strs = strs + 'R' + IntToString(myKinect.avgR[i]);
						strs = strs + "I000";
						count++;
					}
				}
			}
			if (count > 0)
			{
				strs = '1' + IntToString(count) + strs;
				//���������������
				ZeroMemory(buf, BUF_SIZE);
				strs.copy(buf, count * 20 + 4, 0); //����count * 12 +4�������Ƽ����ַ���0�����Ƶ�λ��
				*(buf + count * 20 + 4) = '\0'; //Ҫ�ֶ����Ͻ�����
				retVal = send(sHost, buf, strlen(buf), 0);
				if (SOCKET_ERROR == retVal)
				{
					cout << "send failed!" << endl;
					closesocket(sHost);	//�ر��׽���
					WSACleanup();		//�ͷ��׽�����Դ
					return -1;
				}
				//RecvLine(sHost, bufRecv);
			}
			if (count == 0)
			{
				strs = "N1";
				//���������������
				ZeroMemory(buf, BUF_SIZE);
				strs.copy(buf, 2, 0);
				*(buf + 2) = '\0'; //Ҫ�ֶ����Ͻ�����
				retVal = send(sHost, buf, strlen(buf), 0);
				if (SOCKET_ERROR == retVal)
				{
					cout << "send failed!" << endl;
					closesocket(sHost);	//�ر��׽���
					WSACleanup();		//�ͷ��׽�����Դ
					return -1;
				}
			}
		}
		//�˳�
	}
	else
	{
		cout << "kinect initialization failed!" << endl;
		system("pause");
	}

	closesocket(sHost);	//�ر��׽���
	WSACleanup();		//�ͷ��׽�����Դ
	return 0;
}

string IntToString(int & i)
{
	string s;
	stringstream ss(s);
	ss << setw(3) << setfill('0') << i;
	return ss.str();
}


