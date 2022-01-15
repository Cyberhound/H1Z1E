#pragma once
#include <stdint.h>
#include <string>


enum StringWriteFormat
{
	/*
	This is for strings that are written as null-terminated ONLY.
	For some reason H1Z1 likes to switch that shit.

	By this, I mean this mode allows reading of strings that don't
	have a size. The method to get the size, is exact same as strlen().
	Follow until first '\0' or 0 byte, which is called the null-terminator.
	*/
	UNTIL_NULL_TERM,

	/*
	This is for strings that are written as non-null terminated (or null-terminated, no difference).
	For some reason H1Z1 likes to switch that shit.

	By this, I mean this mode allows reading of strings that have
	the size given right before the string (in a 4 byte uint32_t or __int32).
	It will then use this size and say that is the real string's size. Be careful.
	*/
	GIVEN_SIZE,

	/* Combination of both above. This gives the size, and null-terminating character... Why? */
	GIVEN_SIZE_NULL_TERM,
};


/* This class is responsible for the buffer, it will allocate, resize, and so on. */
class ByteStreamBuffer
{
private:
	uint8_t allocatedInst;

protected:
	uint8_t *buffer;
	size_t bufferSize;
	int idx;

	uint8_t autoBuffer;
	uint8_t bigEndian;

	uint8_t managed;
	uint8_t freeableInst;

public:
	ByteStreamBuffer();
	ByteStreamBuffer(size_t bufferSize, uint8_t bigEndian, uint8_t autoBuffer = 1);
	ByteStreamBuffer(uint8_t *buffer, size_t bufferSize, uint8_t bigEndian, uint8_t autoBuffer = 1);
	~ByteStreamBuffer();

	/* Initialize the ByteStreamBuffer like the constructors do. */
	void SetupBuffer(size_t bufferSize, uint8_t bigEndian, uint8_t autoBuffer = 1)
	{
		this->buffer = bufferSize > 0 ? (uint8_t *)malloc(bufferSize) : nullptr;
		this->bufferSize = bufferSize;
		this->idx = 0;
		this->bigEndian = bigEndian;
		this->autoBuffer = autoBuffer;

		this->allocatedInst = 1;
		this->freeableInst = 1;


		memset(buffer, 0, bufferSize);
	}

	/* Initialize the ByteStreamBuffer like the constructors do. */
	void SetupBuffer(uint8_t *buffer, size_t bufferSize, uint8_t bigEndian, uint8_t autoBuffer = 1)
	{
		this->buffer = buffer;
		this->bufferSize = bufferSize;
		this->idx = 0;
		this->bigEndian = bigEndian;
		this->autoBuffer = autoBuffer;

		this->allocatedInst = 0;
		this->freeableInst = 0;
	}

	/* Initialize the ByteStreamBuffer like the constructors do, but copy the buffer. */
	void SetupBufferCopy(uint8_t *buffer, size_t bufferSize, uint8_t bigEndian, uint8_t autoBuffer = 1)
	{
		if (!buffer || buffer == nullptr)
		{
			/* Invalid buffer, result to normal buffering */
			this->SetupBuffer(bufferSize, bigEndian, autoBuffer);
			return;
		}

		this->buffer = (uint8_t *)malloc(bufferSize);
		memcpy(this->buffer, buffer, bufferSize);

		this->bufferSize = bufferSize;
		this->idx = 0;
		this->bigEndian = bigEndian;
		this->autoBuffer = autoBuffer;

		this->allocatedInst = 1;

		this->managed = 1;
		this->freeableInst = 1;
	}


	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	void SetIdx(int idx) { this->idx = idx; }
	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	void SetIndex(int idx) { SetIdx(idx); }

	/* This returns the internal index. */
	int GetIdx() { return this->idx; }
	/* This returns the internal index. */
	int GetIndex() { return this->idx; }

	/* This will return the amount of bytes written. */
	int GetSize() { return this->idx; }
	/* This will return the size of the buffer, in bytes. */
	int GetBufferSize() { return this->bufferSize; }


