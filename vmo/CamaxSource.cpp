/////////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2025 Tom Wong  
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
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CamaxSource.h"

#include "..\davsdk\includes\ffmo.h"
#include "..\cblibs\includes\CamBridge.h"

//#include <Vfw.h>

#define MAX_BUFFER_COUNT   2


#define WC_MAX_CAMERA_COUNT  10




//class CUtString
//{
//public:
//	CUtString(const char* _strUrl)
//	{
//		m_str=NULL;
//		if (_strUrl)
//		{
//			int len =strlen(_strUrl) + 1;
//			m_str = new char[len];
//			MX_ASSERT(m_str);
//			strcpy_s(m_str, len, _strUrl);
//		}
//
//	}
//	~CUtString()
//	{
//		if (m_str){
//		    delete[] m_str;
//			m_str=NULL;
//		}
//	}
//
//	char* GetPtr(){ return m_str;};
//private:
//	char* m_str;
//
//};


void SetCamParams(HANDLE hCam,  char* strOption){
    if (hCam == NULL || strOption == NULL) return ;

	CUtString ms(strOption);
	char* strItem = ms ;

	while(TRUE){	    
		char* str =strstr(strOption, "&");
		if (str){
		    str[0] = '\0';
			strItem = strOption;
			strOption = str+1;
			
			str = strstr(strItem, "=");
			if (NULL == str){
			    continue;
			}
			char* strName = strItem;
			char* strValue = str+1;
			str[0] = '\0'; 
			
			if (_stricmp(strName , CS_PARAM_INDEX) == 0){
			    continue;
			}

			if (strlen(strName)>0 && strlen(strValue) >0){
			    rcbSetParam(hCam, strName, atoi(strValue));
			}
			
		}
		else{
			strItem = strOption;

			str = strstr(strItem, "=");
			if (  str){
				char* strName = strItem;
				char* strValue = str+1;
				str[0] = '\0'; 

				if (strlen(strName)>0 && strlen(strValue) >0){
					rcbSetParam(hCam, strName, atoi(strValue));
				}
			}
		
		    break;
		}
	}
}

//BOOL  m_bCamTabInit = FALSE;
//
//struct web_cam_log
//{
//	BOOL     bOpened;
//	HWND     hWnd;
//	CCamaxSource* pCam;	
//}m_webcamTab[WC_MAX_CAMERA_COUNT];


//LRESULT PASCAL FrameCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr) 
//{ 
//    if (!hWnd) 
//        return FALSE; 
//  
//	if (lpVHdr->dwFlags & VHDR_DONE ){
//
//		CCamaxSource* pCam = (CCamaxSource*)capGetUserData(hWnd);;
//
//		RV_ASSERT(pCam);
//		pCam->HanldeVideoData(lpVHdr->lpData, lpVHdr->dwBytesUsed);		 
//
//	}
//
//   /* wsprintf(gachBuffer, "Preview frame# %ld ", gdwFrameNum++); 
//    SetWindowText(hWnd, (LPSTR)gachBuffer); */
//
//    return (LRESULT) TRUE ; 
//} 
// 


// ErrorCallbackProc: error callback function 
// hWnd:              capture window handle 
// nErrID:            error code for the encountered error 
// lpErrorText:       error text string for the encountered error 
// 
//LRESULT PASCAL ErrorCallbackProc(HWND hWnd, int nErrID,  LPSTR lpErrorText) 
//{ 
// 
//    if (!hWnd) 
//        return FALSE; 
// 
//    if (nErrID == 0)            // Starting a new major function. 
//        return TRUE;            // Clear out old errors. 
// 
//	CCamaxSource* pCam = (CCamaxSource*) capGetUserData(hWnd);
//	RV_ASSERT(pCam);
//
////	pCam->RaiseGameOverEvent();
//
//    // Show the error identifier and text. 
//  /*  wsprintf(gachBuffer, "Error# %d", nErrID); 
// 
//    MessageBox(hWnd, lpErrorText, gachBuffer, 
//        MB_OK | MB_ICONEXCLAMATION); */
// 
//    return (LRESULT) TRUE; 
//} 


