#pragma once
#include <detail/string.hpp>
#include <detail/win_iocp_event_wait_op.hpp>
#include <detail/win_iocp_service_base.hpp>

namespace light_wind
{
	class win_iocp_event_service : public boost::asio::detail::execution_context_service_base<win_iocp_event_service>,
								   public win_iocp_service_base
	{
	public:
		struct implementation_type : base_implementation_type
		{};

	public:
		win_iocp_event_service(boost::asio::execution_context& context)
			: boost::asio::detail::execution_context_service_base<win_iocp_event_service>(context)
			, win_iocp_service_base()
			, iocp_service_(boost::asio::use_service<boost::asio::detail::win_iocp_io_context>(context))
		{}

	public:
		// void construct(implementation_type& impl)
		//{
		// }

		// void destroy(implementation_type& impl)
		//{}

		 void shutdown()
		{}

	public:
		void create_window(implementation_type& impl, const string& title, const string& name);

		void show(implementation_type& impl);

	public:
		template <typename Event, typename Handler, typename Func, typename PeerIoExecutor, typename IoExecutor>
		void async_wait(implementation_type& impl, Handler& handler, const PeerIoExecutor& peer_ex, Func&& f,
						const IoExecutor& io)
		{
			using op = win_iocp_event_wait_op<Event, Handler, PeerIoExecutor, IoExecutor>;

			typename op::ptr p{ boost::asio::detail::addressof(handler), op::ptr::allocate(handler), 0 };

			boost::asio::detail::operation* o = p.p = new (p.v) op(handler, peer_ex, io);

			start_wait_op(impl, (impl.state & static_cast<int>(event_state::success)) != 0, static_cast<op*>(o)->msg(),
						  o, std::forward<Func>(f), io);
			p.v = p.p = 0;
		}

		template <typename Event>
		std::unique_ptr<Event> wait(implementation_type& impl)
		{
			MSG msg{};

			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

			return std::make_unique<Event>(msg);
		}

	private:
		template <typename IoExecutor, typename Func>
		void start_wait_op(implementation_type& impl, bool noop, MSG& msg, boost::asio::detail::operation* op, Func&& f,
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
					[&, func = std::move(f)] () -> boost::asio::awaitable<void>
					{
						while (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
						{
							func();
						}

						iocp_service_.on_completion(op);

						co_return;
					},
					boost::asio::detached);
			}
		}

	private:
		boost::asio::detail::win_iocp_io_context& iocp_service_;
	};
} // namespace light_wind