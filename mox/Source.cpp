// Copyright (c) 2025 Tom Wong  
// Email:  buffi@163.com
// SPDX-License-Identifier: MIT
// 
// RVB website   : http://www.rvb.net.cn/
// FFMPEG website: https://www.ffmpeg.org/
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "stdafx.h"
#include "Source.h"

#include <stdlib.h> 

CSource::CSource(void)
{

	//m_hExitEvent = NULL;
	//m_nOutputCount =0;
	//m_pOutputPad  = 0;

	/*m_hExitEvent = ::CreateEvent(NULL,TRUE,FALSE,NULL);
	RV_ASSERT(m_hExitEvent);*/

	m_fnSamplePass =NULL;
	 
	//m_hGameOverEvent = NULL;
	m_nPlayDuration = -1;
	m_nStartPos = 0;



	m_nStreamFlags = ST_NONE;

}

CSource::~CSource(void)
{
	/*if (m_hExitEvent){
		::CloseHandle(m_hExitEvent);
	}

	if (m_hPlayThread){
		::CloseHandle(m_hPlayThread);
		m_hPlayThread=NULL;
	}*/
	
}

//
//void CSource::RaiseGameOverEvent()
//{
//
//	if (m_hGameOverEvent)
//	{
//		::SetEvent(m_hGameOverEvent);
//	}
//
//}

BOOL  CSource::HasVideoStream()
{
	return ((ST_VIDEO & m_nStreamFlags) == ST_VIDEO);
}

BOOL  CSource::HasAudioStream()
{
	return ((ST_AUDIO & m_nStreamFlags) == ST_AUDIO);
}

BOOL  CSource::HasSubtitleStream()
{
	return ((ST_SUBTITLE & m_nStreamFlags) == ST_SUBTITLE);
}

BOOL  CSource::HasActivePageStream()
{
	return ((ST_ACTIVEPAGE & m_nStreamFlags) == ST_ACTIVEPAGE);
}

BOOL CSource::ParseParam(const char* strText, const char* strName, char* strValue, int valueSize)
{
	if (NULL == strText || strName == NULL  || strValue == NULL || valueSize<=0){
		return FALSE;
	}

	int len = (int)strlen(strText);

	char* strNew = new char[len+1];
	if (NULL == strNew) return FALSE;

	strcpy_s(strNew, len+1, strText);

	char* tmp = (char*)strText;

	char* strStart = strstr(tmp, strName);
	if (NULL == strStart){
	    return FALSE;
	}


	char* str = strstr(strStart, "&");
	 
	if (str){
		str[0] = '\0';
	}

	str = strstr(strStart, "=");

	char* right = str +1;
	//str[0] = '\0';

	//if (_stricmp(tmp, strName ) == 0)
	{
		int len =(int) strlen(right);

		if (len < valueSize){
			strcpy_s(strValue, valueSize, right);

			delete[] strNew;

			return TRUE;
		}
	}
	 
	delete[]strNew;
	return FALSE;

}

void  CSource::ParseParam(const char* strText)
{
	
	
	char left[64];

	strcpy_s(left, 64, strText);

	char* str = strstr(left, "=");


	if (str){
		 
		char* right = str +1;
		str[0] = '\0';

		if (strcmp(left, "startpos" ) >= 0){
			m_nStartPos = atoi(right);
			return;
		}

		if (strcmp(left, "duration" ) >= 0){
			m_nPlayDuration = atoi(right);
			return;
		}
	}
}

	
BOOL CSource::ParseField(const TCHAR* _strText, const TCHAR* strName, TCHAR* strValue, int valueSize)
{

	if (NULL == _strText || strName == NULL  || strValue == NULL || valueSize<=0){
		return FALSE;
	}

	//int len = strlen(_strText);

	//char* strNew = new char[len+1];
	//if (NULL == strNew) return FALSE;

	//strcpy_s(strNew, len+1, _strText);

	CUtString ms(_strText);
	char* tmp = (char*)ms;

	char* strStart = strstr(tmp, strName);
	if (NULL == strStart){
	    return FALSE;
	}
	 
	char* str = strstr(strStart, "&");
	 
	if (str){
		str[0] = '\0';
	}

	str = strstr(strStart, "=");

	char* right = str +1;
	 

	int len = (int)strlen(right);

	if (len < valueSize){
		strcpy_s(strValue, valueSize, right);

	//	delete[] strNew;

		return TRUE;
	}
 
	 
//	delete[]strNew;
	return FALSE;
}
	
BOOL CSource::ParseField(const TCHAR* strText, const TCHAR* strName, int* pValue)
{
	char tmp[64];

	if (ParseField(strText, strName, tmp, sizeof(tmp))){
		 if (pValue) *pValue = atoi(tmp);
	     return TRUE;
	}
	 
	return FALSE;
}
	
BOOL CSource::ParseField(const TCHAR* strText, const TCHAR* strName, float* pValue)
{
	char tmp[128];

	if (ParseField(strText, strName, tmp, sizeof(tmp))){
		 if (pValue) *pValue =(float) atof(tmp);
	     return TRUE;
	}
	 
	return FALSE;
}
	
BOOL CSource::ParseField(const TCHAR* strText, const TCHAR* strName, bool* pValue)
{
	char tmp[64];

	if (ParseField(strText, strName, tmp, sizeof(tmp))){
		 if (pValue) *pValue = atoi(tmp)?true:false;
	     return TRUE;
	}
	 
	return FALSE;
}
	
BOOL CSource::ParseField(const TCHAR* strText, const TCHAR* strName, double* pValue)
{
	char tmp[64];

	if (ParseField(strText, strName, tmp, sizeof(tmp))){
		 if (pValue) *pValue = atof(tmp);
	     return TRUE;
	}
	 
	return FALSE;

}


char* CSource::ParseUrl(const char* strUrl, char* strNew, int nNewSize)
{
	if (strUrl == NULL || NULL == strNew ) return strNew;
	RV_ASSERT(strNew);

	strcpy_s(strNew, nNewSize, strUrl);

	_strlwr_s(strNew, nNewSize );

	char buf[128];
	
	char* str = strstr(strNew, "file://");

	//去掉file://头
	if ( str == strNew)
	{
		 memmove(strNew, strNew + 7, strlen(strNew)+1 - 7);		
	}
	else{
		//如果是网络资源，直接返回。网络资源不支持位置和播放长度等参数。
	     str = strstr(strNew, "dev://");
		 if (NULL == str) return strNew;
	}
	
	str = strstr(strNew, "?");

	if (str){
		char* tmp,   *strRight;

		strcpy_s(buf, 128, str +1);

		while(TRUE){

			tmp = strstr(buf, "&");

			if (tmp) tmp[0] = '\0';
			
			ParseParam( buf);

			if (tmp){
				strRight = tmp+1;
				//tmp[0] = '\0';

				strcpy_s(buf, 128, strRight);
			}
			else{
				break;
			}

		}

		str[0] = '\0';
	}
	
	return strNew;

}

void   CSource::SetSamplePassHandler(MO_SAMPLE_PASS_FUNC pfnSamplePass )
{
	m_fnSamplePass = pfnSamplePass;
}

MO_SAMPLE_PASS_FUNC CSource::GetSamplePassHandler()
{
	return m_fnSamplePass;
}