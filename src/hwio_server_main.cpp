#include <hwio/hwio_cli.h>
#include <hwio/hwio_server.h>
#include <hwio/hwio_remote_utils.h>
#include <iostream>
#include <csignal>
#include <memory>
#include <vector>
#include <string>
#include <dlfcn.h>

using namespace hwio;

typedef void (*plugin_init_t)(HwioServer*);

static bool run_server_flag = true;
HwioServer::loglevel_e logLevel = HwioServer::loglevel_e::logWARNING;

void print_hwio_server_help() {
	std::cout << "## hwio_server ##" << std::endl;
	std::cout << "-a <ip:port> or --address <ip:port> where to run server default "
			  << HwioServer::DEFAULT_ADDR << std::endl;
	std::cout << "--log <level>  logERROR=0, logWARNING=1, logINFO=2, logDEBUG=3" << std::endl;
	std::cout << "--help to show this help msg" << std::endl;
	std::cout << "-p <plugin> or --plugin <plugin> to load hwio-server RPC plugin. May be used repetitively." << std::endl;
	std::cout << hwio_help_str();
}

void signal_handler( __attribute__((unused)) int signal) {
	if (logLevel >= HwioServer::loglevel_e::logINFO)
		std::cout << "SIGTERM recieved" << std::endl;
	run_server_flag = false;
}

void init_signal_handler() {
	sigset_t mask;
	sigset_t orig_mask;
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = signal_handler;

	// This server should shut down on SIGTERM.
	if (sigaction(SIGTERM, &act, 0)) {
		perror("sigaction");
		exit(1);
	}

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);

	if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
		perror("sigprocmask");
		exit(1);
	}
}


int main(int argc, char * argv[]) {
	init_signal_handler();
	std::vector<std::string> plugins;
	std::vector<void*> plugin_handles;

	auto bus = hwio_init(argc, argv);
	if (bus == nullptr) {
		throw std::runtime_error("Can not initialize HWIO");
	}
	std::unique_ptr<ihwio_bus> _bus(bus);

	const option long_opts[] = { //
			{ "address", no_argument, nullptr, 'a' },     //
			{ "help", required_argument, nullptr, 'h' },  //
			{ "log", required_argument, nullptr, 'l' },   //
			{ "plugin", required_argument, nullptr, 'p' },   //
			{ nullptr, no_argument, nullptr, 0 }          //
	};

	const char * server_address = HwioServer::DEFAULT_ADDR;

	while (true) {
		const auto opt = getopt_long(argc, argv, "a:hl:p:", long_opts, nullptr);
		if (-1 == opt)
			break;

		switch (opt) {
		case 'h':
			print_hwio_server_help();
			return 0;

		case 'a':
			server_address = optarg;
			break;
		case 'l':
			logLevel = (HwioServer::loglevel_e) std::stoi(optarg);
			break;
		case 'p':
			plugins.push_back(optarg);
			break;
		case '?':
		default:
			std::cerr << "Unknown CLI argument" << std::endl;
			print_hwio_server_help();
			exit(1);
			break;
		}
	}

	std::cout << "[INFO] Running hwio_server (build" << __TIMESTAMP__<< ") on "
			<< server_address << " ..." << std::endl;

	std::string server_addr_str(server_address);
	struct addrinfo * addr = parse_ip_and_port(server_addr_str);
	HwioServer server(addr, {bus});
	server.log_level = logLevel;
	for(const auto& plugin : plugins) {
		void *handle;
		plugin_init_t hwio_server_on_load;
                
		handle = dlopen(plugin.c_str(), RTLD_NOW | RTLD_LOCAL);
		if (handle == nullptr) {
			std::cerr << "[ERROR] Can not load plugin: " << plugin << " Error: " << dlerror() << std::endl;
			continue;
		}
		hwio_server_on_load = (plugin_init_t) (uintptr_t) dlsym(handle, "hwio_server_on_load");
		if (hwio_server_on_load == nullptr) {
			std::cerr << "[ERROR] Can not find function hwio_server_on_load() in plugin: " << plugin << " Error: " << dlerror() << std::endl;
			dlclose(handle);
			continue;
		}
		(*hwio_server_on_load)(&server);
		plugin_handles.push_back(handle);
	}
	server.prepare_server_socket();
	while(run_server_flag) {
		server.pool_client_msgs();
	}
	for(const auto handle : plugin_handles) {
		dlclose(handle);
	}
}
