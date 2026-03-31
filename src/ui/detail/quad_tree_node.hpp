#pragma once
#include <ui/detail/rectangle.hpp>

namespace light_wind
{
	template <typename T>
	class quad_tree_node
	{
	public:
		static constexpr int DIVISION = 50;

		static constexpr int MERGE = 15;

		using rect_type = rectangle<int>;

		quad_tree_node(rect_type size)
			: node_size_(size)
		{}

		~quad_tree_node() = default;

		// 插入元素
		bool insert(std::shared_ptr<T> element)
		{
			if (!element)
				return false;

			if (!contain(element->absolute()))
			{
				return false;
			}

			if (relations_.size() < DIVISION)
			{
				relations_.push_back(element);
				return true;
			}

			if (relations_.size() >= DIVISION)
			{
				sub_divide();
			}

			for (auto& r : relations_)
			{
				bool result = left_up_ptr_->insert(element);

				if (!result)
				{
					result = right_up_ptr_->insert(element);
					if (!result)
					{
						result = left_bottom_ptr_->insert(element);
						if (!result)
						{
							right_bottom_ptr_->insert(element);
						}
					}
				}
			}

			relations_.clear();

			return true;
		}

		std::shared_ptr<T> find_top_order(coordinate2D<int> point)
		{
			std::map<std::size_t, std::shared_ptr<T>, std::greater<std::size_t>> result{};

			find(point, result);

			if (result.empty())
			{
				return nullptr;
			}

			return *result.begin();
		}

		std::size_t size() const
		{
			return left_up_ptr_->relations_.size() + left_bottom_ptr_->relations_.size() +
				   right_up_ptr_->relations_.size() + right_bottom_ptr_->relations_.size();
		}

		bool remove(std::shared_ptr<T> element)
		{
			if (!element)
			{
				return true;
			}

			if (!contain(element->position()))
			{
				return false;
			}

			bool result = left_up_ptr_->remove(element);
			if (!result)
			{
				result = left_bottom_ptr_->remove(element);
				if (!result)
				{
					result = right_up_ptr_->remove(element);
					if (!result)
					{
						result = right_bottom_ptr_->remove(element);
					}
				}
			}

			if (!result)
			{
				return result;
			}

			if (size() < MERGE)
			{
				sub_merge();
			}

			return result;
		}

	private:
		void sub_divide()
		{
			auto half_width = (node_size_.right - node_size_.left) / 2;
			auto half_height = (node_size_.bottom - node_size_.top) / 2;

			left_up_ptr_ = std::make_shared<quad_tree_node<T>>(rectangle<int>{
				node_size_.left, node_size_.top, node_size_.left + half_width, node_size_.top + half_height });
			right_up_ptr_ = std::make_shared<quad_tree_node<T>>(rectangle<int>{
				node_size_.left + half_width, node_size_.top, node_size_.right, node_size_.top + half_height });
			left_bottom_ptr_ = std::make_shared<quad_tree_node<T>>(rectangle<int>{
				node_size_.left, node_size_.top + half_height, node_size_.left + half_width, node_size_.bottom });
			right_bottom_ptr_ = std::make_shared<quad_tree_node<T>>(rectangle<int>{
				node_size_.left + half_width, node_size_.top + half_height, node_size_.right, node_size_.bottom });
		}

		void sub_merge()
		{
			std::copy(left_up_ptr_->relations_.begin(), left_up_ptr_->relations_.end(), std::back_inserter(relations_));

			left_up_ptr_.reset();

			std::copy(left_bottom_ptr_->relations_.begin(), left_bottom_ptr_->relations_.end(),
					  std::back_inserter(relations_));

			left_bottom_ptr_.reset();

			std::copy(right_up_ptr_->relations_.begin(), right_up_ptr_->relations_.end(),
					  std::back_inserter(relations_));

			right_up_ptr_.reset();

			std::copy(right_bottom_ptr_->relations_.begin(), right_bottom_ptr_->relations_.end(),
					  std::back_inserter(relations_));

			right_bottom_ptr_.reset();
		}

		bool contain(coordinate2D<int> point)
		{
			return point.x() >= node_size_.left && point.y() <= node_size_.right && point.y() >= node_size_.top &&
				   point.y() <= node_size_.bottom;
		}

		template <typename ResultMap>
		void find(coordinate2D<int> point, ResultMap& result)
		{
			if (!relations_.empty())
			{
				for (auto& r : relations_)
				{
					if (!r->contain(point))
					{
						continue;
					}

					result.insert({ r->order(), r });
				}

				return;
			}

			if (!left_up_ptr_->find(point, result))
			{
				if (!right_up_ptr_->find(point, result))
				{
					if (!left_bottom_ptr_->find(point, result))
					{
						right_bottom_ptr_->find(point, result);
					}
				}
			}
		}

	private:
		rectangle<int> node_size_;

		std::vector<std::shared_ptr<T>> relations_;

		std::shared_ptr<quad_tree_node<T>> left_up_ptr_;
		std::shared_ptr<quad_tree_node<T>> right_up_ptr_;
		std::shared_ptr<quad_tree_node<T>> left_bottom_ptr_;
		std::shared_ptr<quad_tree_node<T>> right_bottom_ptr_;
	};
} // namespace light_wind