	/* Returns the internal buffer (if any) */
	uint8_t *GetBuffer() { return this->buffer; }

	/* Returns if the internal buffer is big endian (it's interpreted differently) */
	uint8_t IsBigEndian() { return this->bigEndian; }

	/* Returns if the internal buffer does auto-buffering (automatically resizes the buffer, as it's being written) */
	uint8_t IsAutoBuffering() { return this->autoBuffer; }


	/* Returns if the internal buffer is managed */
	uint8_t IsManaged() { return this->managed; }

	/* Returns if the internal buffer is managed */
	void SetManaged(uint8_t isManaged) { this->managed = isManaged; }

	/* Returns if the internal buffer is allocated by this buffer  */
	uint8_t IsAllocated() { return this->managed; }

	/* Sets if the internal buffer is allocated by this buffer  */
	void SetAllocated(uint8_t isAllocated) { this->allocatedInst = isAllocated; }

	/* Returns if the internal buffer is allocated and is freeable (by this instance of the buffer) */
	uint8_t IsFreeable() { return this->freeableInst; }

	/* Sets if the allocated buffer is freeable (by this instance of the buffer) */
	void SetFreeable(uint8_t isFreeable) { this->freeableInst = isFreeable; }


	/*
	Resizes the buffer to write to 'bufferSize'.
	This also clears the buffer, so be careful.

	If you want it not to clear the buffer, set
	the argument 'persist' to true.
	*/
	void Resize(size_t bufferSize, bool persist = false);

	/*
	Free's the buffer.
	*/
	void Free();
};


class ByteStreamWriter : public ByteStreamBuffer
{
public:
	ByteStreamWriter();
	/*
	This will setup the internal buffer for usage.
	'bufferSize' can be used as a fixed or starting buffer size,
	meanwhile if you set autoBuffer to 1 it will automatically
	resize and extend the buffer when writing data if it is too small.
	*/
	ByteStreamWriter(size_t bufferSize, uint8_t bigEndian = 0, uint8_t autoBuffer = 1);
	~ByteStreamWriter();

	/* Initialize the ByteStreamWriter like the constructors do. */
	void Initialize(size_t bufferSize, uint8_t bigEndian = 0, uint8_t autoBuffer = 1) { this->SetupBuffer(bufferSize, bigEndian, autoBuffer); }

	/* Initialize the ByteStreamWriter like the constructors do. */
	void Initialize(uint8_t *buffer, size_t bufferSize, uint8_t bigEndian = 0, uint8_t autoBuffer = 1) { this->SetupBufferCopy(buffer, bufferSize, bigEndian, autoBuffer); }


	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIdx;
	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIndex;

	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIdx;
	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIndex;

	/*
	This will return the amount of bytes written.
	*/
	using ByteStreamBuffer::GetSize;
	/*
	This will return the size of the buffer, in bytes.
	*/
	using ByteStreamBuffer::GetBufferSize;


	/* Returns if the internal buffer is big endian (it's interpreted differently) */
	using ByteStreamBuffer::IsBigEndian;

	/* Returns if the internal buffer does auto-buffering (automatically resizes the buffer, as it's being written) */
	using ByteStreamBuffer::IsAutoBuffering;


	/* Returns the buffer */
	using ByteStreamBuffer::GetBuffer;


	using ByteStreamBuffer::SetupBuffer;
	using ByteStreamBuffer::SetupBufferCopy;


	using ByteStreamBuffer::IsAllocated;
	using ByteStreamBuffer::SetAllocated;

	using ByteStreamBuffer::IsManaged;
	using ByteStreamBuffer::SetManaged;

	using ByteStreamBuffer::IsFreeable;
	using ByteStreamBuffer::SetFreeable;

	using ByteStreamBuffer::Free;



	/*
	Resizes the buffer to write to 'bufferSize'.
	This also clears the buffer, so be careful.

	If you want it not to clear the buffer, set
	the argument 'persist' to true.
	*/
	using ByteStreamBuffer::Resize;


