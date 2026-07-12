#include "CCore.h"
#include "main.h"
#include "CStream.h"

// Input Buffers (Read / Inbound Data)
uint8_t CStream::inputStreamData[STREAM_MAX_SIZE];
size_t CStream::inputStreamSize = 0;
size_t CStream::inputStreamPosition = 0;
bool CStream::inputStreamError = false;

// Output Buffers (Write / Outbound Data)
uint8_t CStream::outputStreamData[STREAM_MAX_SIZE];
size_t CStream::outputStreamEnd = 0;
size_t CStream::outputStreamPosition = 0;
bool CStream::outputStreamError = false;

void CStream::StartWrite(void)
{
	outputStreamPosition = 0;
	outputStreamEnd = 0;
	outputStreamError = false;
}

void CStream::SetWritePosition(int position)
{
	if (position < 0 || (size_t)position > outputStreamEnd)
	{
		std::string warning = "CStream::SetWritePosition: Invalid position " + std::to_string(position) + ", resetting to end " + std::to_string(outputStreamEnd);
		OutputWarning(warning.c_str());
		position = (int)outputStreamEnd;
	}

	outputStreamPosition = (size_t)position;
}

int CStream::GetWritePosition(void)
{
	return (int)outputStreamPosition;
}

int CStream::GetWriteSize(void)
{
	return (int)outputStreamEnd;
}

bool CStream::HasWriteError(void)
{
	return outputStreamError;
}

void CStream::WriteByte(int value)
{
	uint8_t data = (uint8_t)value;
	Write(&data, sizeof(data));
}

void CStream::WriteInt(int value)
{
	Write(&value, sizeof(value));
}

void CStream::WriteFloat(float value)
{
	Write(&value, sizeof(value));
}

void CStream::WriteString(SQChar* value)
{
	if (value == nullptr)
	{
		OutputWarning("CStream::WriteString: Attempted to write NULL string. Writing empty string.");
		uint16_t empty = 0;
		Write(&empty, sizeof(empty));
		return;
	}

#ifdef SQUNICODE
	size_t size = wcslen(value);
#else
	size_t size = strlen(value);
#endif

	uint16_t length = size > UINT16_MAX ? UINT16_MAX : (uint16_t)size;

	if (!CanWrite(sizeof(uint16_t)))
	{
		OutputWarning("CStream::WriteString: No space for string length.");
		outputStreamError = true;
		return;
	}

	size_t available = sizeof(outputStreamData) - outputStreamPosition - sizeof(uint16_t);
	size_t maxChars = available / sizeof(SQChar);

	if (length > maxChars)
	{
		std::string warning = "CStream::WriteString: String truncated. " "Original length: " + std::to_string(length) + " Max: " + std::to_string(maxChars);
		OutputWarning(warning.c_str());
		length = (uint16_t)maxChars;
		outputStreamError = true;
	}

	uint16_t lengthBE = ((length >> 8) & 0xFF) | ((length & 0xFF) << 8);
	Write(&lengthBE, sizeof(lengthBE));
	Write(value, length * sizeof(SQChar));
}

bool CStream::CanWrite(size_t size)
{
	if (outputStreamPosition > sizeof(outputStreamData))
	{
		return false;
	}

	return size <= (sizeof(outputStreamData) - outputStreamPosition);
}

void CStream::Write(const void* value, size_t size)
{
	if (value == nullptr)
	{
		OutputWarning("CStream::Write: NULL source buffer.");
		outputStreamError = true;
		return;
	}

	if (size == 0)
	{
		OutputWarning("CStream::Write: Attempted to write zero bytes.");
		return;
	}

	if (!CanWrite(size))
	{
		std::string warning = "CStream::Write: Output buffer overflow prevented. " "Requested: " + std::to_string(size) + " Remaining: " + std::to_string(sizeof(outputStreamData) - outputStreamPosition);
		OutputWarning(warning.c_str());
		outputStreamError = true;
		return;
	}

	memcpy(&outputStreamData[outputStreamPosition], value, size);
	outputStreamPosition += size;


	if (outputStreamPosition > outputStreamEnd)
	{
		outputStreamEnd = outputStreamPosition;
	}
}

