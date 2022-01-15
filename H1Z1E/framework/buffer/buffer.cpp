#include "buffer.h"



#define SAFETY_CHECK(f) \
uint8_t incrIndex = 0; \
 \
/* set the starting index */ \
if (startIndex == -1) \
{ \
	startIndex = this->idx; \
	incrIndex = 1; \
	if (startIndex == -1) \
		throw std::exception(f " tried to access buffer with invalid index."); \
}


#define BUFFER_CHECK(f) \
/* check if we have a valid buffer. buffer is always initialized to something, either nullptr or a real buffer. */ \
if (!buffer || !bufferSize) \
	throw std::exception(f " got null buffer or null size."); /* wtf you doing retard xD */ \


/* only different interpretation on big endian */
#define AUTO_ENDIAN(result, size) \
if (this->bigEndian) \
{ \
	uint8_t data[size]; \
	 \
	for (int i = size - 1; i >= 0; i--) \
		data[size - i] = ((uint8_t *)&result)[i]; \
	 \
	memcpy(&result, data, size); \
}

#define AUTO_ENDIAN2(result) \
if (this->bigEndian) result = (result & 0xFF) << 8 | (result & 0xFF00) >> 8;

#define AUTO_ENDIAN4(result) \
if (this->bigEndian) result = _byteswap_ulong(result);

#define AUTO_ENDIAN8(result) \
if (this->bigEndian) result = _byteswap_uint64(result);


#define AUTO_BUFFER(size) \
if (autoBuffer && (size) > this->bufferSize - startIndex) \
	this->Resize(this->bufferSize + (size), true);

#define AUTO_BUFFERF(size, forcecond) \
if ((autoBuffer || forcecond) && (size) > this->bufferSize - startIndex) \
	this->Resize(this->bufferSize + (size), true);



/* byte stream buffer, responsible for the internal buffer. */
ByteStreamBuffer::ByteStreamBuffer() :
	buffer(nullptr),
	bufferSize(0),
	idx(-1),
	bigEndian(0),
	autoBuffer(1),

	allocatedInst(0),

	managed(0),
	freeableInst(0)
{}

ByteStreamBuffer::ByteStreamBuffer(size_t _bufferSize, uint8_t _bigEndian, uint8_t _autoBuffer) :
	buffer(_bufferSize > 0 ? (uint8_t *)malloc(_bufferSize) : nullptr),
	bufferSize(_bufferSize),
	idx(0),
	bigEndian(_bigEndian),
	autoBuffer(_autoBuffer),

	allocatedInst(1),

	managed(0),
	freeableInst(1)
{
	memset(buffer, 0, bufferSize);
}

ByteStreamBuffer::ByteStreamBuffer(uint8_t *_buffer, size_t _bufferSize, uint8_t _bigEndian, uint8_t _autoBuffer) :
	buffer(_buffer),
	bufferSize(_bufferSize),
	idx(0),
	bigEndian(_bigEndian),
	autoBuffer(_autoBuffer),

	allocatedInst(0),

	managed(0),
	freeableInst(0)
{}

ByteStreamBuffer::~ByteStreamBuffer()
{
	if (!allocatedInst || !buffer || !bufferSize || managed)
		return;

	if (buffer && freeableInst)
	{
		free(buffer);
		buffer = nullptr;
	}
}


void ByteStreamBuffer::Resize(size_t bufferSize, bool persist)
{
	if (!this->allocatedInst)
		return;

	if (!persist)
	{
		/* reallocate the buffer, and clear it then set the size */
		free(buffer);
		buffer = (uint8_t *)malloc(bufferSize);
		if (!buffer || buffer == 0)
		{
			/* reallocation failed. */
			bufferSize = 0;
			return;
		}

		memset(buffer, 0, bufferSize);
		this->bufferSize = bufferSize;
		return;
	}

	/* create/store buffers */
	uint8_t *oldBuffer = buffer;
	buffer = (uint8_t *)malloc(bufferSize);
	memset(buffer, 0, bufferSize);

	/* copy the buffer */
	memcpy(buffer, oldBuffer, this->bufferSize);

	/* free the old buffer & set the new buffer size */
	free(oldBuffer);
	this->bufferSize = bufferSize;
}

void ByteStreamBuffer::Free()
{
	if (!this->allocatedInst)
		return;

	if (buffer != nullptr)
	{
		free(buffer);
		buffer = nullptr;
	}
}



/* byte stream writing */
void ByteStreamWriter::WriteData(uint8_t *data, size_t dataSize, int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	AUTO_BUFFER(dataSize);
	BUFFER_CHECK(__FUNCTION__);

	if (startIndex + dataSize > bufferSize)
		throw std::exception((__FUNCTION__ " tried to write too many bytes to buffer (" + std::to_string(dataSize) + " bytes) while there are " + std::to_string(bufferSize - startIndex) + " avaliable bytes.").c_str());

	memcpy(buffer + startIndex, data, dataSize);
	if (incrIndex)
		this->idx = startIndex + dataSize;
}


