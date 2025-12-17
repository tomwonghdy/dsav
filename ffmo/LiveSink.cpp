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
#include "LiveSink.h"
#include "avlib.h"

#include <rvb\dsm.h>
  
  
CLiveSink::CLiveSink(void)
{ 

}

CLiveSink::~CLiveSink(void)
{

}


static DWORD WINAPI ThreadFileSinkProc(LPVOID lpParameter)
{
	CLiveSink* pDa = (CLiveSink*)lpParameter;

	pDa->HandleMux();

	return 0;
}
 

M_RESULT CLiveSink::Play(const char* strUrl )
{
	if (m_nState == MS_PLAYING) return M_FAILED;

	if (NULL == strUrl) return M_FILE_NONEXIST;
	 
	ResetFrameArrays();
	  
	FfEncodeContext* pCxt = InitEncodeContext();
	RV_ASSERT(pCxt);
	this->m_hEncodeContext = pCxt;

	const char* strFormatName = NULL;
	
	int type = GetLiveType(strUrl);
	if (type == SST_RTSP) {
		strFormatName = "rtsp";
	}
	else if (type == SST_RTMP) {
		strFormatName = "rtmp";
	}
	else {
	   return	M_UNSUPPORT;
	}

	BOOL ret = InitContext(pCxt, strFormatName, strUrl);

	if (!ret) {
		ReleaseContext();
		return M_FAILED;
	}

	// 设置输出参数
	av_dict_set(&pCxt->pOption, "rtsp_transport", "tcp", 0);  
	av_dict_set(&pCxt->pOption, "buffer_size", "1024000", 0);
	av_dict_set(&pCxt->pOption, "max_delay", "500000", 0);
	av_dict_set(&pCxt->pOption, "stimeout", "10000000", 0);

	if (!OpenDiskFile(strUrl, &ret)) {
		ReleaseContext();
		return M_FAILED;
	}
	 

	m_nState = MS_PLAYING;

	m_hOutputThread = CreateThread(NULL, 0, ThreadFileSinkProc, this, NULL, NULL);
	RV_ASSERT(m_hOutputThread);

	return M_OK;
	 

}


void CLiveSink::Stop()
{  
	CFfSink::Stop();
}
  
 

void CLiveSink::Pause(BOOL bResume)
{	

}
    
int  CLiveSink::GetLiveType(const char* strUrl)
{
	if (NULL == strUrl) return SST_UNK;

	CUtString s = strUrl;
	s = s.Left(4);
	  
	if (stricmp((const char*)s, "rtsp") == 0) {
		return SST_RTSP;
	}
	else if (stricmp((const char*)s, "rtmp") == 0) {
		return SST_RTMP;
	}

	return SST_UNK;

}


BOOL CLiveSink::SetVideoCodecOptions(int codecId)
{
	FfEncodeContext* pCxt = (FfEncodeContext*)this->m_hEncodeContext;
	RV_ASSERT(pCxt);

	FfOutputStream* ost =  &pCxt->videoStream ;
 
	ost->encode->max_b_frames = 0;  // 无B帧，降低延迟
	  
	// 设置H.264参数
	int ret = av_opt_set(ost->encode->priv_data, "preset", "ultrafast", 0);
	RV_ASSERT(ret);

	ret = av_opt_set(ost->encode->priv_data, "tune", "zerolatency", 0);
	RV_ASSERT(ret);

	ret = av_opt_set(ost->encode->priv_data, "profile", "baseline", 0);
	RV_ASSERT(ret);

	return TRUE;
}

BOOL CLiveSink::SetAudioCodecOptions(int codecId)
{
	return TRUE;
}


void CLiveSink::HandleMux()
{
	FfEncodeContext* pCxt = (FfEncodeContext*)this->m_hEncodeContext;
	RV_ASSERT(pCxt);
	RV_ASSERT(m_lstAudioFrame && m_lstVideoFrame);
	RV_ASSERT(pCxt->pFmtCxt);
	 
	BOOL  bEncodeVideo = (this->m_nOutputType & MX_MT_VIDEO) == MX_MT_VIDEO && pCxt->bHaveVideo;
	BOOL  bEncodeAudio = (this->m_nOutputType & MX_MT_AUDIO) == MX_MT_AUDIO && pCxt->bHaveAudio;

	int ret = 0;
	RV_ASSERT((bEncodeVideo || bEncodeAudio));

	while (TRUE)
	{
		ret = Mux(FALSE, FALSE, bEncodeVideo, bEncodeAudio);
		if (MR_NODATA == ret)
		{
			if (m_nState == MS_STOPPED) {
				break;
			}
			Sleep(15);
		}
		else if (ret < 0) {
			break;
		}
		else {
			Sleep(15);
		}
	}

	if (ret >= 0) {
		RV_ASSERT(m_nState == MS_STOPPED);
		 
		{
			FfOutputStream* ost = &pCxt->audioStream;
			int fifosze = (ost->fifo ? av_audio_fifo_size(ost->fifo) : 0);

			while (rlsGetCount(m_lstAudioFrame) > 0 || fifosze > 0 || rlsGetCount(m_lstVideoFrame) > 0)
			{
				//	PrintToOutput("audio frame left: %d\r\n", rlsGetCount(m_lstAudioFrame));

				ret = Mux(FALSE, TRUE, bEncodeVideo, bEncodeAudio);
				if (ret < 0 || ret == MR_EOF) {
					break;
				}

				if (!(bEncodeVideo || bEncodeAudio)) {
					break;
				}
			}

			//flush
			if ((this->m_nOutputType & MX_MT_VIDEO) == MX_MT_VIDEO)
			{
				FfOutputStream* ost = &pCxt->videoStream;
				ret = write_frame(pCxt->pFmtCxt, ost->encode, ost->stream, NULL, ost->packet);
			}
			if ((this->m_nOutputType & MX_MT_AUDIO) == MX_MT_AUDIO) {
				FfOutputStream* ost = &pCxt->audioStream;
				ret = write_frame(pCxt->pFmtCxt, ost->encode, ost->stream, NULL, ost->packet);
			}
		}
		 
	}
	else {
		RaiseErrorReport(ret);
	}

}
