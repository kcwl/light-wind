#pragma once
#include <boost/qvm.hpp>
#include <string>
#include <vector>

namespace light_wind
{
	namespace render
	{
		struct mesh3d
		{
			using vertex_t = boost::qvm::vec<float, 3>;
			using face_t = boost::qvm::vec<int32_t, 3>;

			using texture_t = boost::qvm::vec<float, 2>;
			using normal_t = boost::qvm::vec<float, 3>;

			std::vector<vertex_t> vertexs;

			std::vector<texture_t> textures;

			std::vector<normal_t> normals;

			std::vector<face_t> faces;

			std::string material;
		};
	} // namespace render
} // namespace light_wind