void  WINAPI  OnFrameArrived(void* pImageData, UINT nImageSize, double timestamp, void* pUserData)
{
	CCamaxSource* pFrame = (CCamaxSource*)pUserData;
	RV_ASSERT(pFrame);

	if (nImageSize <=0 ) return;

	 pFrame->HanldeVideoData(pImageData, nImageSize);

}

CCamaxSource::CCamaxSource(void)
{

	SetDefaults();
}
 
CCamaxSource::CCamaxSource(const TCHAR* strLocation)
{
	//必须先defaults
	SetDefaults();

	//在location
	SetLocation(strLocation);
	
}

CCamaxSource::~CCamaxSource(void)
{
	
	RV_ASSERT(m_seqSample == NULL);
	if(m_strLocation){
	    delete []m_strLocation;
		m_strLocation =NULL;
	}
	
	
	//ReleaseContext();
	if (m_hCamera)
	{
		if (rcbIsOpen(m_hCamera)){
			rcbClose(m_hCamera);
		}
		rcbDestroy(m_hCamera);
		m_hCamera =NULL;
	}

	::DeleteCriticalSection(&m_csState);
	::DeleteCriticalSection(&m_csSample);

	

}


void  CCamaxSource::SetDefaults()
{
	m_nType =-1;	
	m_nCamIndex = -1;
	m_nSubType  = -1;
	m_nStreamFlags = ST_VIDEO;
 
	m_strModule[0] = '\0';
	m_strOption[0] = '\0';

	m_nSerial = 0;

	m_outputPad.m_pOwner = this;	
	SetMode(WM_ACTIVE);
	m_outputPad.SetMediaType(MX_MT_VIDEO);

	m_hCamera =NULL;	 
	m_seqSample = NULL;

	m_strLocation =NULL;

//	ResetVideoDescEx( &m_videoOutDesc, MX_VF_BGR, 640, 480 );
	
	//ResetVideoDescEx( &m_captureDesc, MX_VF_BGR, 320, 240 );
	
	::InitializeCriticalSection(&m_csState);
	::InitializeCriticalSection(&m_csSample);
}

void CCamaxSource::SetLocation(const TCHAR* strLocation)
{
	if(m_strLocation){
	    delete []m_strLocation;
		m_strLocation =NULL;
	}

	if (strLocation){

		int len = strlen(strLocation);
	     
		m_strLocation = new TCHAR[len+1];
		MX_ASSERT(m_strLocation);

		strcpy(m_strLocation, strLocation);
	}
}



M_RESULT CCamaxSource::Play(const char* _strUrl ) 
{

	CUtAutoLock lock(&m_csState);

	if (m_nState != MS_STOPPED ) return M_FAILED;
	if (NULL == _strUrl) return M_FAILED;
	 

	if (!this->ParseURL(_strUrl)){
	    return M_FAILED;
	}

	char* strUrl =m_strModule;
	char strPath[MAX_PATH + CS_MAX_MODULE_NAME_LEN + 3] = {'\0'};

	if (this->m_strLocation){
		int len =strlen(m_strLocation);
		

		if ((strlen(m_strModule) + strlen(m_strLocation)+1)  < sizeof(strPath)){
			strcat(strPath, m_strLocation);
			strcat(strPath, "\\");
			strcat(strPath, m_strModule);

			strUrl = strPath;
		}
	}
	
	if (m_nType == CS_DEFAULT   ){
    	if ( m_nSubType == CS_DEFAULT){ 
			if (this->Open(strUrl, m_nCamIndex)==FALSE){
				return M_FAILED;
			}
		}
		else{
			RV_ASSERT(0);
			return M_FAILED;
		}
	}
	else if (m_nType == CS_FAKE   )
	{
		if ( m_nSubType == CS_DESK){ 
			if (this->Open(strUrl, m_nCamIndex)==FALSE){
				return M_FAILED;
			}
		}
		else if ( m_nSubType == CS_IPDESK){ 
			if (this->Open(strUrl,this->m_strOption)==FALSE){
				return M_FAILED;
			}
		}
		else{
			RV_ASSERT(0);
			return M_FAILED;
		}
	}
	else{
	    return M_FAILED;
	}

	if (NULL == m_hCamera){		
		return M_FAILED;
	} 


	m_nSerial = 0;

	if (m_nWorkMode == WM_PASSIVE){
		m_seqSample = rvCreateSequence(NULL /*mxDestroySample*/);
		RV_ASSERT(m_seqSample);
	}

		
	/*if (! m_imageConverter.Create( GetVideoPixelFormat(m_captureDesc.format) , m_captureDesc.width, m_captureDesc.height  , \
		                    GetVideoPixelFormat(m_videoOutDesc.format) , m_videoOutDesc.width, m_videoOutDesc.height )){

			goto ON_ERR;
	}
	 */
	 
	if (!rcbStart(m_hCamera))
	{
		goto ON_ERR;
	}
	  
	m_nState = MS_PLAYING;

	  
	return M_OK;


ON_ERR:		
	rcbClose(m_hCamera);
	//capDriverDisconnect((HWND)m_hCamera);	 

	return M_FAILED;

}
	
