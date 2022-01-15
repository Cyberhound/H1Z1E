#pragma once
#include <stdint.h>
#include <string>

// Static server config
#define SERVER_DEFAULT_CONFIG_FILE "server_properties.json"

// Default config
#define SERVER_DEFAULT_IP "127.0.0.1"
#define SERVER_DEFAULT_PORT 1905


// Dynamic server config
struct server_props
{
public:
	std::string ip;
	uint16_t port;

	void deserialize(const std::string& json, bool show_warnings = false);
	std::string serialize();
};