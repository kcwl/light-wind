#pragma once
#ifdef _WIN32
#include <detail/directd3d_11.h>
#include <detail/win_iocp_render_device.hpp>
#endif

namespace light_wind
{
#ifdef _WIN32
	using rhi_service = win_iocp_render_device<DX::directd3d_11>;
#endif
} // namespace light_wind
