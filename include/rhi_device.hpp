#pragma once
#include <window_device_service.hpp>
#include <directd3d_11.hpp>


namespace light_wind
{
#ifdef _WIN32
	using rhi_device = window_device_service<directd3d_11>;
#endif
}