void     CCamaxSource::Pause(BOOL bResume) {
	m_nState = MS_PAUSED;
}

void     CCamaxSource::Stop() {

	CUtAutoLock lock(&m_csState);

	if (m_nState == MS_STOPPED) return;
	 
	MX_ASSERT(m_hCamera);

	//capCaptureStop((HWND)m_hCamera);
	//capDriverDisconnect((HWND)m_hCamera);

	rcbStop(m_hCamera);
	//rcbClose(m_hCamera);

	Close();
	//if (m_hCamera)
	//{
	//	rcbDestroy(m_hCamera);
	//	m_hCamera =NULL;
	//}

	m_nState = MS_STOPPED;

	//m_imageConverter.Destroy();

	if (m_seqSample){

		RvSeqReader sr;
		MX_HANDLE hTmp=NULL;

		RV_BEGIN_READ_SEQ(m_seqSample, &sr);

		while(!RV_IS_SEQ_END(&sr)){
			RV_READ_FR_SEQ_(&sr, MX_HANDLE, hTmp);
			mxDestroySample(hTmp);
		}

		RV_END_READ_SEQ(m_seqSample, &sr);		

		rvDestroySequence(m_seqSample);
		m_seqSample = NULL;
	}

}
 
//
//MX_MEDIA_TYPE CCamaxSource::GetMediaType(CPad* pPad) 
//{
//	RV_ASSERT(pPad == &m_outputPad);
//
//	return MX_MT_VIDEO;
//}
//
//
//BOOL CCamaxSource::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type)
//{
//	return (type == MX_MT_VIDEO);
//}


BOOL CCamaxSource::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	if (pOutputPad == &m_outputPad)
	{
		return pOutputPad->NegotiateDescriptor(m_outputPad.GetFormatDescriptor()); 
	}

	return FALSE;

}

////当接收方通过PAD接收SAMPLE时,产生该事件。
MX_HANDLE CCamaxSource::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode)
{
	RV_ASSERT(m_nWorkMode == WM_PASSIVE);
	//RV_ASSERT(NULL == hSample);

	CUtAutoLock lock(&m_csSample);

	if (m_seqSample)
	{
		int cnt = rsqGetCount(m_seqSample);
		if ( cnt > 0){

			MX_HANDLE hFirstSample  =NULL;
			 
			hFirstSample = rsqRemoveFirst_(m_seqSample );

			RV_ASSERT(hFirstSample);
			
			if (cnt == 1)
			{
				MX_HANDLE hNew = mxDuplicateSample(hFirstSample);

				rsqAddFirst_(m_seqSample,  hFirstSample);

				return hNew;
			}
			else{
			    return hFirstSample;
			}
		}
	 
	}
	 
 
	return NULL;
}


//MxDescriptor*  CCamaxSource::GetFormatDescriptor(CPad* pPad)
//{
//	return ((MxDescriptor*)(&m_captureDesc));
//}


void   CCamaxSource::SetMode(CMediaObject::WORK_MODE mode)
{
	if (m_nWorkMode == mode) return ;

	m_nWorkMode = mode;

	if (m_nWorkMode == WM_ACTIVE)
	{
		m_outputPad.SetCaptureType(CPad::CT_PUSH);
	}
	else if (m_nWorkMode == WM_PASSIVE)
	{
		m_outputPad.SetCaptureType(CPad::CT_PULL);
	}

}

