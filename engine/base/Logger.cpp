#include "Logger.h"

namespace Logger {
	void Log(const std::string& message)
	{
		OutputDebugStringA(message.c_str());
	}
	void wLog(const std::wstring& message)
	{
		OutputDebugStringW(message.c_str());
	}
}