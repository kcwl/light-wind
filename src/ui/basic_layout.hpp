#pragma once
#include <ui/layout_base.h>
#include <memory>

namespace light_wind
{
	template <typename T>
	class basic_layout : public layout_base
	{
	public:
		basic_layout(std::shared_ptr<T> base, std::shared_ptr<T> parent)
			: layout_ptr_(base)
			, parent_ptr_(parent)
			, space_(0)
		{}

		void space(int32_t space)
		{
			space_ = space;
		}

	protected:
		std::weak_ptr<T> layout_ptr_;

		std::weak_ptr<T> parent_ptr_;

		int32_t space_;
	};
} // namespace light_wind