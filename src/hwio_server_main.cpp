#include <hwio/hwio_cli.h>
#include <hwio/hwio_server.h>
#include <hwio/hwio_remote_utils.h>
#include <iostream>
#include <csignal>

using namespace hwio;

const char * DEFAULT_ADDR = "0.0.0.0:8896";

static bool run_server_flag = true;

void print_hwio_server_help() {
	std::cout << "## hwio_server ##" << std::endl;
	std::cout << "-a <ip:port> or --address <ip:port> where to run server default "
			  << DEFAULT_ADDR << std::endl;
	std::cout << hwio_help_str();
}

void signal_handler( __attribute__((unused)) int signal) {
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

	const option long_opts[] = { //
			{ "address", no_argument, nullptr, 'a' },     //
			{ "help", required_argument, nullptr, 'h' },  //
			{ nullptr, no_argument, nullptr, 0 }          //
	};

	const char * server_address = DEFAULT_ADDR;

	while (true) {
		const auto opt = getopt_long(argc, argv, "a:h", long_opts, nullptr);
		if (-1 == opt)
			break;

		switch (opt) {
		case 'h':
			delete bus;
			print_hwio_server_help();
			break;

		case 'a':
			server_address = optarg;
			break;

		case '?':
		default:
			std::cerr << "Unknown CLI argument" << std::endl;
			delete bus;
			print_hwio_server_help();
			exit(0);
			break;
		}
	}

	std::cout << "[INFO] Running hwio_server on " << server_address << " ..." << std::endl;

	std::string server_addr_str(server_address);
	struct addrinfo * addr = parse_ip_and_port(server_addr_str);
	Hwio_server server(addr, {bus});

	server.prepare_server_socket();
	server.handle_client_msgs(&run_server_flag);

}