void  CCamaxSource::HanldeVideoData(void* pDataBuffer,  DWORD dwDataSize)
{
	RV_ASSERT(m_hCamera);
	//CUtAutoLock lock(&m_cs);

	if (::TryEnterCriticalSection(&m_csState) == FALSE){
		return;
	}

	if (m_nState != MS_PLAYING)
	{
		::LeaveCriticalSection(&m_csState);
		 
		return;
	}



	MX_HANDLE hNew =NULL;
	//if (m_bNoConvert ==FALSE){

	//	RV_ASSERT(dwDataSize == m_captureDesc.dataSize);
	//    RV_ASSERT(pDataBuffer);

	//	UINT nDstSize = m_imageConverter.GetDestBufferSize();	
	//	RV_ASSERT(	nDstSize == m_videoOutDesc.dataSize );

	//	hNew = mxCreateSampleEx((MxDescriptor *)&m_videoOutDesc, sizeof(MxVideoDesc), nDstSize);
	//	RV_ASSERT(hNew);

	//	BOOL ret = m_imageConverter.ExcuteEx((uint8_t*)pDataBuffer, dwDataSize, (uint8_t*)mxGetSampleData(hNew) , nDstSize);
	//	RV_ASSERT(ret); 

	//	

	//}
	//else
	{
		MxVideoDesc* pDesc =(MxVideoDesc*) m_outputPad.GetFormatDescriptor();
		
	    hNew = mxCreateSample((MxDescriptor *)pDesc, sizeof(MxVideoDesc), pDataBuffer,  dwDataSize);
		RV_ASSERT(hNew);
	}

	__int64  pts = m_nSerial;// GetAvTime();
	double rate =RV_MAX(0.05, rcbGetFrameRate(m_hCamera));

	mxSetSampleOptions(hNew, pts, NULL, m_nSerial, 1./ rate);
	
	if (WM_ACTIVE == m_nWorkMode)
	{		
		if (m_outputPad.IsConnected()){
			if (!m_outputPad.Pass(hNew, NULL)){
				mxDestroySample(hNew);
			}
		}
		else
		{
			if (this->m_fnSamplePass){
				m_fnSamplePass(this, &m_outputPad, hNew, this->m_pUserData);
			}
			mxDestroySample(hNew);
		}
	}
	else
	{		

		if (m_seqSample)
		{ 
			if (::TryEnterCriticalSection(&m_csSample)){ 

				if (rsqGetCount(m_seqSample) < MAX_BUFFER_COUNT){
					rsqAddLast_(m_seqSample, hNew);			 
				}
				else{
					MX_HANDLE hFirstSample=NULL;

					while( rsqGetCount(m_seqSample) >= MAX_BUFFER_COUNT){
						hFirstSample = rsqRemoveFirst_(m_seqSample );
						mxDestroySample(hFirstSample);
					}

					rsqAddLast_(m_seqSample, hNew);
					//TRACE("Audio cap buffer is full, a sample was abandoned.\r\n ");
				}

				::LeaveCriticalSection(&m_csSample);
			}
			else{
				mxDestroySample(hNew);
			}
		}
		else{
			mxDestroySample(hNew);
		}

	}

	::LeaveCriticalSection(&m_csState);
	
}

int   CCamaxSource::GetDeviceCount(const TCHAR* strModel)
{
	HANDLE hCam = rcbCreate(strModel, NULL, 0);
	if (NULL == hCam) return 0;

	int n = rcbEnumerateDevice(hCam);
	rcbDestroy(hCam);

	return n;  //reference: capGetDriverDescription - Index of the capture driver. The index can range from 0 through 9.
}

 
RvImage capture_cam(HANDLE hCam)
{
	RvImage im=NULL;

	int w =rcbGetWidth(hCam);
	int h = rcbGetHeight(hCam);
	int dep =24;

	if (!rcbGetParam(hCam, "pixeldepth", &dep ))
	{
		return im;
	}

	MX_ASSERT(w>0 && h>0);

	im =rvCreateImage((dep==8?RIT_GRAY:RIT_RGB), w,h);
	MX_ASSERT(im);

	if (rcbCapture(hCam, im) ==FALSE)
	{
		rvDestroyImage(im);
		im = NULL;
	}


	return im;
}
	
