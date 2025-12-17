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
#include "Diagnose.h"


void PrintToOutput(int d)
{
#ifdef _DEBUG
	char buff[123];
	sprintf_s(buff, "%d", d);

	OutputDebugString(buff);
#endif

}

void PrintToOutput(const char* strFormat, int d)
{
#ifdef _DEBUG
	char buff[1203];
	sprintf_s(buff, strFormat, d);
	OutputDebugString(buff);
#endif
}

void PrintToOutput(const char* strFormat, const char* str)
{
#ifdef _DEBUG
	char buff[1203];
	sprintf_s(buff, strFormat, str);
	OutputDebugString(buff);
#endif
}

void PrintToOutput(const char * str ) 
{
#ifdef _DEBUG
	OutputDebugString(str);
#endif
}

void PrintToOutput(double n) 
{
#ifdef _DEBUG
	char buff[123];
	sprintf_s(buff, "%f", n);

	OutputDebugString(buff);
#endif 
}

 


int validate_video_parameters(AVCodecContext* codec_ctx, AVFrame* frame) {
	int has_errors = 0;

	char buff[1204];

	PrintToOutput("--- Video Parameter Validation ---\n");

	// 检查像素格式
	//PrintToOutput("Pixel format: frame=%s, codec=%s\n",
	//	av_get_pix_fmt_name((AVPixelFormat)frame->format),
	//	av_get_pix_fmt_name(codec_ctx->pix_fmt));

	if (frame->format != codec_ctx->pix_fmt) {
		fprintf(stderr, "ERROR: Pixel format mismatch!\n");
		has_errors = 1;
	}

	// 检查分辨率
	//PrintToOutput("Resolution: frame=%dx%d, codec=%dx%d\n",
	//	frame->width, frame->height,
	//	codec_ctx->width, codec_ctx->height);

	if (frame->width != codec_ctx->width || frame->height != codec_ctx->height) {
		fprintf(stderr, "ERROR: Resolution mismatch!\n");
		has_errors = 1;
	}

	// 检查色彩空间和范围
	//PrintToOutput("Color space: frame=%s, codec=%s\n",
	//	av_color_space_name(frame->colorspace),
	//	av_color_space_name(codec_ctx->colorspace));

	//PrintToOutput("Color range: frame=%s, codec=%s\n",
	//	av_color_range_name(frame->color_range),
	//	av_color_range_name(codec_ctx->color_range));

	// 检查数据指针和行大小
	for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
		if (frame->data[i]) {
			sprintf_s(buff, "Data pointer[%d]: %p, linesize: %d\n",
				       i, frame->data[i], frame->linesize[i]);
			PrintToOutput(buff);

			// 检查行大小对齐
			if (frame->linesize[i] > 0 && frame->linesize[i] < frame->width) {
				sprintf_s(buff, "WARNING: Linesize[%d] %d may be too small for width %d\n",
					i, frame->linesize[i], frame->width);
				 
				PrintToOutput(buff);
			}
		}
	}

	return has_errors;
}

