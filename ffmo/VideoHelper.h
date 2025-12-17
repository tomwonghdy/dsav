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
#ifndef __VIDEO_HELPER_H_INCLUDED_
#define __VIDEO_HELPER_H_INCLUDED_

#include "ffmo.h"

#include "avlib.h"
#include "..\davsdk\includes\mox.h"



class   CVideoHelper
{
public:
	CVideoHelper(void);
	~CVideoHelper(void);

	BOOL Open(const char* strFilePath, int* pErrCode);
	void Close();

	//position is in milisecond.
	M_RESULT       RetrievePicture(__int64 position,  MxVideoDesc* pDesc, void* pDataBuffer, UINT nDataSize);

	//ºÁÃëÎªµ¥Î»
	__int64        GetDuration();
	float          GetFrameRate();
 
	//position in milisecond
	M_RESULT       Seek(__int64 position);
	M_RESULT       Read(MxVideoDesc* pDesc, void* pDataBuffer, UINT nDataSize, BOOL bKeyFrame);


	AVPixelFormat  GetPixelFormat();
	int            GetVideoWidth();
	int            GetVideoHeight();

	const char*    GetCodecName();


	BOOL  m_bRequestAbort ;


private:
	int   OpenStreamComponent( int stream_index);
	void  CloseStreamComponent( int stream_index);

	int   m_nStreamIndex;

	HANDLE m_hSource;

	

	/*AVFormatContext*/void * m_pFmtCxt  ;
	/*AVStream*/void*         m_pStream; 

	/*AVDictionary*/void * m_pFmtOpts;
	/*AVDictionary*/void* m_pCodecOpts;
 

};












#endif
