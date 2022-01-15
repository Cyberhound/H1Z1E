/*
 __    __    __    ________                                                   
/  |  /  | _/  |  /        |                                                  
$$ |  $$ |/ $$ |  $$$$$$$$/   ______    ______   __     __  ______    ______  
$$ |__$$ |$$$$ |      /$$/   /      \  /      \ /  \   /  |/      \  /      \ 
$$    $$ |  $$ |     /$$/   /$$$$$$  |/$$$$$$  |$$  \ /$$//$$$$$$  |/$$$$$$  |
$$$$$$$$ |  $$ |    /$$/    $$    $$ |$$ |  $$/  $$  /$$/ $$    $$ |$$ |  $$/ 
$$ |  $$ | _$$ |_  /$$/____ $$$$$$$$/ $$ |        $$ $$/  $$$$$$$$/ $$ |      
$$ |  $$ |/ $$   |/$$      |$$       |$$ |         $$$/   $$       |$$ |      
$$/   $$/ $$$$$$/ $$$$$$$$/  $$$$$$$/ $$/           $/     $$$$$$$/ $$/       
                                                                              
                                                                              
Developed by Cyberhound#5672 on H1EMU Discord.
	Thanks to Chris, he has helped me as well
	as well as almost the entire community.
	Specially Meme, Kentin, LegendsNeverDie, and
	last few but definitely not least: Rob, and
	HaxMax.
*/


#include <stdint.h>
#include <spdlog/spdlog.h>
#include <json/json.hpp>

#include <framework/io/file.h>
#include <framework/network/net_connection.h>

#include <globals.h>

#include "server_config.h"



bool console_yes_or_no(const char *question, bool default_yes)
{
	std::string input = "";
	printf("%s (%s): ", question, default_yes ? "Y/n" : "y/N");
	while (true)
	{
		std::getline(std::cin, input);
		if (input.size() > 1)
		{
			spdlog::error("Too many characters, please enter y/n/Y/N");
			continue;
		}
		break;
	}

	char r = std::tolower(input[0]);
	if (r == '\n') return default_yes;
	else if (r == 'y')
		return 1;
	else if (r == 'n')
		return 0;
	else
	{
		spdlog::error("Invalid option given \"{}\" ({}), quitting...", r, (int)r);
		Sleep(1500);
		exit(0);
	}
	return default_yes;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32 or _WIN64
	SetConsoleTitle("H1Zerver - Cyberhound Edition");
#endif
	printf(" __    __    __    ________                                                   \n");
	printf("/  |  /  | _/  |  /        |                                                  \n");
	printf("$$ |  $$ |/ $$ |  $$$$$$$$/   ______    ______   __     __  ______    ______  \n");
	printf("$$ |__$$ |$$$$ |      /$$/   /      \\  /      \\ /  \\   /  |/      \\  /      \\ \n");
	printf("$$    $$ |  $$ |     /$$/   /$$$$$$  |/$$$$$$  |$$  \\ /$$//$$$$$$  |/$$$$$$  |\n");
	printf("$$$$$$$$ |  $$ |    /$$/    $$    $$ |$$ |  $$/  $$  /$$/ $$    $$ |$$ |  $$/ \n");
	printf("$$ |  $$ | _$$ |_  /$$/____ $$$$$$$$/ $$ |        $$ $$/  $$$$$$$$/ $$ |      \n");
	printf("$$ |  $$ |/ $$   |/$$      |$$       |$$ |         $$$/   $$       |$$ |      \n");
	printf("$$/   $$/ $$$$$$/ $$$$$$$$/  $$$$$$$/ $$/           $/     $$$$$$$/ $$/       \n\n\n");

	// Working directory: Application's "local path" (launch path).
	working_dir = local_path();

	// Get configuration file, we need this before even beginning to think about the server.
	if (!file_exists(working_dir + "\\" SERVER_DEFAULT_CONFIG_FILE))
	{
		std::string input = "";

		spdlog::critical("No default configuration file found. Missing file \"{}\"", working_dir + "\\" SERVER_DEFAULT_CONFIG_FILE);
		if (console_yes_or_no("Do you want to create a configuration file (y for defaults, n for custom)?", true))
		{
			srv_config.ip = SERVER_DEFAULT_IP;
			srv_config.port = SERVER_DEFAULT_PORT;

		}
		else if (console_yes_or_no("Do you want to create a custom configuration?", true))
		{
			printf("Server IP: ");
			std::getline(std::cin, input);
			if (input.empty())
				srv_config.ip = "127.0.0.1";
			else
				srv_config.ip = input;

			printf("Server Port: ");
			std::getline(std::cin, input);
			srv_config.port = std::stoi(input);
		}
		else
			return 1;

		write_file(working_dir + "\\" SERVER_DEFAULT_CONFIG_FILE, srv_config.serialize());
		system("cls");
		spdlog::info("Config saved. Program restarted.");
		return main(argc, argv);
	}

	spdlog::info("Loading config file...\n");
	
	// Load config from file
	srv_config.deserialize(read_file(working_dir + "\\" SERVER_DEFAULT_CONFIG_FILE), true);
	
	// Just information, the server IP and port if ever needed to be known.
	spdlog::info("Server IP: {}", srv_config.ip);
	spdlog::info("Server Port: {}", srv_config.port);


	return 0;
}