#pragma once
#include <basic_render.hpp>
#include <detail/io_service_pool.h>
#include <detail/string.hpp>
#include <event_service.hpp>

namespace light_wind
{
	class application
	{
		using executor_type = boost::asio::io_context::executor_type;

		using render_type = basic_render<executor_type>;

	public:
		class init_async_wait
		{
		public:
			explicit init_async_wait(application* self);

		public:
			const executor_type& get_executor() const noexcept;

			template <typename Handler, typename Func, typename IoExecutor, typename Event>
			void operator()(Handler&& handle, const IoExecutor& peer_ex, Func&& f, Event*) const
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handle);
				self_->impl_.get_service().async_wait<Event>(self_->impl_.get_implementation(), handler2.value, peer_ex,
															 std::forward<Func>(f), self_->impl_.get_executor());
			}

		private:
			application* self_;
		};

	public:
		explicit application(int argc, char** argv);

		virtual ~application();

	public:
		const executor_type& get_executor() noexcept;

		int run();

		virtual bool preinit();

		virtual void stop();

		template <typename Event, typename Func, typename IoExecutor, typename CompleteToken>
		auto async_wait(const IoExecutor& ex, Func&& loop_if_failed_func,
						CompleteToken&& token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<CompleteToken,
													void(boost::system::error_code, std::shared_ptr<Event>)>(
				std::declval<init_async_wait>(), token, ex, loop_if_failed_func, static_cast<Event*>(0)))
		{
			return boost::asio::async_initiate<CompleteToken, void(boost::system::error_code, std::shared_ptr<Event>)>(
				init_async_wait(this), token, ex, std::forward<Func>(loop_if_failed_func), static_cast<Event*>(0));
		}

		template <typename Event, typename Func, typename ExecutorContext, typename CompleteToken>
		auto async_wait(ExecutorContext& context, Func&& loop_if_failed_func,
						CompleteToken&& token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<CompleteToken,
													void(boost::system::error_code, std::shared_ptr<Event>)>(
				std::declval<init_async_wait>(), token, context.get_executor(), loop_if_failed_func,
				static_cast<Event*>(0)))
		{
			return boost::asio::async_initiate<CompleteToken, void(boost::system::error_code, std::shared_ptr<Event>)>(
				init_async_wait(this), token, context.get_executor(), std::forward<Func>(loop_if_failed_func),
				static_cast<Event*>(0));
		}

		auto start_event_loop() -> boost::asio::awaitable<void>;

		void show();

	private:
		template <typename Event>
		auto process_event(std::shared_ptr<Event> evt) -> boost::asio::awaitable<void>
		{
			co_return;
		}

		void parse_command(int argc, char** argv);

		void create_main_window(const string& title, const string& name);

	private:
		boost::asio::io_context main_io_;

		std::shared_ptr<io_service_pool> pool_ptr_;

		std::shared_ptr<std::thread> event_loop_thread_ptr_;

		boost::asio::detail::io_object_impl<event_service, executor_type> impl_;

		std::shared_ptr<render_type> render_ptr_;

		string name_;
		
		string title_;
	};
} // namespace light_wind