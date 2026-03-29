#pragma once
#include <Windows.h>

namespace light_wind
{
	struct window_handle
	{
		static HWND handle;

		static HINSTANCE instance;
	};
} // namespace light_wind

inline HWND light_wind::window_handle::handle;

inline HINSTANCE light_wind::window_handle::instance;