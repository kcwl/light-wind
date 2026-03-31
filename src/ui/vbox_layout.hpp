#pragma once
#include <ui/basic_layout.hpp>

namespace light_wind
{
	enum class vertical_alignment { no, top, center, bottom };

	template <typename T>
	class vbox_layout : public basic_layout<T>, public std::enable_shared_from_this<vbox_layout<T>>
	{
		using base = basic_layout<T>;

	public:
		vbox_layout(std::shared_ptr<T> lay, std::shared_ptr<T> parent)
			: base(lay, parent)
		{}

		virtual ~vbox_layout()
		{
			return this->parent_ptr_->add_layout(this->shared_from_this());
		}

	public:
		virtual void update() override
		{
			int32_t start = this->layout_ptr_->relative().x() + this->space_;

			switch (static_cast<vertical_alignment>(this->align()))
			{
			case vertical_alignment::top:
				{
					start = 0;
				}
				break;
			case vertical_alignment::center:
				{
					auto [_, height] = this->layout_ptr_->get_window_size();

					auto [_, parent_height] = this->parent_ptr_->get_window_size();

					auto parent_center_pt = this->parent_ptr_->relative().y() + parent_height / 2;

					start = parent_center_pt - height / 2;
				}
				break;
			case vertical_alignment::bottom:
				{
					auto [_, parent_height] = this->parent_ptr_->get_window_size();

					auto parent_right_pt = this->parent_ptr_->relative().y() + parent_height;

					auto [_, height] = this->layout_ptr_->get_window_size();

					start = parent_height - height;
				}
				break;
			default:
				break;
			}

			this->layout_ptr_->relative().y(start);
		}
	};
} // namespace light_wind