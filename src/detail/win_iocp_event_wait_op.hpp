#pragma once
#include <boost/asio.hpp>
#include <event.hpp>

namespace light_wind
{
	template <typename Event, typename Handler, typename IoExecutor, typename PeerIoExecutor>
	class win_iocp_event_wait_op : public boost::asio::detail::operation
	{
		using event_t = Event;

	public:
		BOOST_ASIO_DEFINE_HANDLER_PTR(win_iocp_event_wait_op);

		win_iocp_event_wait_op(Handler& handler, const IoExecutor& io, const PeerIoExecutor& peer_ex)
			: boost::asio::detail::operation(&win_iocp_event_wait_op::do_complete)
			, handler_(static_cast<Handler&&>(handler))
			, work_(handler_, io)
			, event_ptr_(nullptr)
			, msg_()
			, peer_ex_(peer_ex)
		{}

	public:
		void make_event()
		{
			TranslateMessage(&msg_);

			switch (msg_.message)
			{
			case WM_SETCURSOR:
				{
				}
				break;
			case WM_IME_CHAR:
				{
					event_ptr_ = std::make_shared<key_press<PeerIoExecutor>>(peer_ex_);
				}
				break;
			case WM_INPUTLANGCHANGEREQUEST:
			case WM_IME_NOTIFY:
				{
				}
				break;
			case WM_ACTIVATEAPP:
				{
				}
				break;
			case WM_SYSCOMMAND:
				{
				}
				break;
			case WM_COPYDATA:
				{
				}
				break;
			case WM_NCACTIVATE:
			case WM_NCHITTEST:
				{
				}
				break;
			case WM_MOVE:
			case WM_MOVING:
				{
					event_ptr_ = std::make_shared<mouse_move_event<PeerIoExecutor>>(peer_ex_);
				}
				break;
			case WM_WINDOWPOSCHANGED:
			case WM_WINDOWPOSCHANGING:
				{
				}
				break;
			case WM_MOUSELEAVE:
				{
				}
				break;
			case WM_GETMINMAXINFO:
				{
				}
				break;
			default:
				{
					DispatchMessage(&msg_);
				}
			}
		}

		std::shared_ptr<event_t> get_event()
		{
			return event_ptr_;
		}

		MSG& msg()
		{
			return msg_;
		}

		static void do_complete(void* owner, boost::asio::detail::operation* base,
								const boost::system::error_code& result_ec, std::size_t)
		{
			boost::system::error_code ec(result_ec);

			// Take ownership of the operation object.
			BOOST_ASIO_ASSUME(base != 0);
			win_iocp_event_wait_op* o(static_cast<win_iocp_event_wait_op*>(base));
			ptr p = { boost::asio::detail::addressof(o->handler_), o, o };

			BOOST_ASIO_HANDLER_COMPLETION((*o));

			// Take ownership of the operation's outstanding work.
			boost::asio::detail::handler_work<Handler, IoExecutor> w(
				static_cast<boost::asio::detail::handler_work<Handler, IoExecutor>&&>(o->work_));

			BOOST_ASIO_ERROR_LOCATION(ec);

			o->make_event();

			// Make a copy of the handler so that the memory can be deallocated before
			// the upcall is made. Even if we're not about to make an upcall, a
			// sub-object of the handler may be the true owner of the memory associated
			// with the handler. Consequently, a local copy of the handler is required
			// to ensure that any owning sub-object remains valid until after we have
			// deallocated the memory here.
			boost::asio::detail::binder2<Handler, boost::system::error_code, std::shared_ptr<event_t>> handler(o->handler_, ec,
																							  o->get_event());
			p.h = boost::asio::detail::addressof(handler.handler_);
			p.reset();

			// Make the upcall if required.
			if (owner)
			{
				boost::asio::detail::fenced_block b(boost::asio::detail::fenced_block::half);
				BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
				w.complete(handler, handler.handler_);
				BOOST_ASIO_HANDLER_INVOCATION_END;
			}
		}

	private:
		Handler handler_;

		boost::asio::detail::handler_work<Handler, IoExecutor> work_;

		std::shared_ptr<event_t> event_ptr_;

		MSG msg_;

		PeerIoExecutor peer_ex_;
	};
} // namespace light_wind