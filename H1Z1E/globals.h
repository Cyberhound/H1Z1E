#pragma once
#include <string>
#include <iostream>
#include <server_config.h>

inline std::string working_dir = "";

// Just a basic console function so you may see the errors instead of the console immediately closing.
inline void halt()
{
	printf("Press ENTER to continue . . .\n");
	std::cin.get();
}


inline server_props srv_config;