RvImage CCamaxSource::GetStillImage(const TCHAR* strModel, const TCHAR* strParams)
{
	if (NULL == strModel) return NULL;
	if (NULL == strParams) return NULL;


	HANDLE hCam = rcbCreate(strModel, NULL , 0);
	
	if(NULL == hCam) return NULL;
	 
	if (!rcbOpen(hCam,(int) strParams, NULL /*m_hParentWnd*/))
	{
		rcbDestroy(hCam);
		return FALSE;
	}

	RvImage im = capture_cam(hCam);

	rcbClose(hCam);

	rcbDestroy(hCam);
 
	return im;
}



RvImage CCamaxSource::GetStillImage(const TCHAR* strModel, int camIndex)
{
	if (NULL == strModel) return NULL;

	HANDLE hCam = rcbCreate(strModel, NULL , 0);
	
	if(NULL == hCam) return NULL;
	 
	if (!rcbOpen(hCam, camIndex, NULL /*m_hParentWnd*/))
	{
		rcbDestroy(hCam);
		return FALSE;
	}

	RvImage im = capture_cam(hCam);

	rcbClose(hCam);

	rcbDestroy(hCam);
 
	return im;

}

BOOL CCamaxSource::GetDeviceDescription(const TCHAR* strModel, int index, char* strDescHolder, int nHolderSize)
{ 
	HANDLE hCam = rcbCreate(strModel, NULL, 0);
	if (NULL == hCam) return  FALSE;

	BOOL b= rcbGetDeviceString(hCam, index, strDescHolder, nHolderSize ) ;
	rcbDestroy(hCam);

	if (nHolderSize > 0){
		 
	     char* sub = strstr(strDescHolder, "name");
		 if (sub){
		     strcpy(strDescHolder, sub);

			 sub = strstr(strDescHolder, ",");
			 if (sub){
				 sub[0]='\0';
			 }

			 sub = strstr(strDescHolder, "=");
			 if (sub){
				 strcpy(strDescHolder, sub+1);
			 }

		 }
	}

	return b;
	 
}

void  CCamaxSource::SetFlipType(int type)
{
	if (m_hCamera){
	    rcbSetParam(m_hCamera, "Flip", type);
	}
}


BOOL  CCamaxSource::SetParam(const TCHAR* strName, int nValue)
{
	if (m_hCamera){
	    return rcbSetParam(m_hCamera, strName, nValue);
	}

	return FALSE;
}
void  CCamaxSource::ShowParamWin()
{
	if (m_hCamera){
	    rcbShowOptionWindow(m_hCamera );
	}
}

BOOL  CCamaxSource::IsParamWinSupport()
{
	if (m_hCamera){
		 RVC_CAPABILITY cap;
		 if (rcbGetCapability (m_hCamera, &cap, sizeof(RVC_CAPABILITY))){
			 return cap.bWindowOptions;
		 }
	}

	return FALSE;
}

BOOL  CCamaxSource::Open(const TCHAR* strModule, const TCHAR* strOptions )
{
	MX_ASSERT(m_hCamera == NULL);
	if (strOptions == NULL) return FALSE;

	m_hCamera = rcbCreate(strModule, NULL , 0);
	
	
	if(NULL == m_hCamera) return FALSE;

	rcbSetFrameArrivalCallback(m_hCamera, OnFrameArrived);
	rcbSetUserData(m_hCamera, this);
	//if (strUrl){
	//	m_nDeviceID = ParseDeviceID(strUrl);
	//	if (((int) m_nDeviceID )< 0){
	//		return M_INVALID_OBJECT;
	//	}
	//} 

	if (!rcbOpen(m_hCamera, (int)strOptions, NULL /*m_hParentWnd*/))
	{
		rcbDestroy(m_hCamera);
		m_hCamera =NULL;
		return FALSE;
	}

	SetCamParams(m_hCamera, this->m_strOption);

	UpdateDesc();

	return TRUE;
}
 
