#pragma once
#include <boost/pfr.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ranges>
#include <string_view>

using namespace std::string_view_literals;

namespace light_wind
{
	class ui_config
	{
	public:
		ui_config() = default;

	public:
		bool load(const std::string& filename);

		bool save(const std::string& filename);

		void clear();

		void erase_section(const std::string& section);

		void erase_key(const std::string& section, const std::string& key);

	public:
		std::string get_string(const std::string& section, const std::string& key);

		template <typename T>
		std::vector<T> get_integers(const std::string& section, const std::string& key)
		{
			auto str = get_string(section, key);

			std::vector<T> result{};

			if (str.empty())
			{
				return result;
			}

			auto sp_str = std::string_view(str.c_str()) | std::ranges::views::split(","sv);

			std::stringstream ss{};
			for (const auto& s : sp_str)
			{
				ss << std::string_view(s);
				T value;
				ss >> value;

				result.push_back(value);
				ss.str("");
			}

			return result;
		}

		template <typename T>
		T get_integer(const std::string& section, const std::string& key)
		{
			return config_tree_.get<T>(std::format("{}.{}", section, key));
		}

		bool get_bool(const std::string& section, const std::string& key);

		template <typename T>
		T get_struct(const std::string& section)
		{
			constexpr static auto size = boost::pfr::tuple_size<T>::value;

			auto f = [&]<std::size_t... I>(std::index_sequence<I...>)
			{
				return T{ config_tree_.get<boost::pfr::tuple_element_t<I, T>>(
					std::format("{}.{}", std::string(section), std::string(boost::pfr::get_name<I, T>())))... };
			};

			return f(std::make_index_sequence<size>());
		}

		bool set_string(const std::string& section, const std::string& key, const std::string& value);

		template <typename T>
		bool set_integers(const std::string& section, const std::string& key, const std::vector<T>& values)
		{
			try
			{
				for (auto& v : values)
				{
					config_tree_.put(std::format("{}.{}", section, key), v);
				}
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		template <typename T>
		bool set_integer(const std::string& section, const std::string& key, const T& value)
		{
			try
			{
				config_tree_.put(std::format("{}.{}", section, key), value);
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		template <typename T>
		void set_struct(const std::string& section, const T& value)
		{
			constexpr static auto size = boost::pfr::tuple_size<T>::value;

			auto f = [&]<std::size_t... I>(std::index_sequence<I...>)
			{
				(config_tree_.put(std::format("{}.{}", std::string(section), std::string(boost::pfr::get_name<I, T>())),
								  boost::pfr::get<I>(value)),
				 ...);
			};

			return f(std::make_index_sequence<size>());
		}

	private:
		boost::property_tree::ptree config_tree_;
	};
} // namespace light_wind