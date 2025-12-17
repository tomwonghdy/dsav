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
#include "Mp3Sink.h"

#include <string>
#include "Utf8Encoder.h"

#include <rvb\dsmsp.h>
#include "avlib.h"

 
#define STREAM_DURATION     (60 * 10) 
   
//MUX RESULT
#define MR_FULL        2
#define MR_NODATA      1
#define MR_NORMAL      0
  
 

static const int MAX_PKT_SIZE = 1024 * 1024;


struct CMp3Sink::Pimp
{
    AVCodecContext*     m_codec;
    SwrContext*		m_swrContext;
    AVAudioFifo*	m_fifo;
    int64_t         m_sampleCount;
    AVFormatContext*   m_fmtCtx;
    AVStream* m_stream;
    AVSampleFormat  m_inSampleFmt;
    uint8_t*        m_pktBuffer;
    AVFrame*  m_frame;
    std::string     m_filename;
    int             m_maxDuration;
    AVPacket*       m_pkt;

    Pimp():
        m_codec(),
        m_swrContext(),
        m_fifo(),
        m_sampleCount(),
        m_fmtCtx(),
        m_inSampleFmt(AV_SAMPLE_FMT_NONE),
        m_pktBuffer(),
        m_maxDuration()
    {
        m_pktBuffer = new uint8_t[MAX_PKT_SIZE];
    }

    ~Pimp()
    {
        Close();

        delete[] m_pktBuffer;
    }