int validate_audio_parameters(AVCodecContext* codec_ctx, AVFrame* frame) {
	int has_errors = 0;
	char buff[1204];

	PrintToOutput("--- Audio Parameter Validation ---\n");

	// 检查样本格式
	sprintf_s(buff, "Sample format: frame=%s, codec=%s\n",
		av_get_sample_fmt_name((AVSampleFormat)frame->format),
		av_get_sample_fmt_name(codec_ctx->sample_fmt));
	PrintToOutput(buff);

	if (frame->format != codec_ctx->sample_fmt) {
		fprintf(stderr, "ERROR: Sample format mismatch!\n");
		has_errors = 1;
	}

	// 检查采样率
	sprintf_s(buff, "Sample rate: frame=%d, codec=%d\n",
		frame->sample_rate, codec_ctx->sample_rate);
	
	PrintToOutput(buff);
 

	if (frame->sample_rate != codec_ctx->sample_rate) {
		fprintf(stderr, "ERROR: Sample rate mismatch!\n");
		has_errors = 1;
	}

	// 检查声道布局
	char frame_ch_layout[64];
	char codec_ch_layout[64];

	av_channel_layout_describe(&frame->ch_layout, frame_ch_layout, sizeof(frame_ch_layout));
	av_channel_layout_describe(&codec_ctx->ch_layout, codec_ch_layout, sizeof(codec_ch_layout));

	sprintf_s(buff, "Channel layout: frame=%s, codec=%s\n", frame_ch_layout, codec_ch_layout);
	PrintToOutput(buff);

	sprintf_s(buff, "Channels: frame=%d, codec=%d\n",
		frame->ch_layout.nb_channels, codec_ctx->ch_layout.nb_channels);
	PrintToOutput(buff);
  

	if (av_channel_layout_compare(&frame->ch_layout, &codec_ctx->ch_layout) != 0) {
		PrintToOutput(  "ERROR: Channel layout mismatch!\n");
		has_errors = 1;
	}

	// 检查样本数
	sprintf_s(buff, "Samples per frame: frame=%d, codec_frame_size=%d\n",
		frame->nb_samples, codec_ctx->frame_size);
	PrintToOutput(buff); 

	if (codec_ctx->frame_size > 0 && frame->nb_samples != codec_ctx->frame_size) {
		sprintf_s(buff, "ERROR: Frame size mismatch! Expected %d, got %d\n",
			codec_ctx->frame_size, frame->nb_samples);
		PrintToOutput(buff);
	 

		// 特定编码器检查
		if (codec_ctx->codec_id == AV_CODEC_ID_AAC) {
			//fprintf(stderr, "AAC requires exact frame size of %d samples\n", codec_ctx->frame_size);
			sprintf_s(buff, "AAC requires exact frame size of %d samples\n", codec_ctx->frame_size);
			PrintToOutput(buff);
		}
		else if (codec_ctx->codec_id == AV_CODEC_ID_MP3) {
			PrintToOutput("MP3 typically uses 1152 samples per frame\n");
			//fprintf(stderr, );
		}
		has_errors = 1;
	}

	// 检查数据布局（平面 vs 打包）
	int frame_is_planar = av_sample_fmt_is_planar((AVSampleFormat)frame->format);
	PrintToOutput("Sample format is planar: %s\n", frame_is_planar ? "YES" : "NO");

	// 检查数据指针
	for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
		if (frame->data[i]) {
			sprintf_s(buff, "Data pointer[%d]: %p, linesize: %d\n", 
				i, frame->data[i], frame->linesize[i]);
			PrintToOutput(buff);
			 
		}
	}

	// 验证数据对齐
	if (frame_is_planar) {
		for (int ch = 0; ch < frame->ch_layout.nb_channels; ch++) {
			if (!frame->data[ch]) {
				fprintf(stderr, "ERROR: Planar format but data[%d] is NULL\n", ch);
				has_errors = 1;
			}
		}
	}
	else {
		if (!frame->data[0]) {
			fprintf(stderr, "ERROR: Packed format but data[0] is NULL\n");
			has_errors = 1;
		}
	}

	return has_errors;
}
void validate_frame_parameters(AVCodecContext* codec_ctx, AVFrame* frame) {
	int has_errors = 0;

	// 检查帧数据指针
	if (!frame->data[0]) {
		fprintf(stderr, "ERROR: Frame data pointer is NULL\n");
		has_errors = 1;
	}

	// 根据编解码器类型进行特定检查
	switch (codec_ctx->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		has_errors |= validate_audio_parameters(codec_ctx, frame);
		break;
	case AVMEDIA_TYPE_VIDEO:
		has_errors |= validate_video_parameters(codec_ctx, frame);
		break;
	default:
		fprintf(stderr, "WARNING: Unknown codec type: %d\n", codec_ctx->codec_type);
		break;
	}

	// 检查时间戳
	if (frame->pts == AV_NOPTS_VALUE) {
		PrintToOutput("WARNING: Frame PTS is not set (AV_NOPTS_VALUE)\n");
	}
	else {
		//PrintToOutput("Frame PTS: %"PRId64"\n", frame->pts);
		;
	}

	if (!has_errors) {
		PrintToOutput("Frame parameters appear valid\n");
	}
}
void validate_parameters_before_send(AVCodecContext* codec_ctx, AVFrame* frame)
{
	if (!codec_ctx) {
		//PrintToOutput(stderr, "ERROR: Codec context is NULL\n");
		return;
	}

	PrintToOutput("\n=== AVCodec Parameter Validation ===\n");

	// 检查编解码器基本信息
	if (!codec_ctx->codec) {
		fprintf(stderr, "ERROR: No codec associated with context\n");
		return;
	}

	//PrintToOutput("Codec: %s (%s)\n", codec_ctx->codec->name, codec_ctx->codec->long_name);
	PrintToOutput("Codec type: %s\n", av_get_media_type_string(codec_ctx->codec_type));

	if (!avcodec_is_open(codec_ctx)) {
		fprintf(stderr, "ERROR: Codec context is not opened\n");
		return;
	}
	PrintToOutput("Codec is opened: YES\n");

	if (frame) {
		PrintToOutput("Frame provided: YES\n");
		validate_frame_parameters(codec_ctx, frame);
	}
	else {
		PrintToOutput("Frame: NULL (flush packet)\n");
	}

	PrintToOutput("====================================\n\n");
}