void ByteStreamWriter::WriteString(std::string str, StringWriteFormat stringMode, bool autoResize, int startIndex, bool wideString, bool wideStringLE)
{
	SAFETY_CHECK(__FUNCTION__);
	AUTO_BUFFERF((wideString ? str.size() * 2 : str.size()) + sizeof(uint32_t), autoResize);
	BUFFER_CHECK(__FUNCTION__);

	if (startIndex + str.size() > bufferSize && !autoResize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	switch (stringMode)
	{
		case GIVEN_SIZE:
		{
			if (wideString)
			{
				/* allocate a temporary buffer for writing the string */
				int str_size = str.size() * 2;
				uint8_t *data = (uint8_t *)malloc(str_size);

				/* convert the string */
				for (size_t i = 0; i < str.size(); i++)
				{
					if (wideStringLE)
					{
						/* little endian wide-string */
						data[0] = 0;
						data[1] = (uint8_t)str[i];
					}
					else
					{
						/* big endian wide-string */
						data[0] = (uint8_t)str[i];
						data[1] = 0;
					}
					data += 2;
				}

				/* put the string size */
				*(uint32_t *)&buffer[startIndex] = str.size();

				/* copy the actual string */
				memcpy(buffer + startIndex + sizeof(uint32_t), data, str_size);
				if (incrIndex)
					this->idx = startIndex + str_size + sizeof(uint32_t);

				/* free the temporary buffer */
				free(data);
				return;
			}


			/* put the string size */
			*(uint32_t *)&buffer[startIndex] = str.size();

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(uint32_t), str.c_str(), str.size());
			if (incrIndex)
				this->idx = startIndex + str.size() + sizeof(uint32_t);
			break;
		}
		case UNTIL_NULL_TERM:
		{
			if (wideString)
			{
				/* allocate a temporary buffer for writing the string + null-terminator */
				int str_size = str.size() * 2 + sizeof(uint16_t);
				uint8_t *data = (uint8_t *)malloc(str_size);

				/* convert the string */
				for (size_t i = 0; i < str.size(); i++)
				{
					if (wideStringLE)
					{
						/* little endian wide-string */
						data[0] = 0;
						data[1] = (uint8_t)str[i];
					}
					else
					{
						/* big endian wide-string */
						data[0] = (uint8_t)str[i];
						data[1] = 0;
					}
					data += 2;
				}

				/* put a null-terminator */
				*(uint16_t *)&data[0] = 0;

				/* copy the actual string */
				memcpy(buffer + startIndex, data, str_size);
				if (incrIndex)
					this->idx = startIndex + str_size;

				/* free the temporary buffer */
				free(data);
				return;
			}


			/* put the null-terminator in the string */
			str.push_back('\0');

			/* write the whole string + null-terminator */
			memcpy(buffer + startIndex, str.c_str(), str.size());
			if (incrIndex)
				this->idx = startIndex + str.size();
			break;
		}
		case GIVEN_SIZE_NULL_TERM:
		{
			if (wideString)
			{
				/* allocate a temporary buffer for writing the string */
				int str_size = str.size() * 2 + 2;
				uint8_t *data = (uint8_t *)malloc(str_size);

				/* convert the string */
				for (size_t i = 0; i < str.size(); i++)
				{
					if (wideStringLE)
					{
						/* little endian wide-string */
						data[0] = 0;
						data[1] = (uint8_t)str[i];
					}
					else
					{
						/* big endian wide-string */
						data[0] = (uint8_t)str[i];
						data[1] = 0;
					}
					data += 2;
				}

				/* put the null-terminator in the string */
				*(uint16_t *)&data = 0;

				/* put the string size */
				*(uint32_t *)&buffer[startIndex] = str.size();

				/* copy the actual string */
				memcpy(buffer + startIndex + sizeof(uint32_t), data, str_size);
				if (incrIndex)
					this->idx = startIndex + str_size + sizeof(uint32_t);

				/* free the temporary buffer */
				free(data);
				return;
			}


			/* put the null-terminator in the string */
			str.push_back('\0');

			/* put the string size - 1, "- 1" for null-terminator that is NOT part of the string */
			*(uint32_t *)&buffer[startIndex] = str.size() - 1;

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(uint32_t), str.c_str(), str.size());
			if (incrIndex)
				this->idx = startIndex + str.size() + sizeof(uint32_t);
			break;
		}
	}
}