    bool Open(int sampleRate, int channels, int sampleFmt, const char* filename)
    {
        Utf8Encoder utf8Encoder;
        m_filename = utf8Encoder.gbkToUtf8(filename);
        
        m_inSampleFmt = (AVSampleFormat)sampleFmt;
                  
        int rc = avformat_alloc_output_context2(&m_fmtCtx, NULL, "mp3", NULL/*filename*/);
        if (!m_fmtCtx)
        {
            return false;
        }
         
        const AVOutputFormat* pOutFmt = m_fmtCtx->oformat;

        const AVCodec* codec = avcodec_find_encoder_by_name("libmp3lame");

        if (codec == NULL) {
            codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
        }

        if (codec == NULL) {
            return false;
        }

        
        AVCodecContext* c = avcodec_alloc_context3(codec);
        if (!c) {
            /*fprintf(stderr, "Could not alloc an encoding context\n");
            exit(1);*/
            return FALSE;
        }
        
        m_codec = c;
       
        m_stream = avformat_new_stream(m_fmtCtx, NULL); 
        RV_ASSERT(m_stream);

        av_channel_layout_default(&m_codec->ch_layout, channels);

        m_codec->sample_rate = sampleRate;
         
        m_codec->sample_fmt =(AVSampleFormat)codec->sample_fmts[0];

        m_codec->time_base.num = 1;
        m_codec->time_base.den = sampleRate;

         m_pkt = av_packet_alloc();

        //m_codec->flags |= CODEC_FLAG2_LOCAL_HEADER;

        AVDictionary* options = NULL;
        
        rc = avcodec_open2(m_codec, codec, &options);
        if (options)
        {
            av_dict_free(&options);
        }
        
        if (rc < 0)
        {
           
            /*char buf[256];
            av_strerror(rc, buf, 256);
            strcat_s(buf, "\n");
            OutputDebugStringA(buf);*/

            Close();
            return false;
        }

        /* copy the stream parameters to the muxer */
        rc = avcodec_parameters_from_context(m_stream->codecpar, c);
        if (rc < 0) {
            //fprintf(stderr, "Could not copy the stream parameters\n");
            //exit(1);
            Close();
            return FALSE;
        }

       m_frame = av_frame_alloc();
       m_frame->nb_samples = m_codec->frame_size;
        // frame->channels = m_codec->ch_layout.nb_channels;
        av_channel_layout_copy(&m_frame->ch_layout, &m_codec->ch_layout);
        m_frame->sample_rate = m_codec->sample_rate;
        m_frame->format = m_codec->sample_fmt;

        int ret = av_frame_get_buffer(m_frame, 0);
        if (ret < 0) {
            // 错误处理
            av_frame_free(&m_frame);
            return ret;
        }

        //rc = av_samples_alloc(m_frame->data, m_frame->linesize, m_frame->ch_layout.nb_channels, m_frame->nb_samples, m_codec->sample_fmt, 1);
        ////av_frame_get_buffer(frame, 0);
        //if (rc < 0) {
        //    Close();
        //   
        //    return false;
        //}

        if (!OpenFile(m_filename.c_str()))
        {
            Close();
            return false;
        }

        if (m_fmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        return true;
    }

    void Close()
    {
        CloseFile();

        if (m_codec)
        {
            avcodec_close(m_codec);
            avcodec_free_context(&m_codec);

           // av_free(m_codec);
            m_codec = NULL;
        }

        if (m_pkt) {
            av_packet_free(&m_pkt);
            m_pkt = NULL;
        }

        if (m_frame) {
            av_frame_unref(m_frame);
            av_freep(m_frame->data);

            av_frame_free(&m_frame);
            m_frame = NULL;
         }

        if (m_swrContext)
        {
            swr_free(&m_swrContext);
        }

        if (m_fifo)
        {
            av_audio_fifo_free(m_fifo);
            m_fifo = NULL;
        }
    }

    bool IsOpen()
    {
        return (m_fmtCtx != NULL);
    }

    bool Write(BYTE* data, int size, int* pErrCode)
    {
        if (IsReachMaxDuration())
        {
            if (pErrCode)*pErrCode = MR_FULL;
            return false;
        }

        int ret = Resample(data, size);
        RV_ASSERT(ret >= 0);

       
        if (!Encode(m_pkt, pErrCode)) {
           return false ;
        }
       

        return true;
    }

    void SetMaxDuration(int ms)
    {
        m_maxDuration = ms;
    }

    bool IsReachMaxDuration()
    {
        if (m_maxDuration <= 0)
        {
            return false;
        }

        return (m_sampleCount * 1000 / m_codec->sample_rate) >= m_maxDuration;
    }

    int Resample(BYTE* data, int size)
    {
        int ret = 0;
        static int s_count = 0;
        s_count ++;

        int bytesPerSample = av_get_bytes_per_sample(m_inSampleFmt);
        int in_samples = size / bytesPerSample / m_codec->ch_layout.nb_channels;

        if (m_swrContext == NULL)
        {
            
            ret = swr_alloc_set_opts2(&m_swrContext,
                        &m_codec->ch_layout, m_codec->sample_fmt, m_codec->sample_rate,
                        &m_codec->ch_layout, m_inSampleFmt, m_codec->sample_rate,
                        0, NULL);
            RV_ASSERT(ret >= 0);

            ret = swr_init(m_swrContext);
            RV_ASSERT(ret >= 0);
        }

        int64_t delay =  swr_get_delay(m_swrContext, m_codec->sample_rate);

        int64_t out_samples =  av_rescale_rnd(delay + in_samples,
                    m_codec->sample_rate, m_codec->sample_rate, AV_ROUND_UP);


        uint8_t* outData[AV_NUM_DATA_POINTERS] = {0};
        int linesize = 0;
        ret = av_samples_alloc(outData, &linesize, m_codec->ch_layout.nb_channels,(int) out_samples, m_codec->sample_fmt, 1);
        RV_ASSERT(ret >= 0);

        
        ret = swr_convert(m_swrContext, outData, (int)out_samples, (const uint8_t**)&data, in_samples);
        
        if (ret > 0)
        {
            StoreFrame(outData, linesize, ret);
        }

        av_freep(outData);

        return ret;
    }

    void StoreFrame(BYTE** data, int size, int samples)
    {
        if (!m_fifo)
        {
            m_fifo = av_audio_fifo_alloc(m_codec->sample_fmt, m_codec->ch_layout.nb_channels, m_codec->frame_size);
        }

        //int samples = size / (m_codec->bits_per_raw_sample / 8);
       int ret = av_audio_fifo_write(m_fifo, (void**)data, samples);
       RV_ASSERT(ret > 0);
    }

    bool LoadFrame(AVFrame* frame, int nb_samples, int* pErrCode)
    {
        int ret = 0;

        if (!m_fifo)
        {
            if (pErrCode) *pErrCode = -1;
            return false;
        }

        if (av_audio_fifo_size(m_fifo) < nb_samples)
        {
            if (pErrCode) *pErrCode = MR_NODATA;
            return false;
        }

       /* frame->nb_samples*/ ret = av_audio_fifo_read(m_fifo, (void**)frame->data, nb_samples);

        if (ret < 0) {
            if (pErrCode) *pErrCode = ret;
            return false;
        }
        
        return true;
    }

    void CloseFifo()
    {
        if (m_fifo)
        {
            av_audio_fifo_free(m_fifo);
            m_fifo = NULL;
        }
    }

    int GetFifoSize()
    {
        if (!m_fifo)
        {
            return 0;
        }
        return av_audio_fifo_size(m_fifo);
    }

    bool Encode(AVPacket* pkt, int* pErrCode)
    {
        if (av_audio_fifo_size(m_fifo) < m_codec->frame_size)
        {
            if (pErrCode)*pErrCode = MR_NODATA;
            return false;
        }

        int ret = 0;
        //AVFrame* frame = avcodec_alloc_frame();
        ret = av_frame_make_writable(m_frame);
        if (ret < 0) {
            //exit(1);
            if (pErrCode)*pErrCode = ret;
            return false;
        }

        if (!LoadFrame(m_frame, m_frame->nb_samples, pErrCode))
        {
             
            return false;
        }

        m_frame->pts = m_sampleCount;
        m_sampleCount += m_frame->nb_samples;
        

        //int got_packet = 0;
        //int rc = avcodec_encode_audio2(m_codec, pkt, frame, &got_packet);
        ret = write_frame(m_fmtCtx, m_codec, m_stream, m_frame, pkt);
        RV_ASSERT(ret >= 0);

       /* av_frame_unref(frame);
        av_freep(frame->data);*/
       // avcodec_free_frame(&frame);
       // av_frame_free(&frame);

        return (ret >=0);
        //return got_packet > 0;
    }

    bool OpenFile(const char* filename)
    {
        

        
        //out_stream->codec = m_codec;
        
        int rc = avio_open(&m_fmtCtx->pb, filename, AVIO_FLAG_WRITE);
        if (rc < 0)
        {
            CloseFile();
            return false;
        }
        rc = avformat_write_header(m_fmtCtx, NULL);
    //   rc = avformat_write_header(m_fmtCtx, NULL);
       RV_ASSERT(rc >=0);

        return true;
    }

    void CloseFile()
    {
        if (m_fmtCtx)
        {
            int ret = write_frame(m_fmtCtx, m_codec, m_stream, NULL, m_pkt);
            RV_ASSERT(ret >= 0);

            ret = av_write_trailer(m_fmtCtx);
            RV_ASSERT(ret >= 0);
          //  av_write_trailer(m_fmtCtx);

            avio_close(m_fmtCtx->pb);

            avformat_free_context(m_fmtCtx);
            m_fmtCtx = NULL;
        }

		m_sampleCount =0;
    }

    //int WriteToFile(AVPacket* pkt)
    //{
    //    return av_interleaved_write_frame(m_fmtCtx, pkt);
    //}


};



