#pragma once
#include <list>
#include <mutex>

namespace light_wind
{
	template <typename T, auto Max>
	class limit_list
	{
	public:
		limit_list() = default;

	public:
		void push_back(std::shared_ptr<T> node)
		{
			std::lock_guard lk(mutex_);

			if (nodes_.size() >= Max)
			{
				nodes_.pop_back();
			}

			nodes_.push_front(node);
		}

		void erase(std::shared_ptr<T> node)
		{
			std::lock_guard lk(mutex_);

			auto iter = std::find_if(nodes_.begin, nodes_.end(), [&](auto ptr) { return ptr.lock() == node; });

			if (iter == nodes_.end())
			{
				return;
			}

			nodes_.erase(iter);
		}

		std::shared_ptr<T> front()
		{
			std::lock_guard lk(mutex_);

			auto ptr = nodes_.front();

			return ptr->lock();
		}

	private:
		std::mutex mutex_;

		std::list<std::weak_ptr<T>> nodes_;
	};
} // namespace light_wind