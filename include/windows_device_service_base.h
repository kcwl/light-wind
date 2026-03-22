#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <string.hpp>
#include <win_iocp_event_wait_op.hpp>

namespace light_wind
{
	class windows_device_service_base
	{
		enum class event_state
		{
			success
		};

	public:
		struct base_implementation_type
		{
			HWND handle;
			HINSTANCE instance;
			uint8_t state;
		};

	public:
		windows_device_service_base(boost::asio::execution_context& context);

		void construct(base_implementation_type& impl)
		{}

		void destroy(base_implementation_type& impl)
		{}

	public:
		void create_window(base_implementation_type& impl, const string& title, const string& name);

		void show(base_implementation_type& impl);

		template <typename Event, typename Handler, typename PeerIoExecutor, typename IoExecutor>
		void async_wait(base_implementation_type& impl, Handler& handler, const PeerIoExecutor& peer_ex,
						const IoExecutor& io)
		{
			boost::asio::associated_cancellation_slot_t<Handler> slot =
				boost::asio::get_associated_cancellation_slot(handler);

			using op = win_iocp_event_wait_op<Event, Handler, PeerIoExecutor, IoExecutor>;

			typename op::ptr p{ boost::asio::detail::addressof(handler), op::ptr::allocate(handler), 0 };

			boost::asio::detail::operation* o = p.p = new (p.v) op(handler, peer_ex, io);

			start_wait_op(impl, (impl.state & static_cast<int>(event_state::success)) != 0, static_cast<op*>(o)->msg(),
						  o, io);
			p.v = p.p = 0;
		}

		template <typename Event>
		std::unique_ptr<Event> wait(base_implementation_type& impl)
		{
			MSG msg{};

			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

			return std::make_unique<Event>(msg);
		}

	private:
		template <typename IoExecutor>
		void start_wait_op(base_implementation_type& impl, bool noop, MSG& msg, boost::asio::detail::operation* op,
						   const IoExecutor& ex)
		{
			iocp_service_.work_started();

			if (noop)
			{
				PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

				iocp_service_.on_completion(op);
			}
			else
			{
				boost::asio::co_spawn(
					ex,
					[&]() -> boost::asio::awaitable<void>
					{
						while (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
							;

						iocp_service_.on_completion(op);

						co_return;
					},
					boost::asio::detached);
			}
		}

	private:
		boost::asio::execution_context& context_;

		boost::asio::detail::win_iocp_io_context& iocp_service_;
	};
} // namespace light_wind