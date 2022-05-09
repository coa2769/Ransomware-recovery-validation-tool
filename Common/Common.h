#pragma once
#include<stdio.h>

typedef enum _TEST_OS
{
	WINDOW_7,
	WINDOW_8,
	WINDOW_10
}TEST_OS;

typedef struct FILE_INFO
{
	wchar_t FileName[FILENAME_MAX];
	unsigned long FileSize;				//파일 사이즈
	unsigned char OriginalHash[20];		//BYTE
	unsigned char InfectionHash[20];	//BYTE
	unsigned char RecoveryHash[20];		//BYTE
}FILE_INFO;

typedef enum Head_Type
{
	ACCEPT_AGENT = 1,
	FILE_RASOM,
	FILE_RASOM_ERROR,
	FILE_RECOVERY,
	FILE_RECOVERY_ERROR,
	SAVE_RASOMWARE_HASH,
	SAVE_RECOVERY_HASH,
	START_RASOM,
	END_RASOM,
	START_RECOVERY,
	END_RECOVERY,
	TEST_PROGRESS,
	TEST_RESULT,
	HEALTH_CHECK,
	ALL_STOP
}HEAD_TYPE;

typedef struct Tcp_H
{
	HEAD_TYPE type;
	size_t TestFileCount;	//전체 파일 개수
	size_t ResultCount;	//감염 또는 복구가 완료된 파일 개수
	FILE_INFO  FileInfo;//파일 정보에 대한 			
}TCP_H;