BOOL  CCamaxSource::Open(const TCHAR* strModule, int  index )
{
	MX_ASSERT(m_hCamera == NULL);
	m_hCamera = rcbCreate(strModule, NULL , 0);
	
	if(NULL == m_hCamera) return FALSE;

	rcbSetFrameArrivalCallback(m_hCamera, OnFrameArrived);
	rcbSetUserData(m_hCamera, this);

	if (!rcbOpen(m_hCamera, index, NULL /*m_hParentWnd*/))
	{
		rcbDestroy(m_hCamera);
		m_hCamera =NULL;
		return FALSE;
	}

	SetCamParams(m_hCamera, this->m_strOption);

	UpdateDesc();
	
	return TRUE;
}
 
void  CCamaxSource::Close()
{
	if (m_hCamera){
		rcbClose(m_hCamera);
	    rcbDestroy(m_hCamera);
	 	m_hCamera =NULL;
	}
}

//
//BOOL  CCamaxSource::SetVideoFormat( )
//{
//	if (NULL == m_hCamera) return FALSE;
//
//	//CAPDRIVERCAPS CapDrvCaps; 
//
//	//HWND hWndC = (HWND)m_hCamera;
//
//	//if (capDriverGetCaps(hWndC, &CapDrvCaps, sizeof (CAPDRIVERCAPS))){
//	//	// Video format dialog box. 
//	//	if (CapDrvCaps.fHasDlgVideoFormat) 
//	//	{
//	//		capDlgVideoFormat(hWndC); 
//
//	//		//// Are there new image dimensions?
//	//		//if (capGetStatus(hWndC, &CapStatus, sizeof (CAPSTATUS))){
//	//		//	
//	//		//}
//
//	//		UpdateDesc();	
//
//	//		 
//	//	} 
//	//}
//	
//	
//	return TRUE;
//}
//
//
//BOOL  CCamaxSource::SetVideoOptions( )
//{
//	if (NULL == m_hCamera) return FALSE;
//
//	//CAPDRIVERCAPS CapDrvCaps; 
//
//	//HWND hWndC = (HWND)m_hCamera;
//
//	//if (capDriverGetCaps(hWndC, &CapDrvCaps, sizeof (CAPDRIVERCAPS))){
//	//	// Video source dialog box. 
//	//	if (CapDrvCaps.fHasDlgVideoSource)
//	//		capDlgVideoSource(hWndC); 
//	//	else if (CapDrvCaps.fHasDlgVideoDisplay) // Video display dialog box. 
//	//	    capDlgVideoDisplay(hWndC); 
//
//	//	if (CapDrvCaps.fHasOverlay) 
//	//		capOverlay(hWndC, TRUE);
//
//	//}
//
//	return TRUE;
//
//}

BOOL  CCamaxSource::UpdateDesc()
{
	RV_ASSERT(m_hCamera);
	 

	int format = FF_PIXEL_FMT_BGR24;

	int dep =8;
	rcbGetParam(m_hCamera, "PixelDepth", &dep);

	int w = 320, h=240;
	rcbGetParam(m_hCamera, "width", &w);
	rcbGetParam(m_hCamera, "height", &h);
	 
	if (dep == 8){
		format = FF_PIXEL_FMT_GRAY8;
	}

	MxVideoDesc* pDesc = (MxVideoDesc*)m_outputPad.GetFormatDescriptor();

	ResetVideoDescEx(pDesc, format, w, h);

	if (m_outputPad.IsConnected()) {
		return m_outputPad.NegotiateDescriptor((MxDescriptor*)pDesc);
	}

	return TRUE;	   
}



//
//void CCamaxSource::SetCaptureParams(HWND hCapWin)
//{
//	CAPTUREPARMS CaptureParms;
//	float FramesPerSec = 15.0;
//
//	HWND hWndC = hCapWin;
//
//	capCaptureGetSetup(hWndC, &CaptureParms, sizeof(CAPTUREPARMS));
//
//	CaptureParms.dwRequestMicroSecPerFrame = (DWORD) (1.0e6 / FramesPerSec);
//
//    CaptureParms.fYield = TRUE;
//	CaptureParms.fCaptureAudio = FALSE;
//	CaptureParms.fAbortLeftMouse = FALSE;
//	CaptureParms.fAbortRightMouse = FALSE;
//
//	CaptureParms.fMCIControl = FALSE;
//
//	CaptureParms.vKeyAbort = 0;
//
//	  
//	capCaptureSetSetup(hWndC, &CaptureParms, sizeof (CAPTUREPARMS)); 
//
//
//}

 

