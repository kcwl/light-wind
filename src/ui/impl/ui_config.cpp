#include <ui/ui_config.h>

namespace light_wind
{
	bool ui_config::load(const std::string& filename)
	{
		try
		{
			boost::property_tree::read_ini(filename, config_tree_);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	bool ui_config::save(const std::string& filename)
	{
		try
		{
			boost::property_tree::write_ini(filename, config_tree_);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	void ui_config::clear()
	{
		return;
	}

	void ui_config::erase_section(const std::string& section)
	{
		config_tree_.erase(section);
	}

	void ui_config::erase_key(const std::string& section, const std::string& key)
	{
		config_tree_.erase(std::format("{}.{}", section, key));
	}

	std::string ui_config::get_string(const std::string& section, const std::string& key)
	{
		return config_tree_.get<std::string>(std::format("{}.{}", section, key));
	}

	bool ui_config::get_bool(const std::string& section, const std::string& key)
	{
		return config_tree_.get<bool>(std::format("{}.{}", section, key));
	}

	bool ui_config::set_string(const std::string& section, const std::string& key, const std::string& value)
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
} // namespace light_wind