	/*
	Appends 'data' to buffer (with length of 'dataSize').

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteData(uint8_t *data, size_t dataSize, int startIndex = -1);



	/*
	Writes a string to the internal buffer, 'str'.

	If you set autoResize (to true), it will automatically resize the buffer for you
	JUST ENOUGH to fit the string.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.

	If you set wideString (to true) it will write 'str' as a wide-string.
	If you set wideStringLE (to true) it will write 'str' as a wide-string little endian.
	*/
	void WriteString(std::string str, StringWriteFormat stringMode = StringWriteFormat::GIVEN_SIZE, bool autoResize = true, int startIndex = -1, bool wideString = false, bool wideStringLE = false);

	/*
	Writes a string to the internal buffer, 'str' as a wide string.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.

	If you set autoResize (to true), it will automatically resize the buffer for you
	JUST ENOUGH to fit the string.

	If you set wideStringLE (to true) it will write 'str' as a wide-string little endian.
	*/
	void WriteWString(std::wstring str, StringWriteFormat stringMode = StringWriteFormat::GIVEN_SIZE, bool autoResize = true, int startIndex = -1);


	/*
	Writes a byte onto the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteByte(uint8_t data, int startIndex = -1);


	/*
	Writes a uint16_t or a "short" onto the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteWord(uint16_t data, int startIndex = -1);


	/*
	Writes a uint32_t or a "__int32" onto the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteDword(uint32_t data, int startIndex = -1);

	/*
	Reads a uint64_t or a "__int64" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteQword(uint64_t data, int startIndex = -1);


	/*
	Writes any value to the internal buffer.

	I suggest using this for:
		- double
		- float
		- any basic struct with no foreign data types [IF YOU KNOW WHAT YOU'RE DOING], though you have to know that reading the struct will give only the struct.
			MUST NOT BE A POINTER. this does only write the bytes of that memory address.
		- other foreign values

	Be careful with this function. It can result to 'undefined byte writing' or,
	where it is not what you expected the memory to be. Pointers are not something you should
	even serialize either, which this will accept any value you give it.


	If you set size, it will use that size instead of sizeof().

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	template<typename Value_T>
	void WriteValue(Value_T data, int size = -1, int startIndex = -1)
	{
		WriteData(&data, size == -1 ? sizeof(data) : size, startIndex);
	}



	/* when you explicitly cast the class to a uint8_t *, this will return the actual buffer */
	explicit operator uint8_t *()
	{
		return this->buffer;
	}

	/* when you explicitly cast the class to a std::string, this will return the actual buffer (within a string) */
	explicit operator std::string()
	{
		return std::string((char *)this->buffer, bufferSize);
	}

	/* when you explicitly cast the class to a char *, this will return the actual buffer */
	explicit operator char *()
	{
		return (char *)this->buffer;
	}
};


class ByteStreamReader : public ByteStreamBuffer
{
public:
	ByteStreamReader();
	ByteStreamReader(uint8_t *buffer, size_t bufferSize, uint8_t bigEndian);
	~ByteStreamReader();

	template<typename buffer_T>
	ByteStreamReader(buffer_T buffer, uint8_t bigEndian)
	{
		return ByteStreamReader((uint8_t *)buffer, sizeof(buffer), bigEndian);
	}

	template<typename buffer_T>
	ByteStreamReader(buffer_T buffer, size_t bufferSize, uint8_t bigEndian)
	{
		return ByteStreamReader((uint8_t *)buffer, bufferSize, bigEndian);
	}

	/* Initialize the ByteStreamReader like the constructors do. */
	void Initialize(uint8_t *buffer, size_t bufferSize, uint8_t bigEndian) { this->SetupBuffer(buffer, bufferSize, bigEndian, 0 /* read-only, no buffer-sizing needed (or used) */); }

