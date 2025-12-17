/*
 * Utf8Encoder.cpp
 *
 *  Created on: 2011-3-19
 *      Author: terry
 */
#include "stdafx.h"

#include "Utf8Encoder.h"


Utf8Encoder::Utf8Encoder()
{
}

Utf8Encoder::~Utf8Encoder()
{
}


const char* Utf8Encoder::gbkToUtf8(const char* text)
{
    return gbkToUtf8(text, strlen(text));
}

const char* Utf8Encoder::gbkToUtf8(const char* text, size_t length)
{
    int unicodeLen = MultiByteToWideChar(CP_ACP, 0, text, (int)length, NULL, 0);
    m_wcharBuffer.resize(unicodeLen, 0);

    unicodeLen = MultiByteToWideChar(CP_ACP, 0, text, (int)length, m_wcharBuffer.data(), unicodeLen);

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, m_wcharBuffer.data(), unicodeLen, NULL, 0, NULL, NULL);
    m_charBuffer.resize(utf8Len + 1, 0);
    utf8Len = WideCharToMultiByte(CP_UTF8, 0, m_wcharBuffer.data(), unicodeLen,
        m_charBuffer.data(), utf8Len, NULL, NULL);
    m_charBuffer[utf8Len] = 0;
    return m_charBuffer.data();
}

const char* Utf8Encoder::gbkToUtf8(const std::string& text)
{
    return gbkToUtf8(text.c_str(), text.size());
}


const char* Utf8Encoder::utf8ToGbk(const char* text)
{
    return utf8ToGbk(text, strlen(text));
}

const char* Utf8Encoder::utf8ToGbk(const char* text, size_t length)
{
    int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, text, (int)length, NULL, 0);
    m_wcharBuffer.resize(unicodeLen, 0);
    unicodeLen = MultiByteToWideChar(CP_UTF8, 0, text, (int)length, m_wcharBuffer.data(), unicodeLen);

    int gbkLen = WideCharToMultiByte(CP_ACP, 0, m_wcharBuffer.data(), unicodeLen, NULL, 0, NULL, NULL);
    m_charBuffer.resize(gbkLen + 1, 0);
    gbkLen = WideCharToMultiByte(CP_ACP, 0, m_wcharBuffer.data(), unicodeLen,
        m_charBuffer.data(), gbkLen, NULL, NULL);
    m_charBuffer[gbkLen] = 0;
    return m_charBuffer.data();
}

const char* Utf8Encoder::utf8ToGbk(const std::string& text)
{
    return utf8ToGbk(text.c_str(), text.size());
}


