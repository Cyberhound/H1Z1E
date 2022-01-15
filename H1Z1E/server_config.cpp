#include "server_config.h"
#include <json/json.hpp>
#include <spdlog/spdlog.h>


// Dynamic server config
std::string server_props::serialize()
{
	nlohmann::json json = {
		{ "Server IP", this->ip },
		{ "Server Port", this->port }
	};
	return json.dump();
}

void server_props::deserialize(const std::string& json, bool show_warnings)
{
	nlohmann::json config = nlohmann::json::parse(json);

#define safe_json_import(key, type, member) \
	if (config.contains(key)) member = config[key].get<type>(); \
	else if (show_warnings) spdlog::warn("Configuration file is missing key \"{}\" Server may not be configured correctly.", key);

	safe_json_import("Server IP", std::string, this->ip);
	safe_json_import("Server Port", int, this->port);

#undef safe_json_import
}