BOOL CCamaxSource::ParseURL(const char* _strUrl)
{
	 
	if(NULL == _strUrl) return FALSE;

	CUtString ms( _strUrl);
	char*  strUrl = ms ;

	m_strOption[0] = '\0';
	m_strModule[0] = '\0';

	if (!ParseModelNameFromURL(_strUrl, m_strModule, sizeof(m_strModule), &m_nType, &m_nSubType)){
		return FALSE;
	}

	char* sub = strstr(strUrl, "?");
	if (NULL== sub) return FALSE;

	char* strTmp =sub +1;

	////相机头
	//char* sub = strstr(strTmp, CS_URL_LEADER);

	//if (NULL== sub) return FALSE;
	//strTmp = sub+strlen(CS_URL_LEADER);

	////相机类型
	//sub = strstr(strTmp, ST_URL_SUB_DELIMIT);
	//if (NULL== sub) return FALSE;
	//sub[0]='\0';

	//int len = 1;
	//if (_stricmp(strTmp, CS_REALCAM) == 0  ){
	//    m_nType = CS_DEFAULT;
	//	m_nSubType = CS_DEFAULT;
	////	len += strlen(CS_REALCAM);
	//}
	//else if (_stricmp(strTmp, CS_FAKECAM) == 0 ){
	//    m_nType = CS_FAKE;
	//	m_nSubType = CS_FAKE;
	////	len += strlen(CS_FAKECAM);
	//} 
	//else if (_stricmp(strTmp, CS_P2PCAM) == 0 ){
	//    m_nType = CS_DEFAULT;
	//	m_nSubType = CS_P2P;
	////	len += strlen(CS_P2PCAM);
	//} 
	//else if (_stricmp(strTmp, CS_IPDESKCAM) == 0 ){
	//    m_nType = CS_FAKE;
	//	m_nSubType = CS_IPDESK;
	////	len += strlen(CS_IPDESKCAM);
	//} 
	//else if (_stricmp(strTmp, CS_DESKCAM) == 0 ){
	//    m_nType = CS_FAKE;
	//	m_nSubType = CS_DESK;
	////	len += strlen(CS_DESKCAM);
	//} 
	//else{
	//    return FALSE;
	//}
	//strTmp = sub+ len;
	// 
	//char buffval[135];

 //   //模块名称
	//sub = strstr(strTmp, "?");
	//if (NULL== sub) return FALSE;
	//sub[0]='\0';

	if(m_nType == CS_DEFAULT){
		
		if (m_nSubType == CS_P2P){
			MX_ASSERT(0);
			//保留
			return FALSE;
		}
		else{
		    goto ON_DEFAULT_PARSER;
		}
	}
	else if (CS_FAKE == m_nType){
		if (CS_IPDESK == m_nSubType){
			goto ON_REMOTE_DESK_PARSER;
		}
		else{
		    goto ON_DEFAULT_PARSER;
		}
	}
	else{
		MX_ASSERT(0);
		return FALSE;
	}
	
	return FALSE;

ON_DEFAULT_PARSER:

	return ParseParamsFromURL(strTmp, &this->m_nCamIndex);

ON_REMOTE_DESK_PARSER:
	TCHAR local[123], remote[123], pwd[64];
	BOOL b = this->ParseParamsFromURL(strTmp, local, sizeof(local), remote, sizeof(remote), pwd, sizeof(pwd));

	if (b){
	    int n = strlen(local) + strlen(remote) + strlen(pwd) +2;
		if (n < sizeof(m_strOption)){
		   strcpy(m_strOption, local);
		   strcat(m_strOption, ",");
		   strcat(m_strOption, remote);
		   strcat(m_strOption, ",");
		   strcat(m_strOption, pwd);
		}
		else{
		   b=FALSE;
		}
	}

	return b;
}

	
BOOL CCamaxSource::ParseModelNameFromURL(const TCHAR* strUrl, TCHAR* strModelName, int strModelNameLen, int* pModelType, int* pSubType)
{
	if(NULL == strUrl) return FALSE;

	CUtString ms(strUrl);

	char* strTmp = ms ;

	//相机头
	char* sub = strstr(strTmp, CS_URL_LEADER);

	if (NULL== sub) return FALSE;
	strTmp = sub+strlen(CS_URL_LEADER);

	//相机类型
	sub = strstr(strTmp, ST_URL_SUB_DELIMIT);
	if (NULL== sub) return FALSE;
	sub[0]='\0';
	 
	int len = 1;
	if (_stricmp(strTmp, CS_REALCAM) == 0  ){
		if (pModelType) *pModelType= CS_DEFAULT;
		if (pSubType) *pSubType= CS_DEFAULT;
		//	len += strlen(CS_REALCAM);
	}
	else if (_stricmp(strTmp, CS_FAKECAM) == 0 ){
		if (pModelType) *pModelType = CS_FAKE;
		if (pSubType) *pSubType=   CS_DEFAULT;
		//	len += strlen(CS_FAKECAM);
	} 
	else{
		MX_ASSERT(0);
	    return FALSE;
	}

	
	strTmp = sub+ len;
	 
	//char buffval[135];

    //模块名称
	sub = strstr(strTmp, "?");
	if (NULL== sub) return FALSE;
	sub[0]='\0';
	 
	//确定子类型
	if (_stricmp(strTmp, CS_P2PCAM) == 0 ){
		if (pSubType) *pSubType=  CS_P2P;
	} 
	else if (_stricmp(strTmp, CS_IPDESKCAM) == 0 ){
		if (pSubType) *pSubType=   CS_IPDESK;
	} 
	else if (_stricmp(strTmp, CS_DESKCAM) == 0 ){		 
		if (pSubType) *pSubType = CS_DESK;
	} 
	//else{
	//	MX_ASSERT(0);
	//    return FALSE;
	//}

	//输出model
	int n =strlen(strTmp);
	if (n <strModelNameLen){
	    if (strModelName){
		    strcpy(strModelName, strTmp);
		}
	}


	return TRUE;

}

