#include "mySet.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include<iomanip>
#include <sstream>
#include <iostream>
#include<opencv2\opencv.hpp>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
const int BUF_SIZE = 1024;

#define PORT 4000
#define IP_ADDRESS "192.168.0.107"

cv::Mat i_map;
int all_info[18][2] = { 0 };  //X，Z
int ID_info[18][4] = { 0 };  //X，Z，ID
int histroy_info[18][2][100] = { 0 }; //X,Z 历史信息
int map_ID[10000] = { 0 }; //记录地图ID与真实ID的映射关系

float v_x[18] = { 0 };
float v_z[18] = { 0 };
float p_x[18] = { 0 };
float p_z[18] = { 0 };

cv::Scalar color[10] = { cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), cv::Scalar(255, 255, 0), cv::Scalar(255, 0, 255), cv::Scalar(0, 255, 255), cv::Scalar(255, 165, 0), cv::Scalar(128, 64, 0), cv::Scalar(255, 255, 255), cv::Scalar(100, 100, 100) }; //by 许稼轩
cv::Scalar histroy_color[10] = { cv::Scalar(255/2, 0, 0), cv::Scalar(0, 255/2, 0), cv::Scalar(0, 0, 255/2), cv::Scalar(255/2, 255/2, 0), cv::Scalar(255/2, 0, 255/2), cv::Scalar(0, 255/2, 255/2), cv::Scalar(255/2, 165/2, 0), cv::Scalar(128/2, 64/2, 0), cv::Scalar(255/2, 255/2, 255/2), cv::Scalar(100/2, 100/2, 100/2) }; //by 许稼轩
string IntToString_showposition(int & i,int j)
{
	string s;
	stringstream ss(s);
	ss << setw(3) << setfill('0') << (i-j);
	return ss.str();
}

