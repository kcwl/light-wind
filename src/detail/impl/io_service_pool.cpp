#include <detail/io_service_pool.h>

namespace light_wind
{
	io_service_pool::io_service_pool(std::size_t pool_size)
		: io_services_()
		, pool_size_(pool_size)
		, next_to_service_(0)
		, works_()
		, enable_(false)
	{
		if (pool_size == 0)
			throw std::runtime_error("io_service_pool size is 0");

		for (std::size_t i = 0; i < pool_size_; ++i)
		{
			io_service_ptr_t io_service_ptr(new boost::asio::io_context{});

			io_services_.push_back(io_service_ptr);

			works_.push_back(make_work_guard(*io_service_ptr));
		}
	}

	void io_service_pool::run()
	{
		std::vector<std::shared_ptr<std::thread>> threads;

		for (auto& io_service : io_services_)
		{
			threads.push_back(std::make_shared<std::thread>([&] { io_service->run(); }));
		}

		enable_ = true;

		for (auto& thread : threads)
		{
			thread->join();
		}

		enable_ = false;
	}

	std::size_t io_service_pool::pool_size() const
	{
		return pool_size_;
	}

	void io_service_pool::stop()
	{
		for (auto& io_service : io_services_)
		{
			io_service->stop();
		}

		enable_ = false;
	}

	bool io_service_pool::enable() const
	{
		return enable_;
	}

	boost::asio::io_context& io_service_pool::get_io_service()
	{
		boost::asio::io_context& io_service = *io_services_[next_to_service_];
		++next_to_service_;

		if (next_to_service_ == io_services_.size())
			next_to_service_ = 0;

		return io_service;
	}

	std::size_t io_service_pool::size() const
	{
		return pool_size_;
	}
} // namespace light_wind