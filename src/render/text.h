#pragma once
#include <detail/string.hpp>

namespace light_wind
{
	namespace render
	{
		struct text
		{
			string font;
			size_t font_size;
			string content;
			rectangle pos;
		};
	}
}