#include <hwio/hwio_cli.h>
#include <hwio/hwio_server.h>
#include <hwio/hwio_remote_utils.h>
#include <iostream>
#include <csignal>
#include <memory>

using namespace hwio;

const char * DEFAULT_ADDR = "0.0.0.0:8896";

static bool run_server_flag = true;
HwioServer::loglevel_e logLevel = HwioServer::loglevel_e::logWARNING;

void print_hwio_server_help() {
	std::cout << "## hwio_server ##" << std::endl;
	std::cout << "-a <ip:port> or --address <ip:port> where to run server default "
			  << DEFAULT_ADDR << std::endl;
	std::cout << "--log <level>  logERROR=0, logWARNING=1, logINFO=2, logDEBUG=3" << std::endl;
	std::cout << "--help to show this help msg" << std::endl;
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


	auto bus = hwio_init(argc, argv);
	if (bus == nullptr) {
		throw std::runtime_error("Can not initialize HWIO");
	}
	std::unique_ptr<ihwio_bus> _bus(bus);

	const option long_opts[] = { //
			{ "address", no_argument, nullptr, 'a' },     //
			{ "help", required_argument, nullptr, 'h' },  //
			{ "log", required_argument, nullptr, 'l' },   //
			{ nullptr, no_argument, nullptr, 0 }          //
	};

	const char * server_address = DEFAULT_ADDR;

	while (true) {
		const auto opt = getopt_long(argc, argv, "a:h", long_opts, nullptr);
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

	server.prepare_server_socket();
	server.handle_client_msgs(&run_server_flag);

}