void diagnose_audio_issues(const AVFrame* frame)
{
	PrintToOutput("音频帧诊断:\n");
	PrintToOutput("  样本数: %d\n", frame->nb_samples);
	PrintToOutput("  采样率: %d\n", frame->sample_rate);
	PrintToOutput("  声道数: %d\n", frame->ch_layout.nb_channels);
	PrintToOutput("  采样格式: %s\n", av_get_sample_fmt_name((AVSampleFormat)frame->format));
	PrintToOutput("  平面格式: %s\n", av_sample_fmt_is_planar((AVSampleFormat)frame->format) ? "是" : "否");

	// 检查是否有静音或异常数据
	if (frame->format == AV_SAMPLE_FMT_S16P || frame->format == AV_SAMPLE_FMT_S16) {
		int16_t* samples = (int16_t*)frame->data[0];
		int silent = 1;
		for (int i = 0; i < frame->nb_samples * 2; i++) {
			if (samples[i] != 0) {
				silent = 0;
				break;
			}
		}
		if (silent) {
			PrintToOutput("  警告: 检测到静音帧\n");
		}
	}
}

int validate_ac3_packet(const AVPacket* pkt) 
{
	if (!pkt || !pkt->data || pkt->size <= 0) {
		return -1;
	}

	// AC3 帧应该至少有 7 字节的头部
	if (pkt->size < 7) {
		printf("AC3 错误: 数据包太小，无法包含有效头部\n");
		return -1;
	}

	// 检查 AC3 同步字 (0x0B 0x77)
	if (pkt->data[0] != 0x0B || pkt->data[1] != 0x77) {
		printf("AC3 错误: 未找到同步字 (期望 0x0B77, 得到 0x%02X%02X)\n",
			pkt->data[0], pkt->data[1]);
		return -1;
	}

	// 解析 AC3 头部获取帧大小
	unsigned int frame_size = ((pkt->data[2] & 0x03) << 11) |
		(pkt->data[3] << 3) |
		((pkt->data[4] & 0xE0) >> 5);

	// 检查帧大小是否合理
	if (frame_size < 7 || frame_size > 3840) {
		printf("AC3 错误: 无效的帧大小: %u\n", frame_size);
		return -1;
	}

	// 检查数据包大小是否与帧大小匹配
	if (pkt->size != frame_size * 2) { // AC3 帧大小以 16-bit 字为单位
		printf("AC3 警告: 数据包大小 (%d) 与帧大小 (%d) 不匹配\n",
			pkt->size, frame_size * 2);
	}

	printf("AC3 数据包验证通过:\n");
	printf("  - 同步字: 0x%02X%02X\n", pkt->data[0], pkt->data[1]);
	printf("  - 帧大小: %u 字 (%u 字节)\n", frame_size, frame_size * 2);
	printf("  - 采样率代码: %u\n", (pkt->data[4] & 0x3C) >> 2);

	return 0;
}

#define AC3_SYNC_WORD 0x0B77

int validate_ac3_frame(const uint8_t* data, int size) {
	if (size < 7) {
		printf("帧太小，无法验证\n");
		return -1;
	}

	// 解析 AC3 帧头
	unsigned int frame_size = ((data[2] & 0x03) << 11) |
		(data[3] << 3) |
		((data[4] & 0xE0) >> 5);

	unsigned int sample_rate_code = (data[4] & 0x3C) >> 2;
	unsigned int bitstream_id = (data[5] & 0xF8) >> 3;

	printf("AC3 帧信息:\n");
	printf("  - 帧大小: %u 字 (%u 字节)\n", frame_size, frame_size * 2);
	printf("  - 采样率代码: %u\n", sample_rate_code);
	printf("  - 比特流ID: %u\n", bitstream_id);

	// 验证合理性
	if (frame_size < 7 || frame_size > 3840) {
		printf("无效的帧大小: %u\n", frame_size);
		return -1;
	}

	if (bitstream_id > 16) {
		printf("无效的比特流ID: %u\n", bitstream_id);
		return -1;
	}

	int expected_size = frame_size * 2;
	if (size < expected_size) {
		printf("数据不足: 需要 %d 字节，只有 %d 字节\n", expected_size, size);
		return -1;
	}

	printf("AC3 帧验证通过\n");
	return 0;
}

