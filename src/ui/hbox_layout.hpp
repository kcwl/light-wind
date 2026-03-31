#pragma once
#include <ui/basic_layout.hpp>

namespace light_wind
{
	enum class horizontal_alignment { no, left, center, right };

	template <typename T>
	class hbox_layout : public basic_layout<T>
	{
		using base = basic_layout<T>;

	public:
		hbox_layout(std::shared_ptr<T> lay, std::shared_ptr<T> parent)
			: base(lay, parent)
		{}

		virtual ~hbox_layout()
		{
			return this->parent_ptr_->add_layout(this->shared_from_this());
		}

	public:
		virtual void update() override
		{
			int32_t start = this->layout_ptr_->relative().x() + this->space_;

			switch (static_cast<horizontal_alignment>(this->align()))
			{
			case horizontal_alignment::left:
				{
					start = 0;
				}
				break;
			case horizontal_alignment::center:
				{
					auto [width, _] = this->layout_ptr_->get_window_size();

					auto [parent_width, _] = this->parent_ptr_->get_window_size();

					auto parent_center_pt = this->parent_ptr_->relative().x() + parent_width / 2;

					start = parent_center_pt - width / 2;
				}
				break;
			case horizontal_alignment::right:
				{
					auto [parent_width, _] = this->parent_ptr_->get_window_size();

					auto parent_right_pt = this->parent_ptr_->relative().x() + parent_width;

					auto [width, _] = this->layout_ptr_->get_window_size();

					start = parent_width - width;
				}
				break;
			default:
				break;
			}

			this->layout_ptr_->relative().x(start);
		}
	};
} // namespace light_wind