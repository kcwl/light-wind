#pragma once
#include <boost/qvm.hpp>

namespace light_wind
{
	namespace render
	{
		struct point
		{
			float x;
			float y;
		};

		struct line
		{
			int x;
			int width;
			int height;
		};

		struct rectangle
		{
			float left;
			float top;
			float right;
			float bottom;
		};

		struct sphere
		{
			float radius;
			float scale;
		};

		struct box
		{
			float length;
			float width;
			float height;
			boost::qvm::vec<float, 3> pos;
		};
	} // namespace render
} // namespace light_wind