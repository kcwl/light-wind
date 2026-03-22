#pragma once
#include <string>

namespace light_wind
{
#ifdef _UNICODE
	using string = std::wstring;
#else
	using string = std::string;
#endif
} // namespace light_wind