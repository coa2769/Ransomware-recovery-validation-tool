#include "stdafx.h"
#include "TestOSInfo.h"
#include"Manager_v1.0.h"
#include"MainFrm.h"
#include"HashFuntion.h"

#include<locale.h>

////////////////////////////////////////////////////////////////////////////////////////////

CTestOSInfo::CTestOSInfo(TEST_OS os)
	:m_os(os),
	m_state(STATE_DISABLE),
	m_nReportCount(0)
{
	InitResult();
}


CTestOSInfo::~CTestOSInfo()
{
	ReleaseResult();
}

void CTestOSInfo::CountRecoveryFile(void)
{
	m_nTestFileCount = m_listResult.GetSize();

	POSITION rPos = m_listResult.GetHeadPosition();
	FILE_INFO *file;
	while (rPos)
	{
		file = m_listResult.GetNext(rPos);
		if (SameValue(file->OriginalHash, file->RecoveryHash) == TRUE)
		{
			m_nRecovery++;
		}
	}

}

void CTestOSInfo::ExportData(void)
{
	CMainFrm* Frm = (CMainFrm*)theApp.m_pMainWnd;
	CString FileName;
	CString osName;
	if (m_os == WINDOW_7)
	{
		osName = _T("Window7");
	}
	else if (m_os == WINDOW_8)
	{
		osName = _T("Window8");
	}
	else if (m_os == WINDOW_10)
	{
		osName = _T("Window10");
	}
	FileName.Format(_T("%s결과보고서(%d).csv"), osName, m_nReportCount);;
	m_nReportCount++;

	FILE* fp;
	errno_t err = _wfopen_s(&fp, FileName, _T("wt"));
	
	_wsetlocale(LC_ALL, L"kor");

	CString str;
	//랜섬이름, 복구 도구 이름
	str.Format(_T("OS, %s\n"), osName);
	fwprintf(fp, str);
	str.Format(_T("랜섬웨어,복구도구\n"));
	fwprintf(fp, str);
	str.Format(_T("%s, %s\n"), Frm->m_CRansomwareName, Frm->m_CRecoveryName);
	fwprintf(fp, str);

	//감염시작, 감염종료, 복구시작, 복구 종료
	str.Format(_T("테스트 날짜, 감염시작, 감염종료, 복구시작, 복구종료\n"));
	fwprintf(fp, str);
	str.Format(_T("%hd년 %hd월 %hd일, %hd:%hd:%hd, %hd:%hd:%hd, %hd:%hd:%hd, %hd:%hd:%hd\n"),
		m_TestTime.InfecionStart.wYear, m_TestTime.InfecionStart.wMonth, m_TestTime.InfecionStart.wDay,
		m_TestTime.InfecionStart.wHour, m_TestTime.InfecionStart.wMinute, m_TestTime.InfecionStart.wSecond,
		m_TestTime.InfectionEnd.wHour, m_TestTime.InfectionEnd.wMinute, m_TestTime.InfectionEnd.wSecond,
		m_TestTime.RecoveryStart.wHour, m_TestTime.RecoveryStart.wMinute, m_TestTime.RecoveryStart.wSecond,
		m_TestTime.RecoveryEnd.wHour, m_TestTime.RecoveryEnd.wMinute, m_TestTime.RecoveryEnd.wSecond);
	fwprintf(fp, str);

	//테스트한 총 파일 개수, 복구한 파일 개수
	str.Format(_T("테스트한 총 파일 개수, 복구한 파일 개수\n"));
	fwprintf(fp, str);
	str.Format(_T("%d, %d\n"), m_listResult.GetSize(), m_nRecovery);
	fwprintf(fp, str);

	//파일 명, 원본 Hash, 감염 Hash, 복구 Hash
	str.Format(_T("파일명, 원본 Hash, 감염 Hash, 복구 Hash, 복구\n"));
	fwprintf(fp, str);
	
	POSITION rPos = m_listResult.GetHeadPosition();
	FILE_INFO *file;
	
	CString ori, infe, recov, temp;
	
	while (rPos)
	{
		file = m_listResult.GetNext(rPos);

		ori = _T("");
		for (int i = 0; i < 20; i++)
		{
			temp.Format(_T("%02X"), file->OriginalHash[i]);
			ori += temp;
		}

		infe = _T("");
		for (int i = 0; i < 20; i++)
		{
			temp.Format(_T("%02X"), file->InfectionHash[i]);
			infe += temp;
		}

		recov = _T("");
		for (int i = 0; i < 20; i++)
		{
			temp.Format(_T("%02X"), file->RecoveryHash[i]);
			recov += temp;
		}
		
		str.Format(_T("%s, %s, %s, %s"), file->FileName, ori, infe, recov);
		if (SameValue(file->OriginalHash, file->RecoveryHash) == TRUE)
		{
			str += _T(", TRUE\n");
		}
		else
		{
			str += _T(", FALSE\n");
		}

		fwprintf(fp, str);

	}

	fclose(fp);
}