void CStream::SendStream(CPlayer* player)
{
	if (player == nullptr)
	{
		OutputWarning("CStream::SendStream: Attempted to send stream to NULL player.");
		StartWrite();
		return;
	}


	if (outputStreamError)
	{
		OutputWarning("CStream::SendStream: Stream contains write errors. Sending blocked.");
		StartWrite();
		return;
	}

	functions->SendClientScriptData(player->GetID(), outputStreamData, outputStreamEnd);

	StartWrite();
}

<<<<<<< Updated upstream
void CStream::LoadInput(const void* data, size_t size) {
	// Reset the previous state immediately before validating
=======
void CStream::LoadInput(const void* data, size_t size)
{
>>>>>>> Stashed changes
	inputStreamSize = 0;
	inputStreamPosition = 0;
	inputStreamError = true;

	if (data == nullptr)
	{
		OutputWarning("CStream::LoadInput: Received NULL packet buffer.");
		return;
	}

	if (size == 0)
	{
		OutputWarning("CStream::LoadInput: Received empty packet.");
		return;
	}

	if (size > sizeof(inputStreamData))
	{
		std::string warning = "CStream::LoadInput: Packet size exceeds buffer. " "Received: " + std::to_string(size) + " Maximum: " + std::to_string(sizeof(inputStreamData));
		OutputWarning(warning.c_str());
		return;
	}

	memcpy(inputStreamData, data, size);
	inputStreamSize = size;
	inputStreamPosition = 0;
	inputStreamError = false;
}

void CStream::SetReadPosition(int position)
{
	if (position < 0)
	{
		OutputWarning("CStream::SetReadPosition: Negative position received.");
		inputStreamPosition = inputStreamSize;
		inputStreamError = true;
		return;
	}

	if ((size_t)position > inputStreamSize)
	{
		std::string warning = "CStream::SetReadPosition: Position outside stream. " "Requested: " + std::to_string(position) + " Size: " + std::to_string(inputStreamSize);
		OutputWarning(warning.c_str());
		inputStreamPosition = inputStreamSize;
		inputStreamError = true;
		return;
	}

	inputStreamPosition = (size_t)position;
}

int CStream::GetReadPosition(void)
{
	return (int)inputStreamPosition;
}

int CStream::GetReadSize(void)
{
	return (int)inputStreamSize;
}

bool CStream::HasReadError(void)
{
	return inputStreamError;
}

int CStream::ReadByte(void)
{
	if (inputStreamError)
	{
		OutputWarning("CStream::ReadByte: Stream already corrupted.");
		return 0;
	}

	if (inputStreamPosition > inputStreamSize - sizeof(uint8_t))
	{
		std::string warning = "CStream::ReadByte: Out of bounds read. " "Position: " + std::to_string(inputStreamPosition) + " Size: " + std::to_string(inputStreamSize);
		OutputWarning(warning.c_str());
		inputStreamError = true;
		return 0;
	}

	return inputStreamData[inputStreamPosition++];
}

int CStream::ReadInt(void)
{
	if (inputStreamError)
	{
		OutputWarning("CStream::ReadInt: Stream already corrupted.");
		return 0;
	}

	if (inputStreamPosition > inputStreamSize - sizeof(int))
	{
		std::string warning = "CStream::ReadInt: Invalid integer read. " "Position: " + std::to_string(inputStreamPosition) + " Size: " + std::to_string(inputStreamSize);
		OutputWarning(warning.c_str());
		inputStreamError = true;
		return 0;
	}

	int result = 0;
	memcpy(&result, &inputStreamData[inputStreamPosition], sizeof(int));
	inputStreamPosition += sizeof(int);
	return result;
}


float CStream::ReadFloat(void)
{
	if (inputStreamError)
	{
		OutputWarning( "CStream::ReadFloat: Stream already corrupted.");
		return 0.0f;
	}

	if (inputStreamPosition > inputStreamSize - sizeof(float))
	{
		std::string warning = "CStream::ReadFloat: Invalid float read. " "Position: " + std::to_string(inputStreamPosition) + " Size: " + std::to_string(inputStreamSize);
		OutputWarning(warning.c_str());
		inputStreamError = true;
		return 0.0f;
	}

	float result = 0.0f;
	memcpy(&result, &inputStreamData[inputStreamPosition], sizeof(float));
	inputStreamPosition += sizeof(float);
	return result;
}

uint16_t CStream::ReadBEInt16(void)
{
	if (inputStreamError)
	{
		OutputWarning("CStream::ReadBEInt16: Stream already corrupted.");
		return 0;
	}

	if (inputStreamPosition > inputStreamSize - sizeof(uint16_t))
	{
		std::string warning = "CStream::ReadBEInt16: Invalid uint16 read. " "Position: " + std::to_string(inputStreamPosition) + " Size: " + std::to_string(inputStreamSize);
		OutputWarning(warning.c_str());
		inputStreamError = true;
		return 0;
	}

	uint16_t result = 0;
	memcpy(&result, &inputStreamData[inputStreamPosition], sizeof(uint16_t));
	inputStreamPosition += sizeof(uint16_t);
	return ((result >> 8) & 0xFF) | ((result & 0xFF) << 8);
}

SQChar* CStream::ReadString(void)
{
	static SQChar result[4096];
	result[0] = _SC('\0');

	if (inputStreamError)
	{
		OutputWarning("CStream::ReadString: Stream already corrupted.");
		return result;
	}

	uint16_t length = ReadBEInt16();

	if (inputStreamError)
	{
		OutputWarning("CStream::ReadString: Failed reading string length.");
		return result;
	}

	size_t bytesNeeded = (size_t)length * sizeof(SQChar);

	// Check if packet contains enough data for declared string size
	if (inputStreamPosition > inputStreamSize || bytesNeeded > inputStreamSize - inputStreamPosition)
	{
		std::string warning = "CStream::ReadString: Invalid string size. " "Length: " + std::to_string(length) + " Bytes required: " + std::to_string(bytesNeeded) + " Remaining: " + std::to_string(inputStreamSize - inputStreamPosition);
		OutputWarning(warning.c_str());
		inputStreamError = true;
		return result;
	}

	size_t maxChars = (sizeof(result) / sizeof(SQChar)) - 1;
	size_t copyLength = length;

	if (copyLength > maxChars)
	{
		std::string warning = "CStream::ReadString: String truncated. " "Received chars: " + std::to_string(copyLength) + " Maximum: " + std::to_string(maxChars);
		OutputWarning(warning.c_str());
		copyLength = maxChars;
	}

	if (copyLength > 0)
	{
		memcpy(result, &inputStreamData[inputStreamPosition], copyLength * sizeof(SQChar));
	}

	result[copyLength] = _SC('\0');
	inputStreamPosition += bytesNeeded;
	return result;
}

void CStream::RegisterStream(void)
{
	Class<CStream> c(v, "Stream");
	c
		.StaticFunc(_SC("StartWrite"), &CStream::StartWrite)
		.StaticFunc(_SC("SetWritePosition"), &CStream::SetWritePosition)
		.StaticFunc(_SC("GetWritePosition"), &CStream::GetWritePosition)
		.StaticFunc(_SC("GetWriteSize"), &CStream::GetWriteSize)
		.StaticFunc(_SC("HasWriteError"), &CStream::HasWriteError)

		.StaticFunc(_SC("WriteByte"), &CStream::WriteByte)
		.StaticFunc(_SC("WriteInt"), &CStream::WriteInt)
		.StaticFunc(_SC("WriteFloat"), &CStream::WriteFloat)
		.StaticFunc(_SC("WriteString"), &CStream::WriteString)

		.StaticFunc(_SC("SendStream"), &CStream::SendStream)

		.StaticFunc(_SC("SetReadPosition"), &CStream::SetReadPosition)
		.StaticFunc(_SC("GetReadPosition"), &CStream::GetReadPosition)
		.StaticFunc(_SC("GetReadSize"), &CStream::GetReadSize)
		.StaticFunc(_SC("HasReadError"), &CStream::HasReadError)

		.StaticFunc(_SC("ReadByte"), &CStream::ReadByte)
		.StaticFunc(_SC("ReadInt"), &CStream::ReadInt)
		.StaticFunc(_SC("ReadFloat"), &CStream::ReadFloat)
		.StaticFunc(_SC("ReadString"), &CStream::ReadString);


	RootTable(v).Bind(_SC("Stream"), c);
}