void ByteStreamWriter::WriteWString(std::wstring str, StringWriteFormat stringMode, bool autoResize, int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	AUTO_BUFFERF(str.size() * 2 + sizeof(uint32_t), autoResize);
	BUFFER_CHECK(__FUNCTION__);


	if (startIndex + str.size() + sizeof(uint32_t) > bufferSize && !autoResize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	switch (stringMode)
	{
		case GIVEN_SIZE:
		{
			/* put the string size */
			*(uint32_t *)&buffer[startIndex] = str.size();

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(uint32_t), str.c_str(), str.size() * 2);
			if (incrIndex)
				this->idx = startIndex + str.size() * 2 + sizeof(uint32_t);
			break;
		}
		case UNTIL_NULL_TERM:
		{
			/* put the null-terminator in the string */
			str.push_back(L'\0');

			/* write the whole string + null-terminator */
			memcpy(buffer + startIndex, str.c_str(), str.size() * 2);
			if (incrIndex)
				this->idx = startIndex + str.size() * 2;
			break;
		}
		case GIVEN_SIZE_NULL_TERM:
		{
			/* put the null-terminator in the string */
			str.push_back('\0');

			/* put the string size - 1, "- 1" for null-terminator that is NOT part of the string */
			*(uint32_t *)&buffer[startIndex] = str.size() - 1;

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(uint32_t), str.c_str(), str.size() * 2);
			if (incrIndex)
				this->idx = startIndex + str.size() * 2 + sizeof(uint32_t);
			break;
		}
	}
}