string IntToString_ID(int & i)
{
	string s;
	stringstream ss(s);
	ss << (i);
	return ss.str();
}
//创建线程
DWORD WINAPI ClientThread(LPVOID ipParameter)
{
	SOCKET ClientScoket = (SOCKET)ipParameter;
	int RET = 0;
	char buf[1024];
	//初始化 recvBuffer
	while (true)
	{
		ZeroMemory(buf, BUF_SIZE); 
		RET = recv(ClientScoket, buf, 1024, 0);
		if (RET == 0 || RET == SOCKET_ERROR)
		{
			cout << "failed,exit" << endl;
			break;
		}
		string str = buf;
		//cout << str << endl;
		if (str.at(0) != 'N')
		{
			string client_id(str, 0, 1);
			string s_count(str, 3, 1);
			int c_id = atoi(client_id.c_str());
			int count = atoi(s_count.c_str());
			//cout << count << endl;
			for (int i = 0; i < 6; i++) //置零数组
			{
				all_info[c_id * 6 - 6 + i][0] = 0;
				all_info[c_id * 6 - 6 + i][1] = 0;
			}
			for (int i = 0; i < count; i++)
			{
				string Index(str, i * 20 + 5, 3);
				string X(str, i * 20 + 9, 3);
				string Y(str, i * 20 + 13, 3);
				string Z(str, i * 20 + 17, 3);
				string ID(str, i * 20 + 21, 3);
				int index = c_id * 6 - 6 +atoi(Index.c_str());
				int x = atoi(X.c_str());
				int y = atoi(Y.c_str());
				int z = atoi(Z.c_str());
				int id = atoi(ID.c_str());
				if (id > 0 && ID_info[index][2]>0)
				{
					int temp_id = ID_info[index][2];
					map_ID[temp_id] = id;
				}
				///////////////////////////////////////////////////////////////////////////////
				all_info[index][0] = x;
				all_info[index][1] = z;
				/*
				int j;
				for ( j = 0; j < index; j++) //检查是否有一样的点
				{
					if (all_info[j][2] != -1)
					{
						if (abs(all_info[index][0] - all_info[j][0]) + abs(all_info[index][1] - all_info[j][1]) < 14.14)

						{
							all_info[index][2] = j;
							break;
						}
					}
				}
				if (j == index) //没有一样的点
				{
					all_info[index][2] = 666;
						i_map.setTo(0);
					//if (all_info[index][2] == 666)
						//cout << x << " " << y << " " << z << endl;
						circle(i_map, cvPoint(x, z), 15, color[atoi(Index.c_str())], -1);
			    }*/
				//////////////////////////////////////////////////////////////////////////
			}
		}
		else
		{
			string client_id(str, 1, 1);
			int c_id = atoi(client_id.c_str());
			for (int i = 0; i < 6; i++) //置零数组
			{
				all_info[c_id * 6 - 6 + i][0] = 0;
				all_info[c_id * 6 - 6 + i][1] = 0;
			}

		}

		//cv::imshow("map", i_map);
		//cv::waitKey(1);
	}

	return 0;
}
int main(void)
{

	i_map.create(550, 700, CV_8UC3);
	i_map.setTo(0);

	for (int i = 0; i<18; i++)
		for (int j = 0; j<4; j++)
			ID_info[i][j] = 0;

	int current_ID = 1;
	int client_num = 0;

	WSADATA     WSA;
	SOCKET      severScoket, clientScoket;
	struct      sockaddr_in  LocalAddr, clientAddr;
	int         AddrLen = 0;
	HANDLE      hThread = NULL;
	int         RET = 0;
	//init windows socket
	if (WSAStartup(MAKEWORD(2, 2), &WSA) != 0)
	{
		cout << "init failed" << endl;
		return -1;
	}
	//creat socket
	severScoket = socket(AF_INET, SOCK_STREAM, 0);
	if (severScoket == INVALID_SOCKET)
	{
		cout << "creat failed" << GetLastError() << endl;
		return -1;
	}
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	LocalAddr.sin_port = htons(PORT);
	memset(LocalAddr.sin_zero, 0x00, 8);
	//bind socket
	RET = bind(severScoket, (struct sockaddr*)&LocalAddr, sizeof(LocalAddr));
	if (RET != 0)
	{
		cout << "bind failed";
		return -1;
	}
	RET = listen(severScoket, 5);
	if (RET != 0)
	{
		cout << "listen failed";
		return -1;
	}
	cout << "服务器已启动" << endl;


	while (client_num < 3)
	{
		cout << "once again" << endl;
		AddrLen = sizeof(clientAddr);
		clientScoket = accept(severScoket, (struct sockaddr*)&clientAddr, &AddrLen);
		if (clientScoket == INVALID_SOCKET)
		{
			cout << "accept failed";
			break;
		}
		//cout << "客户端连接" << inet_ntoa(clientAddr.sin_addr) << "." << clientAddr.sin_port << endl;
		hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)clientScoket, 0, NULL);
		if (hThread == NULL)
		{
			cout << "creat thread failed" << endl;
			break;
		}
		CloseHandle(hThread);
		client_num++;
	}

	int histroy_count = 0; //存储帧数的标记
	/////////////////////////////////////////////////////////////////////////////////////
	while (true)
	{
		i_map.setTo(0);

		//绘制网格
		cv::rectangle(i_map, cvPoint(150,79), cvPoint(550,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint( 250,79), cvPoint(250,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(350,79), cvPoint(350,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(450,79), cvPoint(450,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(150,179), cvPoint(550,179), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(150,279), cvPoint(550,279), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(150,379), cvPoint(550,379), cv::Scalar(255, 255, 255), 1);
		cv::putText(i_map, "(0,0)", cvPoint(150, 79), CV_FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 255, 255));
		cv::putText(i_map, "(400,400)", cvPoint(550, 479), CV_FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 255, 255));

		for (int i = 0; i<18; i++)    //copy X，Z信息
			for (int j = 0; j<2; j++)
				ID_info[i][j] = all_info[i][j];


		for (int i = 0; i < 18; i++)       //遍历所有节点,更新ID信息
		{
			int match_flag = 0;  //表示匹配情况;

			if (ID_info[i][0] == 0 && ID_info[i][1] == 0)  //如果当前帧位置缺失
			{
				if (ID_info[i][2] != 0)   //有ID
				{
					int my_frame = 0; //前一帧下标
					if (histroy_count - 1< 0)
						my_frame = histroy_count - 1 + 100;
					else
						my_frame = histroy_count - 100;
					if (histroy_info[i][0][my_frame] > 144 && histroy_info[i][0][my_frame] < 544 && histroy_info[i][1][my_frame] >55 && histroy_info[i][1][my_frame] < 455) //在范围之内，判断为阻挡
					{
						int trackbyother = 0;//判断是换工作区，有没有其他tracking到的camera
						for (int j = 0; j < 18; j++)
						{
							if ((ID_info[j][2] == ID_info[i][2]) && (i != j))
								ID_info[i][2]=0;
						}
						if (ID_info[i][3] < 100 && ID_info[i][2]!=0)  //被阻挡的时间不长
						{
							if (ID_info[i][3] == 0)
							{
								int my_frame76 = 0;  //前26帧的下标
								if (histroy_count - 76 < 0)
									my_frame76 = histroy_count - 76 + 100;
								else
									my_frame76 = histroy_count - 76;

								v_x[i] = float(histroy_info[i][0][my_frame] - histroy_info[i][0][my_frame76])/float(75); //x轴速度
								v_z[i] = float(histroy_info[i][1][my_frame] - histroy_info[i][1][my_frame76])/float(75); //y轴速度
								p_x[i] = histroy_info[i][0][my_frame];
								p_z[i] = histroy_info[i][1][my_frame];
							}
							ID_info[i][0] = p_x[i] + v_x[i] * float(ID_info[i][3] + 1);  //更新ID_info的x、z坐标
							ID_info[i][1] = p_z[i] + v_z[i] * float (ID_info[i][3] + 1); 
							ID_info[i][3]++;  //丢失帧数加一
						}
						else   //被阻挡时间过长，认为丢失
						{
							ID_info[i][2] = 0;  //ID清空
							ID_info[i][3] = 0; //阻挡帧数清空
						}
					}
					else  //上一帧的数据走出了工作范围，清空ID和blocking
					{
						ID_info[i][2] = 0;  //ID清空
						ID_info[i][3] = 0; //阻挡帧数清空
					}
				}
			}
			else   //有坐标数据，说明正在被tracking
			{				
				if (ID_info[i][2] == 0)  //未标识的追踪者，作为主动的接受者
				{
					int indurance = 0;
					for (int k = 1; k <= 10; k++)  //遍历历史信息，筛选出已经连续出现10帧的点
					{
						int my_frame = 0;
						if (histroy_count - k < 0)
							my_frame = histroy_count - k + 100;
						else
							my_frame = histroy_count - k;
						if (histroy_info[i][0][my_frame] != 0 || histroy_info[i][1][my_frame] != 0)
							indurance++;
					}
					if (indurance == 10)
					{
						int total_distance = 0;
						int mean_distance = 0;
						int varance = 0;
						int distance_record[11] = { 0 };
						int min_distance=800;
						int match_index = 0;

						for (int j = 0; j < 18; j++)       //遍历所有其他摄像头中的节点j
						{
							int a = i / 6;
							int b = j / 6;
							//if (a != b)  //不属于同一个摄像头
							{
								if ((ID_info[j][0] != 0 || ID_info[j][1] != 0)  && ID_info[j][2] != 0)       //j有追踪信息
								{
									 total_distance = 0;
									 mean_distance = 0;
									 varance = 0;
									
									for (int k = 1; k <= 10; k++)
									{
										int my_frame = 0;
										if (histroy_count - k < 0)
											my_frame = histroy_count - k + 100;
										else
											my_frame = histroy_count - k;
										distance_record[k] = abs(histroy_info[i][0][my_frame] - histroy_info[j][0][my_frame]) + abs(histroy_info[i][1][my_frame] - histroy_info[j][1][my_frame]);
										total_distance += distance_record[k];
									}

									mean_distance = total_distance / 10;
									for (int k = 1; k <= 10; k++)
										varance += (distance_record[k] - mean_distance)*(distance_record[k] - mean_distance);

									//if (total_distance< 400 && varance < 400)  //i,j 是一个点,并且j有ID标识
									{
										//ID_info[i][2] = ID_info[j][2];
										
										if (min_distance > (total_distance + varance))
										{
											match_flag = 1;
											min_distance = total_distance + varance;
											match_index = j;
										}

									}
								}
							}
						}

						if (match_flag == 1)
						{
							ID_info[i][2] = ID_info[match_index][2];
							
							if (ID_info[match_index][3] > 0)//正在被遮挡阶段,解除遮挡成功，消除预测
							{
								ID_info[match_index][3] = 0;
								ID_info[match_index][2] = 0;
							}
						}

						if (match_flag == 0) //未标示，并且没有匹配成功
						{
							if (current_ID < 10000)
							{
								ID_info[i][2] = current_ID;
								current_ID++;
								if (current_ID > 2)
									cout << "opps" << endl;
							}
							else
								cout << "人数已达上限" << endl;
						}
					}
				}
				else  //已标示的追踪者，有ID信息
				{
					if (ID_info[i][3] > 0)  //之前正在处于遮挡状态的点，对比新出现的点和预测的距离
					{
						int my_frame = 0; //前一帧下标
						if (histroy_count - 1< 0)
							my_frame = histroy_count - 1 + 100;
						else
							my_frame = histroy_count - 1;

						if ((abs(ID_info[i][0] - histroy_info[i][0][my_frame]) + abs(ID_info[i][1] - histroy_info[i][1][my_frame])) < 100)  //范围以内，阻挡解除，保留ID
							ID_info[i][3] = 0;
						else
						{
							ID_info[i][2] = 0;
							ID_info[i][3] = 0;
						}
					}
				}
			}
		}
		for (int i = 0; i<18; i++)    //histroy_info同步copy预测后的 X，Z信息
			for (int j = 0; j<2; j++)
				histroy_info[i][j][histroy_count] = ID_info[i][j];

		histroy_count++;
		if (histroy_count == 100)
			histroy_count = 0;  //刷新存储帧数

		Set user_info;
		for (int i = 0; i < 18; i++)       //遍历所有节点,绘制map
		{
			if (ID_info[i][2] != 0)
				if (user_info.Add(ID_info[i][2]))
				{
					circle(i_map, cvPoint(ID_info[i][0]+6, ID_info[i][1]+24), 8, color[(ID_info[i][2] % 6)], -1);
					if (map_ID[ID_info[i][2]]==0)
					cv::putText(i_map, "Unknown(" + IntToString_showposition(ID_info[i][0], 144) + "," + IntToString_showposition(ID_info[i][1], 55) + ")", cvPoint(ID_info[i][0], ID_info[i][1]), CV_FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(255, 255, 255));
					if (map_ID[ID_info[i][2]]>0)
						cv::putText(i_map, "ID:" + IntToString_ID(map_ID[ID_info[i][2]])+"(" + IntToString_showposition(ID_info[i][0], 144) + " ," + IntToString_showposition(ID_info[i][1], 55) + " )", cvPoint(ID_info[i][0], ID_info[i][1]), CV_FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(255, 255, 255));
					for (int j = 0; j < 100; j++)
					{
						if (histroy_info[i][0][j] != 0 || histroy_info[i][1][j]!=0)
						circle(i_map, cvPoint(histroy_info[i][0][j] + 6, histroy_info[i][1][j] + 24), 5, histroy_color[(ID_info[i][2] % 6)], -1);
					}
			
				}

		}
		cv::imshow("map", i_map);
		cv::waitKey(1);
	}

	closesocket(severScoket);
	closesocket(clientScoket);
	WSACleanup();
	return 0;
}
