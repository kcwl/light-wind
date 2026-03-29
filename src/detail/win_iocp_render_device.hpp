#pragma once
#include <detail/win_device_draw_op.hpp>
#include <render/geometry.h>
#include <detail/win_iocp_service_base.hpp>

namespace light_wind
{
	template <typename Device>
	class win_iocp_render_device
		: public boost::asio::detail::execution_context_service_base<win_iocp_render_device<Device>>
		, public win_iocp_service_base
	{
	public:
		struct implementation_type : base_implementation_type {};
	public:
		win_iocp_render_device(boost::asio::execution_context& context)
			: boost::asio::detail::execution_context_service_base<win_iocp_render_device<Device>>(context)
			, win_iocp_service_base()
			, iocp_service_(boost::asio::use_service<boost::asio::detail::win_iocp_io_context>(context))
			, render_device_()
		{}

	public:
		//void construct(implementation_type& impl)
		//{}

		//void destroy(implementation_type& impl)
		//{}

		void shutdown()
		{}

	public:
		template <typename Handler, typename IoExecutor>
		auto async_draw(implementation_type& impl, Handler& handler, const render::rectangle& rc, const IoExecutor& ex)
		{
			using op = win_device_draw_op<Handler, IoExecutor>;

			typename op::ptr p{ boost::asio::detail::addressof(handler), op::ptr::allocate(handler), 0 };

			boost::asio::detail::operation* o = p.p = new (p.v) op(handler, ex);

			start_draw2d_op(impl, (impl.state & static_cast<int>(event_state::success)) != 0, rc, o, ex);
			p.v = p.p = 0;
		}

		void present()
		{
			return render_device_.Present();
		}

	private:
		template <typename Resource, typename IoExecutor>
		void start_draw2d_op(implementation_type& impl, bool noop, const Resource& resource,
							 boost::asio::detail::operation* op, const IoExecutor& ex)
		{
			this->iocp_service_.work_started();

			if (noop)
			{
				render_device_.draw_rectangle(resource.left, resource.top, resource.right, resource.bottom);

				iocp_service_.on_completion(op);
			}
			else
			{
				boost::asio::co_spawn(
					ex,
					[&]() -> boost::asio::awaitable<void>
					{
						if constexpr (std::same_as<Resource, render::rectangle>)
						{
							render_device_.draw_rectangle(resource.left, resource.top, resource.right, resource.bottom);
						}

						iocp_service_.on_completion(op);

						co_return;
					},
					boost::asio::detached);
			}
		}

	private:
		boost::asio::detail::win_iocp_io_context& iocp_service_;

		Device render_device_;
	};
} // namespace light_wind