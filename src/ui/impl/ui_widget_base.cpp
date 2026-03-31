#include <ui/ui_widget_base.h>

namespace light_wind
{
	ui_widget_base::ui_widget_base()
		: style_()
		, caption_()
		, width_()
		, height_()
	{}

	void ui_widget_base::set_caption(const std::string& caption)
	{
		caption_ = caption;
	}

	uint32_t ui_widget_base::style()
	{
		return style_;
	}

	void ui_widget_base::style(uint32_t style)
	{
		style_ = style;
	}

	void ui_widget_base::hide()
	{
		style_ &= ~wnd_visible;
	}

	void ui_widget_base::show()
	{
		style_ |= wnd_visible;
	}
	bool ui_widget_base::visibled()
	{
		return (style_ & wnd_visible);
	}

	void ui_widget_base::enable()
	{
		style_ &= ~wnd_disable;
	}

	void ui_widget_base::disable()
	{
		style_ |= wnd_disable;
	}

	bool ui_widget_base::disabled()
	{
		return ((style_ & wnd_disable) == wnd_disable);
	}

	void ui_widget_base::destroy()
	{
		style_ |= wnd_destroy;
	}

	std::pair<std::size_t, std::size_t> ui_widget_base::get_widget_size() const
	{
		return std::make_pair(width_, height_);
	}
	void ui_widget_base::set_widget_size(std::size_t width, std::size_t height)
	{
		width_ = width;
		height_ = height;
	}

	std::size_t ui_widget_base::width() const
	{
		return width_;
	}
	std::size_t ui_widget_base::height() const
	{
		return height_;
	}
} // namespace light_wind