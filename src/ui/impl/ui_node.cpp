#include <ui/ui_node.h>

namespace light_wind
{

	ui_node::ui_node()
		: prev_ptr_()
		, next_ptr_()
		, first_child_ptr_()
		, parent_ptr_()
	{}

	std::shared_ptr<ui_node> ui_node::brother_prev()
	{
		return prev_ptr_.expired() ? nullptr : prev_ptr_.lock();
	}
	std::shared_ptr<ui_node> ui_node::next_brother()
	{
		return next_ptr_.expired() ? nullptr : next_ptr_.lock();
	}
	std::shared_ptr<ui_node> ui_node::parent()
	{
		return parent_ptr_.expired() ? nullptr : parent_ptr_.lock();
	}
	std::shared_ptr<ui_node> ui_node::first_child()
	{
		return first_child_ptr_.expired() ? nullptr : first_child_ptr_.lock();
	}
	std::shared_ptr<ui_node> ui_node::end_child()
	{
		return end_child_ptr_.expired() ? nullptr : end_child_ptr_.lock();
	}
	std::shared_ptr<ui_node> ui_node::root()
	{
		return root_ptr_.expired() ? nullptr : root_ptr_.lock();
	}

	void ui_node::set_parent(std::shared_ptr<ui_node> parent)
	{
		parent_ptr_.reset();
		parent_ptr_ = parent;
	}

	void ui_node::add_child(std::shared_ptr<ui_node> child_ptr)
	{
		if (!child_ptr)
		{
			return;
		}

		child_ptr->leave_family();

		child_ptr->absolute().move(absolute().x() + child_ptr->relative().x() - child_ptr->absolute().x(),
								   absolute().y() + child_ptr->relative().y() - child_ptr->absolute().y());

		child_ptr->set_parent(this->shared_from_this());

		set_child(child_ptr);
	}

	void ui_node::add_brother(std::shared_ptr<ui_node> brother_ptr)
	{
		update_brother(brother_ptr);

		if (!next_brother())
		{
			next_ptr_ = brother_ptr;
			brother_ptr->set_brother_prev(shared_from_this());
		}
		else
		{
			auto next_ptr = next_brother();
			while (next_ptr->next_brother())
			{
				auto ptr = next_ptr->next_brother();
				next_ptr.swap(ptr);
			}

			next_ptr->add_brother(brother_ptr);
			brother_ptr->set_brother_prev(next_ptr);
		}
	}

	void ui_node::bring_to_top()
	{
		auto next_ptr = next_brother();

		if (!next_ptr)
		{
			return;
		}

		auto last = next_ptr;

		while (auto next = last->next_brother())
			last.swap(next);

		last->add_brother(shared_from_this());

		next_ptr->set_brother_prev(brother_prev());

		if (auto prev = brother_prev())
		{
			prev->add_brother(next_ptr);
		}
		else if (auto p = parent())
		{
			if (p->first_child() == shared_from_this())
			{
				p->set_child(next_ptr);
			}
		}

		next_brother().reset();
		set_brother_prev(last);

		end_child_ptr_.reset();
		end_child_ptr_ = shared_from_this();
	}

	void ui_node::leave_family()
	{
		if (auto prev = prev_ptr_.lock())
			prev->next_ptr_ = next_ptr_;
		else if (auto parent = parent_ptr_.lock())
			parent->first_child_ptr_ = next_ptr_;
		if (auto next = next_ptr_.lock())
			next->prev_ptr_ = prev_ptr_;
		prev_ptr_.reset();
		next_ptr_.reset();
		parent_ptr_.reset();
	}

	void ui_node::leave_alone()
	{
		leave_family();
		while (auto child = first_child_ptr_.lock())
		{
			child->parent_ptr_.reset();
			child = child->next_ptr_.lock();
		}
	}

	void ui_node::move(int dura_x, int dura_y)
	{
		absolute().move(dura_x, dura_y);

		auto child = first_child_ptr_.lock();
		while (child)
		{
			child->absolute().move(dura_x, dura_y);
			child = child->next_ptr_.lock();
		}
	}

	void ui_node::update_brother(std::shared_ptr<ui_node> brother_ptr)
	{
		if (!brother_ptr)
		{
			return;
		}
		brother_ptr->leave_family();

		if (auto ptr = parent())
		{
			brother_ptr->absolute().move(
				ptr->absolute().x() + brother_ptr->relative().x() - brother_ptr->absolute().x(),
				ptr->absolute().y() + brother_ptr->relative().y() - brother_ptr->absolute().y());

			brother_ptr->set_parent(ptr);
		}
	}

	void ui_node::set_child(std::shared_ptr<ui_node> child_ptr)
	{
		if (auto child = first_child_ptr_.lock())
			child.swap(child_ptr);
		else
		{
			auto bro = first_child_ptr_.lock();
			while (auto next = bro->next_ptr_.lock())
			{
				auto bro_next = bro->next_ptr_.lock();
				bro.swap(bro_next);

				next.swap(child_ptr);
				child_ptr->brother_prev().swap(bro);
			}
		}

		end_child_ptr_.reset();
		end_child_ptr_ = child_ptr;
	}

	void ui_node::set_brother_prev(std::shared_ptr<ui_node> brother_ptr)
	{
		if (!prev_ptr_.expired())
		{
			prev_ptr_.reset();
		}

		prev_ptr_ = brother_ptr;
	}

	void ui_node::set_brother_next(std::shared_ptr<ui_node> brother_ptr)
	{
		if (!next_ptr_.expired())
		{
			next_ptr_.reset();
		}

		next_ptr_ = brother_ptr;
	}
} // namespace light_wind