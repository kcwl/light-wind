#pragma once
#include <boost/asio.hpp>

namespace light_wind
{
	class io_service_pool
	{
	private:
		using io_service_ptr_t = std::shared_ptr<boost::asio::io_context>;

		using work_guard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

	public:
		explicit io_service_pool(std::size_t pool_size);

	public:
		void run();

		std::size_t pool_size() const;

		void stop();

		bool enable() const;

		boost::asio::io_context& get_io_service();

		std::size_t size() const;

	private:
		io_service_pool(const io_service_pool&) = delete;

		io_service_pool& operator=(const io_service_pool&) = delete;

	private:
		std::vector<io_service_ptr_t> io_services_;

		std::size_t pool_size_;

		std::size_t next_to_service_;

		std::list<work_guard> works_;

		bool enable_;
	};
} // namespace light_wind