BOOL CCamaxSource::ParseParamsFromURL(const TCHAR* _strUrl,  int* pCamIndex)
{
	if (NULL == _strUrl) return FALSE;

	CUtString ms(_strUrl);
	char* strUrl = ms ;

//	char buffval[16];

	if (!ParseField(strUrl, CS_PARAM_INDEX, pCamIndex)){
		return FALSE;
	}
	 
	return TRUE;
}

BOOL CCamaxSource::ParseParamsFromURL(const TCHAR* _strUrl, TCHAR* strLocalIp, int strLocalIpLen, TCHAR* strRemoteIp,  int strRemoteIpLen,  \
		                           TCHAR* strPwd, int nPwdLen)
{
	if (NULL == _strUrl) return FALSE;

	CUtString ms(_strUrl);
	char* strUrl = ms ;

	if (strLocalIp){
	    strLocalIp[0]='\0';
	}
	if (strRemoteIp){
	    strRemoteIp[0]='\0';
	}
	if (strPwd){
	    strPwd[0]='\0';
	}

	return (ParseField(strUrl, CS_LOCAL_IP, strLocalIp, strLocalIpLen )  \
		   && ParseField(strUrl, CS_REMOTE_IP, strRemoteIp, strRemoteIpLen ) \
		   && ParseField(strUrl, CS_PASSWORD, strPwd, nPwdLen ));

}

int   CCamaxSource::GetFrameRate()
{
	if (m_hCamera == NULL)  return 0;

	return rcbGetFrameRate(m_hCamera);
}

//void CCamaxSource::SetOutputFormat(int format, int width, int height)
//{
//	RV_ASSERT(width >0 && height > 0);
//
//	ResetVideoDescEx(&m_videoOutDesc, format,   width,  height);
//
//	ResetVideoDescEx(&m_captureDesc  , format,   width,  height);
//
//}

//MxVideoDesc* CCamaxSource::GetCaptureDesc()
//{
//	return &m_captureDesc;
//}
//
//void CCamaxSource::EnableConvert()
//{
//	if (m_nState != MS_STOPPED ) return  ;
//	m_bNoConvert=FALSE;
//}
//
//void CCamaxSource::DisableConvert()
//{
//	if (m_nState != MS_STOPPED ) return  ;
//	m_bNoConvert=TRUE;
//}
//
//
//
//BOOL CCamaxSource::IsConvertEnabled()
//{
//	return !m_bNoConvert;
//}