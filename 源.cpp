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
int all_info[18][2] = { 0 };  //X��Z
int ID_info[18][4] = { 0 };  //X��Z��ID
int histroy_info[18][2][100] = { 0 }; //X,Z ��ʷ��Ϣ
int map_ID[10000] = { 0 }; //��¼��ͼID����ʵID��ӳ���ϵ

float v_x[18] = { 0 };
float v_z[18] = { 0 };
float p_x[18] = { 0 };
float p_z[18] = { 0 };

cv::Scalar color[10] = { cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), cv::Scalar(255, 255, 0), cv::Scalar(255, 0, 255), cv::Scalar(0, 255, 255), cv::Scalar(255, 165, 0), cv::Scalar(128, 64, 0), cv::Scalar(255, 255, 255), cv::Scalar(100, 100, 100) }; //by �����
cv::Scalar histroy_color[10] = { cv::Scalar(255/2, 0, 0), cv::Scalar(0, 255/2, 0), cv::Scalar(0, 0, 255/2), cv::Scalar(255/2, 255/2, 0), cv::Scalar(255/2, 0, 255/2), cv::Scalar(0, 255/2, 255/2), cv::Scalar(255/2, 165/2, 0), cv::Scalar(128/2, 64/2, 0), cv::Scalar(255/2, 255/2, 255/2), cv::Scalar(100/2, 100/2, 100/2) }; //by �����
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
//�����߳�
DWORD WINAPI ClientThread(LPVOID ipParameter)
{
	SOCKET ClientScoket = (SOCKET)ipParameter;
	int RET = 0;
	char buf[1024];
	//��ʼ�� recvBuffer
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
			for (int i = 0; i < 6; i++) //��������
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
				for ( j = 0; j < index; j++) //����Ƿ���һ���ĵ�
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
				if (j == index) //û��һ���ĵ�
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
			for (int i = 0; i < 6; i++) //��������
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
	cout << "������������" << endl;


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
		//cout << "�ͻ�������" << inet_ntoa(clientAddr.sin_addr) << "." << clientAddr.sin_port << endl;
		hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)clientScoket, 0, NULL);
		if (hThread == NULL)
		{
			cout << "creat thread failed" << endl;
			break;
		}
		CloseHandle(hThread);
		client_num++;
	}

	int histroy_count = 0; //�洢֡���ı��
	/////////////////////////////////////////////////////////////////////////////////////
	while (true)
	{
		i_map.setTo(0);

		//��������
		cv::rectangle(i_map, cvPoint(150,79), cvPoint(550,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint( 250,79), cvPoint(250,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(350,79), cvPoint(350,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(450,79), cvPoint(450,479), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(150,179), cvPoint(550,179), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(150,279), cvPoint(550,279), cv::Scalar(255, 255, 255), 1);
		cv::line(i_map, cvPoint(150,379), cvPoint(550,379), cv::Scalar(255, 255, 255), 1);
		cv::putText(i_map, "(0,0)", cvPoint(150, 79), CV_FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 255, 255));
		cv::putText(i_map, "(400,400)", cvPoint(550, 479), CV_FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 255, 255));

		for (int i = 0; i<18; i++)    //copy X��Z��Ϣ
			for (int j = 0; j<2; j++)
				ID_info[i][j] = all_info[i][j];


		for (int i = 0; i < 18; i++)       //�������нڵ�,����ID��Ϣ
		{
			int match_flag = 0;  //��ʾƥ�����;

			if (ID_info[i][0] == 0 && ID_info[i][1] == 0)  //�����ǰ֡λ��ȱʧ
			{
				if (ID_info[i][2] != 0)   //��ID
				{
					int my_frame = 0; //ǰһ֡�±�
					if (histroy_count - 1< 0)
						my_frame = histroy_count - 1 + 100;
					else
						my_frame = histroy_count - 100;
					if (histroy_info[i][0][my_frame] > 144 && histroy_info[i][0][my_frame] < 544 && histroy_info[i][1][my_frame] >55 && histroy_info[i][1][my_frame] < 455) //�ڷ�Χ֮�ڣ��ж�Ϊ�赲
					{
						int trackbyother = 0;//�ж��ǻ�����������û������tracking����camera
						for (int j = 0; j < 18; j++)
						{
							if ((ID_info[j][2] == ID_info[i][2]) && (i != j))
								ID_info[i][2]=0;
						}
						if (ID_info[i][3] < 100 && ID_info[i][2]!=0)  //���赲��ʱ�䲻��
						{
							if (ID_info[i][3] == 0)
							{
								int my_frame76 = 0;  //ǰ26֡���±�
								if (histroy_count - 76 < 0)
									my_frame76 = histroy_count - 76 + 100;
								else
									my_frame76 = histroy_count - 76;

								v_x[i] = float(histroy_info[i][0][my_frame] - histroy_info[i][0][my_frame76])/float(75); //x���ٶ�
								v_z[i] = float(histroy_info[i][1][my_frame] - histroy_info[i][1][my_frame76])/float(75); //y���ٶ�
								p_x[i] = histroy_info[i][0][my_frame];
								p_z[i] = histroy_info[i][1][my_frame];
							}
							ID_info[i][0] = p_x[i] + v_x[i] * float(ID_info[i][3] + 1);  //����ID_info��x��z����
							ID_info[i][1] = p_z[i] + v_z[i] * float (ID_info[i][3] + 1); 
							ID_info[i][3]++;  //��ʧ֡����һ
						}
						else   //���赲ʱ���������Ϊ��ʧ
						{
							ID_info[i][2] = 0;  //ID���
							ID_info[i][3] = 0; //�赲֡�����
						}
					}
					else  //��һ֡�������߳��˹�����Χ�����ID��blocking
					{
						ID_info[i][2] = 0;  //ID���
						ID_info[i][3] = 0; //�赲֡�����
					}
				}
			}
			else   //���������ݣ�˵�����ڱ�tracking
			{				
				if (ID_info[i][2] == 0)  //δ��ʶ��׷���ߣ���Ϊ�����Ľ�����
				{
					int indurance = 0;
					for (int k = 1; k <= 10; k++)  //������ʷ��Ϣ��ɸѡ���Ѿ���������10֡�ĵ�
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

						for (int j = 0; j < 18; j++)       //����������������ͷ�еĽڵ�j
						{
							int a = i / 6;
							int b = j / 6;
							//if (a != b)  //������ͬһ������ͷ
							{
								if ((ID_info[j][0] != 0 || ID_info[j][1] != 0)  && ID_info[j][2] != 0)       //j��׷����Ϣ
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

									//if (total_distance< 400 && varance < 400)  //i,j ��һ����,����j��ID��ʶ
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
							
							if (ID_info[match_index][3] > 0)//���ڱ��ڵ��׶�,����ڵ��ɹ�������Ԥ��
							{
								ID_info[match_index][3] = 0;
								ID_info[match_index][2] = 0;
							}
						}

						if (match_flag == 0) //δ��ʾ������û��ƥ��ɹ�
						{
							if (current_ID < 10000)
							{
								ID_info[i][2] = current_ID;
								current_ID++;
								if (current_ID > 2)
									cout << "opps" << endl;
							}
							else
								cout << "�����Ѵ�����" << endl;
						}
					}
				}
				else  //�ѱ�ʾ��׷���ߣ���ID��Ϣ
				{
					if (ID_info[i][3] > 0)  //֮ǰ���ڴ����ڵ�״̬�ĵ㣬�Ա��³��ֵĵ��Ԥ��ľ���
					{
						int my_frame = 0; //ǰһ֡�±�
						if (histroy_count - 1< 0)
							my_frame = histroy_count - 1 + 100;
						else
							my_frame = histroy_count - 1;

						if ((abs(ID_info[i][0] - histroy_info[i][0][my_frame]) + abs(ID_info[i][1] - histroy_info[i][1][my_frame])) < 100)  //��Χ���ڣ��赲���������ID
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
		for (int i = 0; i<18; i++)    //histroy_infoͬ��copyԤ���� X��Z��Ϣ
			for (int j = 0; j<2; j++)
				histroy_info[i][j][histroy_count] = ID_info[i][j];

		histroy_count++;
		if (histroy_count == 100)
			histroy_count = 0;  //ˢ�´洢֡��

		Set user_info;
		for (int i = 0; i < 18; i++)       //�������нڵ�,����map
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
