#include "stdafx.h"
#include "TextParsing.h"


CTextParsing::CTextParsing()
{
}


CTextParsing::~CTextParsing()
{
}

void CTextParsing::FileNameParsing(CString& fileName)
{
	int count;

	//역순으로 그 문자가 있는 인덱스를 찾아 준다.
	count = fileName.ReverseFind('\\');
	//index값 부터 뒤의 문자열을 얻을 수 있다.
	fileName = fileName.Mid(count + 1);

}