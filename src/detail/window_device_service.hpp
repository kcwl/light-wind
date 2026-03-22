#pragma once
#include <detail/windows_device_service_base.h>

namespace light_wind
{
	template <typename Device>
	class window_device_service
		: public boost::asio::detail::execution_context_service_base<window_device_service<Device>>,
		  public windows_device_service_base
	{
	public:
		struct implementation_type : base_implementation_type
		{};

	public:
		window_device_service(boost::asio::execution_context& context)
			: boost::asio::detail::execution_context_service_base<window_device_service<Device>>(context)
			, windows_device_service_base(context)
		{}

	public:
		void shutdown()
		{}
	};
} // namespace light_wind