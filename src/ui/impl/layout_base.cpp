#include <ui/layout_base.h>

namespace light_wind
{
	void layout_base::margin(margin_type mt)
	{
		margin_ = mt;
	}

	const layout_base::margin_type& layout_base::margin() const
	{
		return margin_;
	}

	layout_base::margin_type& layout_base::margin()
	{
		return margin_;
	}

	void layout_base::padding(padding_type pt)
	{
		padding_ = pt;
	}

	const layout_base::padding_type& layout_base::padding() const
	{
		return padding_;
	}

	layout_base::padding_type& layout_base::padding()
	{
		return padding_;
	}

	void layout_base::align(int32_t align)
	{
		align_ = align;
	}

	const int32_t& layout_base::align() const
	{
		return align_;
	}

	int32_t& layout_base::align()
	{
		return align_;
	}
} // namespace light_wind