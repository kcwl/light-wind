#pragma once
#include <boost/qvm.hpp>

namespace light_wind
{
	template <typename T>
	class coordinate2D
	{
	public:
		using value_type = T;

	public:
		coordinate2D() = default;
		~coordinate2D() = default;

	public:
		std::pair<value_type, value_type> get() const
		{
			return std::make_pair(x_, y_);
		}

		void set(value_type x, value_type y)
		{
			x_ = x;
			y_ = y;
		}

		const value_type& x() const
		{
			return x_;
		}

		value_type& x()
		{
			return x_;
		}

		void x(value_type v)
		{
			x_ = v;
		}

		const value_type& y() const
		{
			return y_;
		}

		value_type& y()
		{
			return y_;
		}

		void y(value_type v)
		{
			y_ = v;
		}

		void move(value_type dura_x, value_type dura_y)
		{
			x_ += dura_x;
			y_ += dura_y;
		}

		void move_x(value_type dura)
		{
			x_ += dura;
		}

		void move_y(value_type dura)
		{
			y_ += dura;
		}

	protected:
		value_type x_;

		value_type y_;
	};
} // namespace light_wind