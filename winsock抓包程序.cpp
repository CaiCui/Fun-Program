// CaptureData.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <stdlib.h>
#include <winsock2.h>
#include <mstcpip.h>
#include<filesystem>

#pragma  comment (lib, "WS2_32.lib") //���ض�̬�� ws2_32.dll (system32)

#define  OK 0
#define ERR -1
#define MY_PORT 10000 //���˿�һ���� 1024 ���ϵĶ˿ڣ�1024���µĶ˿ںܶ�ϵͳ��ĳЩ����ռ��

#define HOST_NAME_LEN 256
#define BUFF_LEN 65535

/*
Э�����ͣ�
1  ICMP
2  IGMP
6  TCP
17 UDP
88 IGRP
89 OSPF
*/

typedef struct IP_HEADER
{
	char cVer : 4;
	char cHeadLen : 4;
	char cTOS;
	unsigned short usLen;
	unsigned short usIdent;
	unsigned short usFlagOffset;
	char cTTL;
	char cProtocol;
	unsigned short usChkSum;
	unsigned int   uiSrcIp;
	unsigned int   uiDstIp;

}IP_HEADER_S;


void PrintData(char szBuf[], int iLen)
{
	int i;
	IP_HEADER_S *pHeader;
	SOCKADDR_IN stAddr = { 0 };
	//printf("RCV: %s\n", szBuf);
	//���������һ�������ַ���
	//��ӡ 16 ��������

	/*
	for (i = 0; i < iLen; i++)
	{
	printf("%02x ",szBuf[i] & 0xff);
	}
	*/
	pHeader = (IP_HEADER_S*)szBuf;
	if (pHeader->cProtocol != 6)
	{
		//return;
	}

	printf("\nRCV:\n");
	printf("Version:%d\n", pHeader->cVer);
	printf("IP len:%d\n", pHeader->cHeadLen);
	printf("TOS:%d\n", pHeader->cTOS);
	printf("Data Len:%d\n", ntohs(pHeader->usLen)); //ע���ֽ���ת��
	printf("TTL:%d\n", pHeader->cTTL);
	printf("Protocol:%d\n", pHeader->cProtocol);

	stAddr.sin_addr.s_addr = pHeader->uiSrcIp;  //stAddr.sin_addr.s_addr ����������ֽ����int
	//inet_ntoa �� in_addr ת���ַ�����ip��ַ
	printf("Source IP:%s\n", inet_ntoa(stAddr.sin_addr));

	stAddr.sin_addr.s_addr = pHeader->uiDstIp;
	printf("Dest IP:%s\n", inet_ntoa(stAddr.sin_addr));

	printf("\n");
	return;
}

int CaptureData()
{
	SOCKET iFd;
	char szHostName[HOST_NAME_LEN]; //�������
	HOSTENT *pHost;
	SOCKADDR_IN stAddr = { 0 };
	DWORD  dwInBuff = 1;
	DWORD  dwOutBuff;
	DWORD  dwBytesRet;
	char   szBuf[BUFF_LEN];
	int iRet;


	WSADATA  stWsa;  //���ڴ����ǵĶ�̬���ӿ⣬�������̽ӿڿ���

	//0x0202 �汾��
	if (WSAStartup(0x0202, &stWsa) != OK)
	{
		printf("Wsa startup failed.\n");
		return ERR;
	}

	if (gethostname(szHostName, HOST_NAME_LEN) != OK)
	{
		WSACleanup();
		return ERR;
	}

	printf("hostName:%s\n", szHostName);
	pHost = gethostbyname(szHostName);

	printf("MyIP: %d.%d.%d.%d\n", pHost->h_addr_list[0][0] & 0xff,
		pHost->h_addr_list[0][1] & 0xff,
		pHost->h_addr_list[0][2] & 0xff,
		pHost->h_addr_list[0][3] & 0xff);

	//������ʾ��IP
	stAddr.sin_addr.s_addr = *(unsigned long*)pHost->h_addr;
	stAddr.sin_family = AF_INET;
	stAddr.sin_port = htons(MY_PORT);

	//����ԭʼ��������ӦΪ����Ҫ���յ����ݲ������Ƿ��� MY_PORT �ģ��������ж˿ڵ�
	iFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);//ICMP ping �İ�
	if (iFd < 0)
	{
		WSACleanup();
		return ERR;
	}

	if (bind(iFd, (SOCKADDR*)&stAddr, sizeof(SOCKADDR)) != OK)
	{
		WSACleanup();
		closesocket(iFd);
		return ERR;
	}

	//����iFd�հ�����SIO_RCVALL �������еİ� 
	iRet = WSAIoctl(iFd, SIO_RCVALL, &dwInBuff, sizeof(dwInBuff),
		&dwOutBuff, sizeof(dwOutBuff), &dwBytesRet, NULL, NULL);
	if (iRet != OK)
	{
		printf("IOCTL failed.\n");
		WSACleanup();
		closesocket(iFd);
		return ERR;
	}

	while (true)
	{
		memset(szBuf, 0, BUFF_LEN);
		iRet = recv(iFd, szBuf, BUFF_LEN, 0);
		if (iRet < 0)
		{
			break;
		}
		else
		{
			PrintData(szBuf, iRet);
		}
	}

	WSACleanup();
	return OK;
}

int _tmain(int argc, _TCHAR* argv[])
{
	CaptureData();
	system("pause");
	return 0;
}

