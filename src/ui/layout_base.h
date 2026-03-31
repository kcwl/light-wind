#pragma once
#include <cstdint>
#include <ui/detail/rectangle.hpp>

namespace light_wind
{
	struct margin
	{
		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;
	};

	struct padding
	{
		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;
	};

	class layout_base
	{
		using margin_type = rectangle<int>;

		using padding_type = rectangle<int>;

	public:
		void margin(margin_type mt);

		const margin_type& margin() const;

		margin_type& margin();

		void padding(padding_type pt);

		const padding_type& padding() const;

		padding_type& padding();

		void align(int32_t align);

		const int32_t& align() const;

		int32_t& align();

	public:
		virtual void update() = 0;

	private:
		margin_type margin_;
		margin_type padding_;
		int32_t align_;
	};
} // namespace light_wind