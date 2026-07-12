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

void CStream::StartWrite(void) {
	outputStreamPosition = 0;
	outputStreamEnd = 0;
	outputStreamError = false;
}

void CStream::SetWritePosition(int position) {
	if (position < 0 || (size_t)position > outputStreamEnd) {
		position = (int)outputStreamEnd;
	}
	outputStreamPosition = (size_t)position;
}

int CStream::GetWritePosition(void) {
	return (int)outputStreamPosition;
}

int CStream::GetWriteSize(void) {
	return (int)outputStreamEnd;
}

bool CStream::HasWriteError(void) {
	return outputStreamError;
}

void CStream::WriteByte(int value) {
	uint8_t data = (uint8_t)value;
	Write(&data, sizeof(data));
}

void CStream::WriteInt(int value) {
	Write(&value, sizeof(value));
}

void CStream::WriteFloat(float value) {
	Write(&value, sizeof(value));
}

void CStream::WriteString(SQChar* value) {
	if (value == nullptr) {
		return;
	}

	// Dynamic length check depending on whether SQChar is mapped as wchar_t or standard char
#ifdef SQUNICODE
	size_t size = wcslen(value);
#else
	size_t size = strlen(value);
#endif

	uint16_t length = (size > UINT16_MAX) ? UINT16_MAX : static_cast<uint16_t>(size);

	if (CanWrite(sizeof(length))) {
		if (!CanWrite(length * sizeof(SQChar))) {
			length = (sizeof(outputStreamData) - outputStreamPosition) / sizeof(SQChar);
			outputStreamError = true;
		}

		uint16_t lengthBE = ((length >> 8) & 0xFF) | ((length & 0xFF) << 8);

		Write(&lengthBE, sizeof(lengthBE));
		Write(value, length * sizeof(SQChar));
	}
}

bool CStream::CanWrite(size_t size) {
	return size <= (sizeof(outputStreamData) - outputStreamPosition);
}

void CStream::Write(const void* value, size_t size) {
	if (CanWrite(size)) {
		memcpy(&outputStreamData[outputStreamPosition], value, size);
		outputStreamPosition += size;

		if (outputStreamPosition > outputStreamEnd) {
			outputStreamEnd = outputStreamPosition;
		}
	}
	else {
		outputStreamError = true;
	}
}

void CStream::SendStream(CPlayer* player) {
	functions->SendClientScriptData(player != nullptr ? player->GetID() : -1, outputStreamData, outputStreamEnd);

	outputStreamPosition = 0;
	outputStreamEnd = 0;
	outputStreamError = false;
}

// ANTI-CLUMSY PROTECTED LOADINPUT
void CStream::LoadInput(const void* data, size_t size) {
	// Reset the previous state immediately before validating
	inputStreamSize = 0;
	inputStreamPosition = 0;
	inputStreamError = true; // Default to true, flipped only if all checks pass

	if (data == nullptr || size == 0) {
		OutputWarning("CStream::LoadInput: Blocked! Null or empty packet payload received.");
		return;
	}

	// Mitigate network exploits / malicious oversized packets
	if (size > STREAM_MAX_SIZE) {
		std::string warningMsg = "CStream::LoadInput: Blocked! Suspect payload size over " + std::to_string(STREAM_MAX_SIZE) + " bytes.";
		OutputWarning(warningMsg.c_str());
		return;
	}

	// Buffer validation successful. Safe to move bytes.
	inputStreamSize = size;
	inputStreamError = false;
	memcpy(inputStreamData, data, size);
}

void CStream::SetReadPosition(int position) {
	if (position < 0 || (size_t)position > inputStreamSize) {
		inputStreamPosition = inputStreamSize;
		inputStreamError = true;
	}
	else {
		inputStreamPosition = (size_t)position;
	}
}

int CStream::GetReadPosition(void) {
	return (int)inputStreamPosition;
}

int CStream::GetReadSize(void) {
	return (int)inputStreamSize;
}

bool CStream::HasReadError(void) {
	return inputStreamError;
}

int CStream::ReadByte(void) {
	// Terminate if the stream is already corrupted or reading goes past the actual size
	if (inputStreamError || inputStreamPosition + sizeof(uint8_t) > inputStreamSize) {
		inputStreamError = true;
		return 0;
	}
	return inputStreamData[inputStreamPosition++];
}

int CStream::ReadInt(void) {
	if (inputStreamError || inputStreamPosition + sizeof(int) > inputStreamSize) {
		inputStreamError = true;
		return 0;
	}

	int result = 0;
	// Direct memcpy extraction to prevent memory alignment exceptions from altered network bytes
	memcpy(&result, &inputStreamData[inputStreamPosition], sizeof(int));
	inputStreamPosition += sizeof(int);
	return result;
}

float CStream::ReadFloat(void) {
	if (inputStreamError || inputStreamPosition + sizeof(float) > inputStreamSize) {
		inputStreamError = true;
		return 0.0f;
	}

	float result = 0.0f;
	memcpy(&result, &inputStreamData[inputStreamPosition], sizeof(float));
	inputStreamPosition += sizeof(float);
	return result;
}

uint16_t CStream::ReadBEInt16(void) {
	if (inputStreamError || inputStreamPosition + sizeof(uint16_t) > inputStreamSize) {
		inputStreamError = true;
		return 0;
	}

	uint16_t result = 0;
	memcpy(&result, &inputStreamData[inputStreamPosition], sizeof(uint16_t));
	inputStreamPosition += sizeof(uint16_t);
	return ((result >> 8) & 0xFF) | ((result & 0xFF) << 8);
}

SQChar* CStream::ReadString(void) {
	static SQChar result[4096];
	result[0] = _SC('\0'); // Clear previous static execution data

	if (inputStreamError) {
		return result;
	}

	uint16_t length = ReadBEInt16();
	if (inputStreamError) {
		return result;
	}

	size_t bytesNeeded = length * sizeof(SQChar);

	// Stop execution if the incoming packet length field lies about the payload boundaries
	if (inputStreamPosition + bytesNeeded > inputStreamSize) {
		inputStreamError = true;
		return result;
	}

	uint16_t maxChars = (sizeof(result) / sizeof(SQChar)) - 1;
	if (length > maxChars) {
		length = maxChars; // Keep string size within internal memory bounds
	}

	if (length > 0) {
		memcpy(result, &inputStreamData[inputStreamPosition], length * sizeof(SQChar));
	}
	result[length] = _SC('\0');

	inputStreamPosition += length * sizeof(SQChar);
	return result;
}

void CStream::RegisterStream(void) {
	Class<CStream> c(v, "Stream");

	// Deduce signatures natively without explicit mask arguments to maintain cross-version stability
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