
// Client.cpp : Defines the entry point for the console application.
//
#include "winsock2.h"
#include "myKinect.h"
#include<iomanip>
#include <sstream>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
BOOL	RecvLine(SOCKET s, char* buf);	//读取一行数据
string IntToString(int & i);
#define PORT 4000
#define IP_ADDRESS "192.168.0.107"


// FPS info.
// FPS相关变量
int g_fps = 0;             // FPS帧率值
char g_fpsStr[16] = { 0 };   // 存放帧率值
float g_time = 0.0f;       // 系统运行时间
float g_lastTime = 0.0f;   // 持续的时间


// 计算帧率
void GetFPS()
{

	// Get the second in millisecond then multiply it to convert to seconds.
	// g_time获取操作系统启动到现在所经过的毫秒数，乘以0.001f得到秒数
	g_time = GetTickCount() * 0.001f;


	// If time - last time is > than 1, save fpt.
	// 持续时间是否大于1秒
	if (g_time - g_lastTime > 1.0f)
	{
		// Record last time.
		// 记录新的持续时间
		g_lastTime = g_time;

		// Save FPS.
		// 把整数g_fps格式化为一个字符串保存在g_fpsStr中，并输出该字符串
		//sprintf(g_fpsStr, "FPS: %d", g_fps);
		cout << g_fps << endl;
		// Reset the FPS counter.
		// 重置帧率值为0
		g_fps = 0;
	}
	else
	{
		// Add to the frames per second.
		// 帧率值递增
		g_fps++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	const int BUF_SIZE = 1024;
	WSADATA			wsd;			//WSADATA变量
	SOCKET			sHost;			//服务器套接字
	SOCKADDR_IN		servAddr;		//服务器地址
	char			buf[BUF_SIZE];	//接收数据缓冲区
	char			bufRecv[BUF_SIZE];
	int				retVal;			//返回值

	//初始化套结字动态库
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		cout << "WSAStartup failed!" << endl;
		return -1;
	}

	//创建套接字
	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sHost)
	{
		cout << "socket failed!" << endl;
		WSACleanup();//释放套接字资源
		return  -1;
	}
	//设置服务器地址
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	servAddr.sin_port = htons(PORT);
	int	nServAddlen = sizeof(servAddr);
	//连接服务器
	retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
	if (SOCKET_ERROR == retVal)
	{
		cout << "connect failed!" << endl;
		closesocket(sHost);	//关闭套接字
		WSACleanup();		//释放套接字资源
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
				//向服务器发送数据
				ZeroMemory(buf, BUF_SIZE);
				strs.copy(buf, count * 20 + 4, 0); //这里count * 12 +4，代表复制几个字符，0代表复制的位置
				*(buf + count * 20 + 4) = '\0'; //要手动加上结束符
				retVal = send(sHost, buf, strlen(buf), 0);
				if (SOCKET_ERROR == retVal)
				{
					cout << "send failed!" << endl;
					closesocket(sHost);	//关闭套接字
					WSACleanup();		//释放套接字资源
					return -1;
				}
				//RecvLine(sHost, bufRecv);
			}
			if (count == 0)
			{
				strs = "N1";
				//向服务器发送数据
				ZeroMemory(buf, BUF_SIZE);
				strs.copy(buf, 2, 0);
				*(buf + 2) = '\0'; //要手动加上结束符
				retVal = send(sHost, buf, strlen(buf), 0);
				if (SOCKET_ERROR == retVal)
				{
					cout << "send failed!" << endl;
					closesocket(sHost);	//关闭套接字
					WSACleanup();		//释放套接字资源
					return -1;
				}
			}
		}
		//退出
	}
	else
	{
		cout << "kinect initialization failed!" << endl;
		system("pause");
	}

	closesocket(sHost);	//关闭套接字
	WSACleanup();		//释放套接字资源
	return 0;
}

string IntToString(int & i)
{
	string s;
	stringstream ss(s);
	ss << setw(3) << setfill('0') << i;
	return ss.str();
}


