#include <application.hpp>

namespace light_wind
{
	application::init_async_wait::init_async_wait(application* self)
		: self_(self)
	{}

	const application::executor_type& application::init_async_wait::get_executor() const noexcept
	{
		return self_->get_executor();
	}

	application::application(int argc, char** argv)
		: main_io_()
		, impl_(0, 0, main_io_)
	{
		parse_command(argc, argv);
	}

	application::~application()
	{
		if (event_loop_thread_ptr_ && event_loop_thread_ptr_->joinable())
		{
			event_loop_thread_ptr_->join();
		}
	}

	const application::executor_type& application::get_executor() noexcept
	{
		return impl_.get_executor();
	}

	int application::run()
	{
		main_io_.run();

		return 0;
	}

	bool application::preinit()
	{
		return true;
	}

	void application::stop()
	{
		return;
	}

	auto application::start_event_loop() -> boost::asio::awaitable<void>
	{
		preinit();

		for (;;)
		{
			boost::system::error_code ec{};
			auto event = co_await async_wait<event_type>(pool_ptr_->get_io_service(),
														 boost::asio::redirect_error(boost::asio::use_awaitable, ec));

			if (!event)
			{
				continue;
			}

			if (event->kind() == event_kind::quit)
			{
				break;
			}

			boost::asio::co_spawn(event->get_executor(), process_event(event), boost::asio::detached);
		}

		stop();

		pool_ptr_->stop();
	}

	void application::show()
	{
		impl_.get_service().show(impl_.get_implementation());
	}

	void application::create_main_window(const string& title, const string& name)
	{
		impl_.get_service().create_window(impl_.get_implementation(), title, name);
	}

	void application::parse_command(int argc, char** argv)
	{
		boost::asio::co_spawn(main_io_, start_event_loop(), boost::asio::detached);

		pool_ptr_ = std::make_shared<io_service_pool>(std::thread::hardware_concurrency());

		event_loop_thread_ptr_ = std::make_shared<std::thread>([&] { pool_ptr_->run(); });
	}
} // namespace light_wind