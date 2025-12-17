/*
 * Utf8Encoder.h
 *
 *  Created on: 2011-3-19
 *      Author: terry
 */

#ifndef UTF8ENCODER_H_
#define UTF8ENCODER_H_

//#include "BasicType.h"
#include <string>
//#include "DataBuffer.h"

#include <utility>

template < class T >
class  DataBuffer
{
public:
	DataBuffer() :
		m_ptr(),
		m_capacity(),
		m_length()
	{
	}

	~DataBuffer()
	{
		cleanup();
	}

	explicit DataBuffer(size_t capacity) :
		m_ptr(),
		m_capacity(),
		m_length()
	{
		ensure(capacity);
	}

	size_t max_size() const
	{
		return m_capacity;
	}

	size_t capacity() const
	{
		return m_capacity;
	}

	size_t size() const
	{
		return m_length;
	}

	size_t length() const
	{
		return m_length;
	}

	bool empty() const
	{
		return (m_length == 0);
	}

	bool setLength(size_t length)
	{
		if (length >= m_capacity)
		{
			return false;
		}

		m_length = length;
		return true;
	}

	T* getPtr()
	{
		return m_ptr;
	}

	T* data()
	{
		return m_ptr;
	}

	T& at(size_t idx)
	{
		if (idx >= m_capacity)
		{
			throw std::_Xout_of_range("invalid index");
		}
		return m_ptr[idx];
	}

	const T& at(size_t idx) const
	{
		if (idx >= m_capacity)
		{
			throw std::_Xout_of_range("invalid index");
		}

		return m_ptr[idx];
	}

	T& operator [](size_t idx)
	{
		return m_ptr[idx];
	}

	const T& operator [](size_t idx) const
	{
		return m_ptr[idx];
	}

	bool ensure(size_t length)
	{
		if (length <= m_capacity)
		{
			return true;
		}

		try
		{
			T* ptr = new T[length];
			if (m_ptr)
			{
				memcpy(ptr, m_ptr, m_length * sizeof(T));
			}

			std::swap(m_ptr, ptr);
			std::swap(m_capacity, length);

			delete[] ptr;
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool resize(size_t length, const T& t)
	{
		if (!ensure(length))
		{
			return false;
		}

		if (length <= m_length)
		{
			m_length = length;
			return true;
		}

		for (size_t i = length; i < m_capacity; i++)
		{
			m_ptr[i] = t;
		}
		m_length = length;
		return true;
	}

	void clear()
	{
		m_length = 0;
	}

	bool expect(size_t length)
	{
		return ensure(m_length + length);
	}


protected:
	void cleanup()
	{
		delete m_ptr;
		m_ptr = NULL;
		m_length = 0;
		m_capacity = 0;
	}

protected:
	T* m_ptr;
	size_t  m_capacity;
	size_t  m_length;

};

class Utf8Encoder
{
public:
    Utf8Encoder();
    ~Utf8Encoder();

    const char* gbkToUtf8(const char* text);
    const char* gbkToUtf8(const char* text, size_t length);
    const char* gbkToUtf8(const std::string& text);

    const char* utf8ToGbk(const char* text);
    const char* utf8ToGbk(const char* text, size_t length);
    const char* utf8ToGbk(const std::string& text);

private:
    DataBuffer< char >		m_charBuffer;
	DataBuffer< wchar_t >	m_wcharBuffer;

};


#endif /* UTF8ENCODER_H_ */