 /*

bool CMp3Sink::open(int sampleRate, int channels, int sampleFmt, const char* filename)
{
    if (sampleRate <= 0)
    {
        return false;
    }
    if (channels <= 0)
    {
        return false;
    }
    if (sampleRate < 0)
    {
        return false;
    }
    if (strlen(filename) <= 0)
    {
        return false;
    }
    return m_pimp->open(sampleRate, channels, sampleFmt, filename);
}*/

//void CMp3Sink::close()
//{
//    m_pimp->close();
//}
//
//bool CMp3Sink::isOpen()
//{
//    return m_pimp->isOpen();
//}

//bool CMp3Sink::write(BYTE* data, int size)
//{
//    if ((data == NULL) || (size <= 0))
//    {
//        return false;
//    }
//
//    return m_pimp->write(data, size);
//}



CMp3Sink::CMp3Sink(void) :m_pimp(new Pimp()) 
{ 
 
	ResetAudioDescEx(&m_audioDesc, AV_DEFAULT_AUDIO_FORMAT, AV_DEFAULT_SAMPLE_RATE,  GetAudioChannelLayout(AV_DEFAULT_CHANNELS) );
    

	m_audioPad.m_pOwner = this;
	m_audioPad.SetCaptureType(CPad::CT_PULL);
    m_audioPad.SetMediaType(MX_MT_AUDIO);
	//m_nDuration = STREAM_DURATION ; 
	 
	
	m_nWorkMode = WM_ACTIVE;
	m_hOutputThread = NULL;
	 
	 
}

CMp3Sink::~CMp3Sink(void)
{
	 delete m_pimp;
}



DWORD WINAPI ThreadMp3SinkProc(  LPVOID lpParameter)
{
	CMp3Sink* pDa = (CMp3Sink* )lpParameter;
	
	pDa->HandleMux();

	 
	
	return 0;
}

void CMp3Sink::HandleMux()
{
	 
	while (m_nState == MS_PLAYING ) 
	{ 
        if (m_nState == MS_STOPPED)
        {
            break;
        }

		int ret = Mux();
		
		if (MR_NODATA == ret)
		{ 
			RaiseStatusReport( MS_NOSAMPLE);

			Sleep(15);

			continue;
		} 
		else if (MR_FULL == ret)
		{
             
			RaiseStatusReport( MS_COMPLETE);
			break;
		}
        else if (ret < 0) {
            break;
        }		    
		RaiseStatusReport( MS_INPROCESS);		 
	} 
  
}


M_RESULT CMp3Sink::Play(const char* strUrl )
{
	 
	if (m_nState == MS_PLAYING) return  M_FAILED;

	if (NULL == strUrl) return M_FILE_NONEXIST;
	 
	RV_ASSERT(m_pimp);

	bool ret = m_pimp->Open(m_audioDesc.sampleRate, m_audioDesc.channelCount, m_audioDesc.sampleFormat, strUrl);

	if (ret ==false) {
		return M_FAILED;
	}

	m_nState =  MS_PLAYING;

	m_hOutputThread =::CreateThread(NULL, 0, ThreadMp3SinkProc, this, 0, 0);

	RV_ASSERT(m_hOutputThread);

	

	return M_OK;

}


void CMp3Sink::Stop()
{
	if (m_nState == MS_STOPPED) return ;

	m_nState = MS_STOPPED;
  
	if (m_hOutputThread){
	     ::WaitForSingleObject(m_hOutputThread, INFINITE);
	      m_hOutputThread = NULL;
	
	}

	m_pimp->Close();

} 
    
 

int CMp3Sink::Mux(void)
{
	
	int errcode=0;
	HANDLE hSample = m_audioPad.Fetch(&errcode);

	if (NULL == hSample){		 
		return MR_NODATA;
	}
	 
	RV_ASSERT(m_pimp->IsOpen());

	void* pSampleData = mxGetSampleData(hSample); 
	UINT size   = mxGetSampleDataSize(hSample); 

	//MxAudioDesc* pAudioDesc =(MxAudioDesc*) mxGetSampleDescriptor(hSample);

	if (false == m_pimp->Write((BYTE*)pSampleData, size, &errcode))
	{
		mxDestroySample(hSample);

        return errcode;
		//return MR_FULL;
	}

	mxDestroySample(hSample);

	return MR_NORMAL;
}

void CMp3Sink::Pause(BOOL bResume)
{	
	;
}

void CMp3Sink::SetDuration(double val)
{
//	if (val > FS_MAX_DURATION) val = FS_MAX_DURATION;
 	//if (val < 60) val = 60;  //最小1分钟
	RV_ASSERT(m_pimp);

	m_pimp->SetMaxDuration(int(val*1000));
	//m_nDuration = val;
	
}


BOOL CMp3Sink::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{
	
	RV_ASSERT(pDescriptor);

	RV_ASSERT (&m_audioPad == pInputPad);
	{
		MxAudioDesc * pDesc =(MxAudioDesc *) pDescriptor;
		RV_ASSERT(pDesc);

		m_audioDesc =*pDesc;
		
		return TRUE;

	} 

	return FALSE;

}

