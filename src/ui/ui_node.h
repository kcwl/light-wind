#pragma once
#include <ui/position.hpp>

namespace light_wind
{
	class ui_node : public position<int32_t>, public std::enable_shared_from_this<ui_node>
	{
	public:
		ui_node();
		virtual ~ui_node() = default;

	public:
		std::shared_ptr<ui_node> brother_prev();

		std::shared_ptr<ui_node> next_brother();

		std::shared_ptr<ui_node> parent();

		std::shared_ptr<ui_node> first_child();

		std::shared_ptr<ui_node> end_child();

		std::shared_ptr<ui_node> root();

		void set_parent(std::shared_ptr<ui_node> parent);

		void add_child(std::shared_ptr<ui_node> child_ptr);

		void add_brother(std::shared_ptr<ui_node> brother_ptr);

		void bring_to_top();

		void leave_family();
		void leave_alone();

		void move(int dura_x, int dura_y);

	private:
		void update_brother(std::shared_ptr<ui_node> brother_ptr);

		void set_child(std::shared_ptr<ui_node> child_ptr);

		void set_brother_prev(std::shared_ptr<ui_node> ptbrother_ptrr);

		void set_brother_next(std::shared_ptr<ui_node> brother_ptr);

	protected:
		std::weak_ptr<ui_node> prev_ptr_;
		std::weak_ptr<ui_node> next_ptr_;
		std::weak_ptr<ui_node> first_child_ptr_;
		std::weak_ptr<ui_node> end_child_ptr_;
		std::weak_ptr<ui_node> parent_ptr_;
		std::weak_ptr<ui_node> root_ptr_;
	};
} // namespace light_wind