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

#ifndef __CAMAX_SOURCE_H_INCLUDED_
#define __CAMAX_SOURCE_H_INCLUDED_

#include "vmo.h"
#include "..\davsdk\includes\source.h"
#include <rvb\rvtypes.h>
#include "..\davsdk\includes\ImageConverter.h"

#define CS_MAX_MODULE_NAME_LEN      31

#define CS_URL_LEADER  ST_URL_CAM_LEADER
#define CS_REALCAM     "real"
#define CS_FAKECAM     "fake"

#define CS_DSHOW_CAM   "DShowCam"
#define CS_IPDESKCAM   "IpdeskCam"
#define CS_P2PCAM     "p2pCam"
#define CS_DESKCAM    "ScreenCam"

#define CS_PARAM_INDEX   "index"
#define CS_LOCAL_IP      "localeIp"
#define CS_REMOTE_IP     "remoteIp"
#define CS_PASSWORD      "password"



#define   CS_DEFAULT  (1<<16)   //默认实体相机, P2P,远程桌面
#define	  CS_FAKE     (1<<17)   //虚拟目录下图像，虚拟图像,本地桌面
#define	  CS_P2P      (CS_DEFAULT | 1) 
#define	  CS_IPDESK   (CS_FAKE | 1)
#define	  CS_DESK     (CS_FAKE | 2)



//支持符合满足camera bridge规范
class VMO_API CCamaxSource :	public CSource
{
public:

	CCamaxSource(void);
	CCamaxSource(const TCHAR* strLocation);
	virtual ~CCamaxSource(void);


	void SetLocation(const TCHAR* strLocation);

	//外部程序不要删除返回的字符串
	const TCHAR* GetLocation(){ return m_strLocation ;};

	// 设备标志头 "://" 视频标志头 ":/" 网络摄像头模块名 "?" 参数 
	//参数顺序不能换
	//实体相机
	// cam://real:dshowcam?index=0
	// cam://p2p:modulename/fdfdsfkgfgfdlgkfdgfhhh?account=admin&password=123456
	
	//虚拟相机(暂未实现)
	// cam://fake:fakecam?source=c:\tmp
	// cam://desk:deskcam?wnd=0x43545454
    // cam://ipdesk:modulename/192.168.100.123?account=admin&password=123456

	virtual M_RESULT Play(const char* strUrl) ;
	virtual void     Pause(BOOL bResume) ;
	virtual void     Stop();
 
	//COutputPad*  GetOutputPad( ){return &m_outputPad;}
	 COutputPad* GetVideoPad(){return &m_outputPad;}

	//virtual MX_MEDIA_TYPE GetMediaType(CPad* pPad) ;
	//virtual BOOL CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;

	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
	
	////当接收方通过PAD接收SAMPLE时,产生该事件。
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode);
 
	//virtual MxDescriptor*      GetFormatDescriptor(CPad* pPad);	
	virtual void               SetMode(CMediaObject::WORK_MODE mode); 
	


	void  SetFlipType(int type);
	BOOL  SetParam(const TCHAR* strName, int nValue);
	void  ShowParamWin();
	BOOL  IsParamWinSupport();

	//上层软件必须释放图像对象
	static RvImage GetStillImage(const TCHAR* strModel, int camIndex);
	//参数: remote desk: local ip, remote ip, password
	static RvImage GetStillImage(const TCHAR* strModel, const TCHAR* strParams);

	//获取可用相机数量，用于实体相机
	//输入绝对路径
	static int   GetDeviceCount(const TCHAR* strModel);
	static BOOL  GetDeviceDescription(const TCHAR* strModel, int index, char* strDescHolder, int nHolderSize);
	 
	
	//MxVideoDesc* GetCaptureDesc();
	//void EnableConvert();
	//void DisableConvert();

	//BOOL IsConvertEnabled();

	static BOOL ParseModelNameFromURL(const TCHAR* strUrl, TCHAR* strModelName, int strModelNameLen, int* pModelType, int* pSubType);
	//remote dest parameters
	static BOOL ParseParamsFromURL(const TCHAR* strUrl, TCHAR* strLocalIp, int strLocalIpLen, TCHAR* strRemoteIp,  int strRemoteIpLen,  \
		                           TCHAR* strPwd, int nPwdLen);

	static BOOL ParseParamsFromURL(const TCHAR* strUrl,  int* pCamIndex);

	int   GetFrameRate();

	//视频输出格式
	//void SetOutputFormat(int format, int width, int height);

public:
	void  HanldeVideoData(void* pDataBuffer,  DWORD dwDataSize);
  
private:
		
	//文件虚拟相加方式
	BOOL  Open(const TCHAR* strModule, const TCHAR* strOptions );
	//实体相机打开方式
	BOOL  Open(const TCHAR* strModule, int  index );
	//窗口拷屏打开方式
	//BOOL  Open(const TCHAR* strModule, HWND hSrcWnd);
	void  Close();

	BOOL  UpdateDesc();
	void  SetDefaults();
	//void  SetCaptureParams(HWND hCapWin);
	//输入: strUrl
	//输出: strModel, strOption, camIndex
	BOOL   ParseURL(const TCHAR* strUrl);


	//CImageConverter m_imageConverter;
	 
	int    m_nType;
	int    m_nSubType;
	int    m_nCamIndex;
	TCHAR  m_strModule[CS_MAX_MODULE_NAME_LEN + 1];
	TCHAR  m_strOption[MAX_PATH + 1];

	TCHAR* m_strLocation;

	HANDLE m_hCamera;

	COutputPad  m_outputPad;
	//MxVideoDesc m_videoOutDesc;
	//MxVideoDesc m_captureDesc;
	//BOOL        m_bNoConvert;
	ULONGLONG     m_nSerial;
	RvSequence m_seqSample;
	CRITICAL_SECTION m_csState;

	CRITICAL_SECTION m_csSample;
	 
};





#endif

