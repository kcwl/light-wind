#include <ui/ui_widget.h>

namespace light_wind
{
	ui_widget::ui_widget()
		: moving_(false)
		, last_mouse_pos_x_(0)
		, last_mouse_hold_pos_y_(0)
		, cursor_style_()
	{}

	ui_widget::~ui_widget()
	{
		leave_alone();
	}

	void ui_widget::paint()
	{
		if (!visibled())
		{
			return;
		}

		auto child_ptr = std::dynamic_pointer_cast<ui_widget>(first_child());
		while (child_ptr)
		{
			child_ptr->paint_widget();

			child_ptr.reset();
			child_ptr = std::dynamic_pointer_cast<ui_widget>(child_ptr->next_brother());
		}
	}

	void ui_widget::paint_widget()
	{
		return;
	}

	void ui_widget::on_focus_gained()
	{
		bring_to_top();
	}

	bool ui_widget::contain(std::size_t x, std::size_t y)
	{
		auto [width, height] = get_widget_size();

		return x >= absolute().x() && x <= absolute().x() + width && y >= absolute().y() &&
			   y <= absolute().y() + height;
	}

	std::shared_ptr<ui_widget> ui_widget::top_node_from_point(std::size_t x, std::size_t y)
	{
		while (auto w = std::dynamic_pointer_cast<ui_widget>(end_child()))
		{
			if (w->contain(x, y) && w->visibled())
			{
				return w;
			}
		}
		return nullptr;
	}

	void ui_widget::set_render_layer(int32_t layer)
	{
		render_layer_ = layer;
	}

	int32_t ui_widget::get_render_layer() const
	{
		return render_layer_;
	}

	void ui_widget::set_cursor(int nCursor)
	{
		cursor_style_ = nCursor;
	}

	int ui_widget::get_cursor()
	{
		return cursor_style_;
	}
} // namespace light_wind