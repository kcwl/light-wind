#include <detail/win_iocp_service_base.hpp>
#include <detail/window_handle.hpp>

namespace light_wind
{
	void win_iocp_service_base::construct(base_implementation_type& impl)
	{
		impl.handle = window_handle::handle;
		impl.instance = window_handle::instance;
	}

	void win_iocp_service_base::destroy(base_implementation_type& impl)
	{}

	void win_iocp_service_base::shutdown()
	{}
} // namespace light_wind