#pragma once
#include <Windows.h>
#include <cstdint>

namespace light_wind
{
	class win_iocp_service_base
	{
	public:
		enum class event_state
		{
			success
		};

		struct base_implementation_type
		{
			HWND handle;
			HINSTANCE instance;
			uint8_t state;
		};

	public:
		void construct(base_implementation_type& impl);

		void destroy(base_implementation_type& impl);

		void shutdown();
	};
} // namespace light_wind