	/* Initialize the ByteStreamReader like the constructors do. */
	template<typename buffer_T>
	void Initialize(buffer_T buffer, size_t bufferSize, uint8_t bigEndian) { this->SetupBuffer((uint8_t *)buffer, bufferSize, bigEndian, 0); }

	/* Initialize the ByteStreamReader like the constructors do. */
	template<typename buffer_T>
	void Initialize(buffer_T buffer, uint8_t bigEndian) { this->SetupBuffer((uint8_t *)buffer, sizeof(buffer), bigEndian, 0); }


	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIdx;
	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIndex;

	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIdx;
	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIndex;

	/*
	This will return the amount of bytes written.
	*/
	using ByteStreamBuffer::GetSize;
	/*
	This will return the size of the buffer, in bytes.
	*/
	using ByteStreamBuffer::GetBufferSize;


	/* Returns if the internal buffer is big endian (it's interpreted differently) */
	using ByteStreamBuffer::IsBigEndian;

	/* Returns if the internal buffer does auto-buffering (automatically resizes the buffer, as it's being written) */
	using ByteStreamBuffer::IsAutoBuffering;


	/* Returns the buffer */
	using ByteStreamBuffer::GetBuffer;


	using ByteStreamBuffer::SetupBuffer;
	using ByteStreamBuffer::SetupBufferCopy;


	using ByteStreamBuffer::IsAllocated;
	using ByteStreamBuffer::SetAllocated;

	using ByteStreamBuffer::IsManaged;
	using ByteStreamBuffer::SetManaged;
	
	using ByteStreamBuffer::IsFreeable;
	using ByteStreamBuffer::SetFreeable;

	using ByteStreamBuffer::Free;



	/*
	Gets a pointer to the buffer (and checks for the information being valid).

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	uint8_t *ReadData(size_t dataSize, int startIndex = -1);



	/*
	Reads a string from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.

	If you set wideString (to true) it will read as a wide-string.
	If you set wideStringLE (to true) it will read as a wide-string little endian.
	*/
	std::string ReadString(StringWriteFormat stringMode = StringWriteFormat::GIVEN_SIZE, int startIndex = -1, bool wideString = false, bool wideStringLE = false);

	/*
	Reads a string from the internal buffer, as a wide string.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	std::wstring ReadWString(StringWriteFormat stringMode = StringWriteFormat::GIVEN_SIZE, int startIndex = -1, bool wideStringLE = false);


	/*
	Reads a byte from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	uint8_t ReadByte(int startIndex = -1);


	/*
	Reads a uint16_t or a "short" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	uint16_t ReadWord(int startIndex = -1);


	/*
	Reads a uint32_t or a "__int32" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	uint32_t ReadDword(int startIndex = -1);

	/*
	Reads a uint64_t or a "__int64" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	uint64_t ReadQword(int startIndex = -1);


	/*
	Reads any value from the internal buffer.

	I suggest using this for:
		- double
		- float
		- any basic struct with no foreign data types [IF YOU KNOW WHAT YOU'RE DOING], though you have to know that reading the struct will give only the struct.
			MUST NOT BE A POINTER. this does only write the bytes of that memory address.
		- other foreign values

	Be careful with this function. It can result to 'undefined byte reading' or,
	where it is not what you expected the memory to be. Pointers are not something you should
	even deserialize either, which this will accept any value you give it.


	If you set size, it will use that size instead of sizeof().

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	template<typename Value_T>
	Value_T ReadValue(int size = -1, int startIndex = -1)
	{
		return *(Value_T *)ReadData(size == -1 ? sizeof(Value_T) : size, startIndex);
	}



	/* when you explicitly cast the class to a uint8_t *, this will return the actual buffer */
	explicit operator uint8_t *()
	{
		return this->buffer;
	}

	/* when you explicitly cast the class to a std::string, this will return the actual buffer (within a string) */
	explicit operator std::string()
	{
		return std::string((char *)this->buffer, bufferSize);
	}

	/* when you explicitly cast the class to a char *, this will return the actual buffer */
	explicit operator char *()
	{
		return (char *)this->buffer;
	}
};