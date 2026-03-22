#pragma once
#include <detail/directd3d_11.hpp>
#include <detail/window_device_service.hpp>

namespace light_wind
{
#ifdef _WIN32
	using rhi_device = window_device_service<directd3d_11>;
#endif
} // namespace light_wind
