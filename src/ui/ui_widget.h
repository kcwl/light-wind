#pragma once
#include <ui/basic_layout.hpp>
#include <ui/ui_config.h>
#include <ui/ui_node.h>
#include <ui/ui_widget_base.h>

namespace light_wind
{
	class ui_widget : public ui_node, public ui_widget_base
	{
	public:
		ui_widget();
		virtual ~ui_widget();

	public:
		virtual void update_data()
		{}

		virtual int load(ui_config* ini_file_ptr, const std::string& section)
		{
			return 0;
		}

		virtual int close_window(bool destory)
		{
			return 0;
		};

		virtual void breathe()
		{}

		void set_cursor(int nCursor);
		int get_cursor();

		virtual void paint();

		virtual void paint_widget();

		virtual void on_focus_gained();

		virtual void on_focus_lost()
		{}

		bool contain(std::size_t x, std::size_t y);

		std::shared_ptr<ui_widget> top_node_from_point(std::size_t x, std::size_t y);

		void set_render_layer(int32_t layer);

		int32_t get_render_layer() const;

		void add_layout(std::shared_ptr<basic_layout<ui_widget>> layout)
		{
			layouts_.push_back(layout);
		}

		void update_layout()
		{
			for (auto& layout : layouts_)
			{
				if (!layout)
				{
					continue;
				}

				layout->update();
			}
		}

	protected:
		int cursor_style_;

		bool moving_;
		int last_mouse_pos_x_;
		int last_mouse_hold_pos_y_;

		int32_t render_layer_;

		std::vector<std::shared_ptr<basic_layout<ui_widget>>> layouts_;
	};
} // namespace light_wind