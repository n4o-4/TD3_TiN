#pragma once
#include <Windows.h>
#include <string>
#include <StringUtility.h>

namespace Logger {

	void Log(const std::string& message);

	void wLog(const std::wstring& message);

 }