void ByteStreamWriter::WriteByte(uint8_t data, int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	AUTO_BUFFER(sizeof(uint8_t));
	BUFFER_CHECK(__FUNCTION__);


	if (startIndex + sizeof(uint8_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	/* write the singular byte xD what a cutie */
	buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(uint8_t);
}


void ByteStreamWriter::WriteWord(uint16_t data, int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	AUTO_BUFFER(sizeof(uint16_t));
	BUFFER_CHECK(__FUNCTION__);


	if (startIndex + sizeof(uint16_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	AUTO_ENDIAN2(data);
	*(uint16_t *)&buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(uint16_t);
}


void ByteStreamWriter::WriteDword(uint32_t data, int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	AUTO_BUFFER(sizeof(uint32_t));
	BUFFER_CHECK(__FUNCTION__);


	if (startIndex + sizeof(uint32_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	AUTO_ENDIAN4(data);
	*(uint32_t *)&buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(uint32_t);
}


void ByteStreamWriter::WriteQword(uint64_t data, int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	AUTO_BUFFER(sizeof(uint64_t));
	BUFFER_CHECK(__FUNCTION__);


	if (startIndex + sizeof(uint64_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	AUTO_ENDIAN8(data);
	*(uint64_t *)&buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(uint64_t);
}



/* byte stream reading */
uint8_t *ByteStreamReader::ReadData(size_t dataSize, int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	BUFFER_CHECK(__FUNCTION__);

	if (startIndex + dataSize > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	if (incrIndex)
		this->idx = startIndex + dataSize;
	return &buffer[startIndex];
}


std::string ByteStreamReader::ReadString(StringWriteFormat stringMode, int startIndex, bool wideString, bool wideStringLE)
{
	uint32_t str_size;

	SAFETY_CHECK(__FUNCTION__);
	BUFFER_CHECK(__FUNCTION__);

	switch (stringMode)
	{
		case GIVEN_SIZE:
		case GIVEN_SIZE_NULL_TERM:
		{
			/* make sure we can read a uint32_t */
			if (startIndex + sizeof(uint32_t) > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

			str_size = ReadDword(startIndex);
			startIndex += 4;

			/* now make sure we can read the string (assuming this uint32_t is the string's size) */
			if (startIndex + str_size > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");
			break;
		}
		case UNTIL_NULL_TERM:
		{
			/* search for the '0' character, aka null-terminator (inclusive for str_size). */
			str_size = 0;
			uint8_t *pBuffer = buffer + startIndex + (wideStringLE ? 1 : 0);
			if (wideString)
			{
				while (*pBuffer && pBuffer <= buffer + bufferSize)
				{
					str_size++;
					pBuffer += 2;
				}
			}
			else
			{
				while (*pBuffer && pBuffer <= buffer + bufferSize)
				{
					str_size++;
					pBuffer++;
				}
			}

			if (str_size == 1)
			{
				/* 'read' the null-terminator */
				this->idx++;
				return "";
			}
			break;
		}
	}


	/* wide string? */
	if (wideString)
	{
		/* allocate a temporary buffer for writing the string */
		uint8_t *data = (uint8_t *)malloc(str_size / 2);

		/* convert the string (from wide to normal) */
		for (size_t i = 0; i < str_size / 2; i++)
		{
			if (wideStringLE)
			{
				/* little endian wide-string */
				*data = (uint8_t)buffer[startIndex + i + 1];
			}
			else
			{
				/* big endian wide-string */
				*data = (uint8_t)buffer[startIndex + i];
			}
			data++;
		}

		std::string result = std::string((char *)data, str_size / 2);
		if (incrIndex)
			this->idx = startIndex + str_size + (stringMode == GIVEN_SIZE_NULL_TERM ? 2 : 0);

		/* free the temporary buffer */
		free(data);
		return result;
	}

	/* read the string, as a normal string. */
	std::string result = std::string((char *)buffer + startIndex, str_size);
	if (incrIndex)
		this->idx = startIndex + str_size + (stringMode == GIVEN_SIZE_NULL_TERM ? 1 : 0);
	return result;
}

std::wstring ByteStreamReader::ReadWString(StringWriteFormat stringMode, int startIndex, bool wideStringLE)
{
	uint32_t str_size;

	SAFETY_CHECK(__FUNCTION__);
	BUFFER_CHECK(__FUNCTION__);

	switch (stringMode)
	{
		case GIVEN_SIZE:
		case GIVEN_SIZE_NULL_TERM:
		{
			/* make sure we can read a uint32_t */
			if (startIndex + sizeof(uint32_t) > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

			str_size = ReadDword(startIndex);
			startIndex += 4;

			/* now make sure we can read the string (assuming this uint32_t is the string's size) */
			if (startIndex + str_size > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");
			break;
		}
		case UNTIL_NULL_TERM:
		{
			/* search for the '0' character, aka null-terminator (inclusive for str_size). */
			str_size = 0;
			uint8_t *pBuffer = buffer + startIndex + (wideStringLE ? 1 : 0);
			while (*pBuffer && pBuffer <= buffer + bufferSize)
			{
				str_size++;
				pBuffer += 2;
			}

			if (str_size == 1)
			{
				/* 'read' the null-terminator */
				this->idx++;
				return L"";
			}
			break;
		}
	}


	std::wstring result = std::wstring((wchar_t *)(buffer + startIndex), str_size / 2);
	if (incrIndex)
		this->idx = startIndex + str_size + (stringMode == GIVEN_SIZE_NULL_TERM ? 1 : 0);
	return result;
}


uint8_t ByteStreamReader::ReadByte(int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	BUFFER_CHECK(__FUNCTION__);

	if (startIndex + sizeof(uint8_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	/* read the singular byte xD what a cutie */
	uint8_t result = buffer[startIndex];
	if (incrIndex)
		this->idx = startIndex + sizeof(uint8_t);
	return result;
}


uint16_t ByteStreamReader::ReadWord(int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	BUFFER_CHECK(__FUNCTION__);

	if (startIndex + sizeof(uint16_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	uint16_t result = *(uint16_t *)&buffer[startIndex];
	AUTO_ENDIAN2(result);
	if (incrIndex)
		this->idx = startIndex + sizeof(uint16_t);
	return result;
}


uint32_t ByteStreamReader::ReadDword(int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	BUFFER_CHECK(__FUNCTION__);

	if (startIndex + sizeof(uint32_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	uint32_t result = *(uint32_t *)&buffer[startIndex];
	AUTO_ENDIAN4(result);
	if (incrIndex)
		this->idx = startIndex + sizeof(uint32_t);
	return result;
}


uint64_t ByteStreamReader::ReadQword(int startIndex)
{
	SAFETY_CHECK(__FUNCTION__);
	BUFFER_CHECK(__FUNCTION__);

	if (startIndex + sizeof(uint64_t) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	uint64_t result = *(uint64_t *)&buffer[startIndex];
	AUTO_ENDIAN8(result);
	if (incrIndex)
		this->idx = startIndex + sizeof(uint64_t);
	return result;
}



/* stream writer constructors */
ByteStreamWriter::ByteStreamWriter() :
	ByteStreamBuffer(0, 0)
{}

ByteStreamWriter::ByteStreamWriter(size_t _bufferSize, uint8_t _bigEndian, uint8_t _autoBuffer) :
	ByteStreamBuffer(_bufferSize, _bigEndian == 1, _autoBuffer == 1)
{}


ByteStreamWriter::~ByteStreamWriter()
{}


/* stream reader constructors */
ByteStreamReader::ByteStreamReader() :
	ByteStreamBuffer()
{}

ByteStreamReader::ByteStreamReader(uint8_t *_buffer, size_t _bufferSize, uint8_t _bigEndian) :
	ByteStreamBuffer(_buffer, _bufferSize, _bigEndian)
{}


ByteStreamReader::~ByteStreamReader()
{
	buffer = nullptr;
	bufferSize = 0;
}