#pragma once
#ifdef _WIN32
#include <detail/win_iocp_event_service.h>
#endif

namespace light_wind
{
#ifdef _WIN32
	using event_service = win_iocp_event_service;
#endif
} // namespace light_wind
