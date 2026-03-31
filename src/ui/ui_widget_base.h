#pragma once
#include <string>

namespace light_wind
{
	class ui_widget_base
	{
	public:
		constexpr static uint32_t wnd_visible = 0x80000000;
		constexpr static uint32_t wmd_topmost = 0x40000000;
		constexpr static uint32_t wnd_moveable = 0x20000000;
		constexpr static uint32_t wnd_disable = 0x10000000;
		constexpr static uint32_t wnd_destroy = 0x08000000;

	public:
		ui_widget_base();
		virtual ~ui_widget_base() = default;

	public:
		void set_caption(const std::string& caption);

		uint32_t style();

		void style(uint32_t style);

		void hide();
		void show();
		bool visibled();
		void enable();
		void disable();
		bool disabled();
		void destroy();

		void set_widget_size(std::size_t width, std::size_t height);
		std::pair<std::size_t, std::size_t> get_widget_size() const;

		std::size_t width() const;
		std::size_t height() const;

	private:
		uint32_t style_;

		std::string caption_;

		std::size_t width_;
		std::size_t height_;
	};
} // namespace light_wind