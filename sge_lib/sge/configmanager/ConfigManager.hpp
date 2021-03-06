#ifndef SGE_CONFIG_MANAGER_HPP
#define SGE_CONFIG_MANAGER_HPP

#include <tinyxml2.h>
#include <easylogging++.h>
#include <string>
#include <vector>

#include <sge/utils/utils.hpp>

namespace SGE
{
	class ConfigManager
	{
	private:
		static std::string filePath;
		static std::map<std::string, std::string> values;
		
		static void recursiveParse(std::string currentNodeName, const tinyxml2::XMLElement* n);
	public:
		Export static void setConfigFile(std::string file);
		Export static bool init();
		Export static std::string get(std::string prop);
	};
}

#endif
