#pragma once
#include <ui/detail/coordinate.hpp>

namespace light_wind
{
	template <typename T>
	class position
	{
	public:
		using position_type = coordinate2D<T>;

	public:
		position()
			: relative_()
			, absolute_()
		{}

		virtual ~position() = default;

	public:
		const position_type& relative() const
		{
			return relative_;
		}

		position_type& relative()
		{
			return relative_;
		}

		void relative(int left, int top)
		{
			absolute_.move(left - relative_.x(), top - relative_.y());

			relative_.set(left, top);
		}

		const position_type& absolute() const
		{
			return absolute_;
		}

		position_type& absolute()
		{
			return absolute_;
		}

		void absolute(int left, int top)
		{
			auto dura_x = left - absolute_.x();
			auto dura_y = top - absolute_.y();

			absolute_.move(dura_x, dura_y);
			relative_.move(dura_x, dura_y);
		}

	protected:
		position_type relative_;

		position_type absolute_;
	};
} // namespace light_wind