 MxDescriptor* CMp3Sink::GetFormatDescriptor(CPad* pPad)
{
	 if (pPad == &m_audioPad){
		return ( MxDescriptor*)&m_audioDesc;
	 } 

	 return NULL;
 }

 BOOL CMp3Sink::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
 {
	 if (pInputPad == &m_audioPad){
		if (type == MX_MT_AUDIO) return TRUE;
	 } 

	 return FALSE;

 }

/*
  BOOL CMp3Sink::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
 {
	RV_ASSERT(pInputPad == &m_inputPad);

	 return (type != MX_MT_UNKNOWN);
 }
*/
// 
//BOOL CMp3Sink::OnSampleReceived( CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode) 
//{
//
//	RV_ASSERT(hSample && pInputPad);
//
//	if (m_nState != MS_PLAYING) return FALSE;
//
//	if (pInputPad == &m_videoPad){
//		void* pSampleData = mxGetSampleData(hSample); 
//		MxVideoDesc* pVideoDesc =(MxVideoDesc*) mxGetSampleDescriptor(hSample);
//	
//		UINT nDstSize = m_imageConverter.GetDestBufferSize();
//
//		MX_VIDEO_DATA* pData = new MX_VIDEO_DATA;
//		RV_ASSERT(pData);
//		pData->pBuffer = new char[nDstSize];		
//		RV_ASSERT(pData->pBuffer);
//		pData->dataSize = nDstSize;
// 
//		RV_ASSERT(	pVideoDesc->dataSize == m_imageConverter.GetSrcBufferSize());
//
//		BOOL ret = m_imageConverter.ExcuteEx((uint8_t*)pSampleData, pVideoDesc->dataSize, (uint8_t*)pData->pBuffer, nDstSize);
//		RV_ASSERT(ret);
//		
//		if (!AddVideoData(pData, mxGetSamplePTS(hSample))){
//			delete []pData->pBuffer;
//			delete pData;
//		}
//
//	}
//	else if (pInputPad == &m_audioPad){
//
//		void* pSampleData = mxGetSampleData(hSample); 
//		MxAudioDesc* pAudioDesc =(MxAudioDesc*) mxGetSampleDescriptor(hSample);
//
//		UINT newsize=0;
//		int nSampleCount = m_waveConverter.GetDestSampleCount(pAudioDesc->frameCount,  &newsize);
//
//		//output to file.
//		MX_AUDIO_DATA* pData = new MX_AUDIO_DATA;
//		RV_ASSERT(pData);
//		pData->pBuffer = new char[newsize];
//		RV_ASSERT(pData->pBuffer);
//		pData->frameCount = nSampleCount;
//		pData->dataSize = newsize;
//
//		BOOL ret =m_waveConverter.ExcuteEx((uint8_t *)pSampleData, pAudioDesc->dataSize,  pAudioDesc->frameCount , (uint8_t *)pData->pBuffer, newsize, nSampleCount);
// 		RV_ASSERT(ret);
//
//		if (!AddAudioData(pData,  mxGetSamplePTS(hSample))){
//			delete []pData->pBuffer;
//			delete pData;
//		}
//
//		//m_nAudioWrapCount++;
//		//av_freep(&m_nAudioDstData[0]);
//
// 	}
//
//	return FALSE;
//
//}

 //*.avi *.rmvb *.rm *.asf *.divx *.mpg *.mpeg *.mpe *.wmv *.mp4 *.mkv *.vob 
 // *.wav, *.mp3, *.midi, *.ogg, *.wma
       