int find_and_validate_ac3_sync(const uint8_t* data, int size) {
	if (!data || size < 2) {
		return -1;
	}

	// 查找 AC3 同步字
	for (int i = 0; i <= size - 2; i++) {
		uint16_t sync_word = (data[i] << 8) | data[i + 1];
		if (sync_word == AC3_SYNC_WORD) {
			printf("在偏移 %d 找到 AC3 同步字 0x%04X\n", i, sync_word);

			// 验证帧结构
			if (validate_ac3_frame(data + i, size - i) == 0) {
				return i; // 返回同步字位置
			}
		}
	}

	printf("在 %d 字节数据中未找到有效的 AC3 同步字\n", size);
	return -1;
}
int fix_ac3_packet(AVPacket* pkt) {
	if (!pkt || !pkt->data || pkt->size <= 0) {
		return -1;
	}

	// 检查是否已经有同步字
	if (pkt->size >= 2) {
		uint16_t sync_word = (pkt->data[0] << 8) | pkt->data[1];
		if (sync_word == AC3_SYNC_WORD) {
			//printf("数据包已包含正确的同步字\n");
			return 0; // 已经正确
		}
	}

	// 在数据中查找同步字
	int sync_offset = find_and_validate_ac3_sync(pkt->data, pkt->size);
	if (sync_offset > 0) {
		//printf("发现同步字在偏移 %d，修复数据包...\n", sync_offset);

		// 创建新数据包，从同步字开始
		AVPacket new_pkt = { 0 };
		av_new_packet(&new_pkt, pkt->size - sync_offset);
		memcpy(new_pkt.data, pkt->data + sync_offset, pkt->size - sync_offset);

		// 复制其他字段
		new_pkt.pts = pkt->pts;
		new_pkt.dts = pkt->dts;
		new_pkt.duration = pkt->duration;
		new_pkt.stream_index = pkt->stream_index;
		new_pkt.flags = pkt->flags;
		new_pkt.pos = pkt->pos;

		// 替换原数据包
		av_packet_unref(pkt);
		*pkt = new_pkt;

		//printf("修复完成，新数据包大小: %d\n", pkt->size);
		return 0;
	}

	//printf("无法修复数据包，未找到有效的同步字\n");
	return -1;
}

int try_alternative_ac3_fixes(AVPacket* pkt) {
	printf("尝试替代修复方法...\n");

	// 方法1: 检查是否为 E-AC3
	if (pkt->size >= 2) {
		uint16_t sync_word = (pkt->data[0] << 8) | pkt->data[1];
		if (sync_word == 0x770B) { // 字节顺序问题？
			printf("检测到可能的字节顺序问题，尝试交换\n");
			// 交换字节顺序
			for (int i = 0; i < pkt->size - 1; i += 2) {
				uint8_t temp = pkt->data[i];
				pkt->data[i] = pkt->data[i + 1];
				pkt->data[i + 1] = temp;
			}
			return 0;
		}
	}

	// 方法2: 手动添加同步字（危险，仅作为最后手段）
	if (pkt->size > 7 && pkt->size < 10000) {
		printf("尝试手动分析帧结构...\n");

		// 检查数据模式是否符合 AC3
		//if (analyze_ac3_like_pattern(pkt->data, pkt->size) == 0) {
		//	printf("数据模式类似 AC3，但缺少同步字\n");
		//	 这里可以根据具体情况决定是否添加同步字
		//}
		return -1;
	}

	return -1;
}

int decode_ac3_with_sync_fix(AVCodecContext* codec_ctx, AVPacket* pkt) 
{
	int ret =0;

	//if (!pkt) {
	//	// NULL 包用于刷新
	//	return avcodec_send_packet(codec_ctx, NULL);
	//}

	// 调试信息
	//printf("原始数据包 - 大小: %d, 前4字节: ", pkt->size);
	//if (pkt->size >= 4) {
	//	for (int i = 0; i < 4; i++) {
	//		printf("%02X ", pkt->data[i]);
	//	}
	//}
	//printf("\n");

	// 检查并修复同步字
	if (pkt->data && pkt->size > 0) {
		if (fix_ac3_packet(pkt) < 0) {
			//printf("AC3 同步字修复失败\n");

			// 尝试其他修复方法
			if (try_alternative_ac3_fixes(pkt) < 0) {
				return AVERROR_INVALIDDATA;
			}
		}
	}

	//// 发送修复后的数据包
	//ret = avcodec_send_packet(codec_ctx, pkt);

	//if (ret == AVERROR_INVALIDDATA) {
	//	printf("解码器报告无效数据，可能仍有同步问题\n");
	//	// 可以尝试更激进的修复方法
	//}

	return ret;
}