#include "Server.hpp"

Server::Server(void)
	: maintainer(this->settings)
{
}

Server::~Server()
{
}

void	Server::initialize(std::string const & host_port, int proto_num)
{
	this->host_port = ft::splitHeader(host_port, ":");

	this->backlog = 0;
	for (std::vector<Settings>::iterator set_start = this->settings.begin(); set_start != this->settings.end(); set_start++)
	{
		for (hosts_ports_type::const_iterator hp_start = set_start->host_port.begin(); hp_start != set_start->host_port.end(); hp_start++)
			if (hp_start->first == this->host_port)
				this->backlog += hp_start->second;

		set_start->host_port.clear();
	}
	
	if (this->backlog < 1)
		this->backlog = std::numeric_limits<int>::max();

	int			reuse_option = 1;
	sockaddr_in	server_addr;

	this->polls.append(socket(PF_INET, SOCK_STREAM, proto_num), POLLIN);
	setsockopt(this->polls.getListenSocket(), SOL_SOCKET, SO_REUSEADDR, &reuse_option, sizeof(reuse_option));
	fcntl(this->polls.getListenSocket(), F_SETFL, O_NONBLOCK);

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_addr.s_addr = inet_addr(this->host_port.first.c_str());
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(strtol(this->host_port.second.c_str(), NULL, 10));

	if (bind(this->polls.getListenSocket(), reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)))
		throw std::runtime_error("bind: socket " + ft::num_to_string(this->polls.getListenSocket()) + ": " + strerror(errno));
}

void	Server::startListen(void)
{
	if (listen(this->polls.getListenSocket(), this->backlog))
		throw std::runtime_error("listen: socket " + ft::num_to_string(this->polls.getListenSocket()) + strerror(errno));
}

void	Server::_poll(int timeout)
{
	this->polls.poll(timeout);

	std::vector<int>	bad_sockets = this->polls.clear();

	for (std::vector<int>::const_iterator socket = bad_sockets.begin(); socket != bad_sockets.end(); socket++)
	{
		this->req_coll.erase(*socket);
		this->maintainer.erase(*socket);
	}
}

void	Server::proceed(int timeout)
{
	int	socket;

	this->maintainer.proceedRequests(req_coll);

	this->_poll(timeout);

	socket = this->polls.getNextSocket();
	if (polls.isListenSocket(socket))
	{
		this->polls.append(accept(this->polls.getListenSocket(), NULL, NULL), POLLIN | POLLOUT);
		socket = this->polls.getNextSocket();
	}

	while (socket)
	{
		// std::cout << "socket " << socket << " " << this->polls[socket]->revents << std::endl;
		if ((this->polls[socket]->revents & POLLIN) == POLLIN)
			this->req_coll.collect(socket);
		
		// if (!this->req_coll[socket].empty())
		// 	this->req_coll[socket].front().printOptions(this->req_coll[socket].front().options);
		
		if ((this->polls[socket]->revents & POLLOUT) == POLLOUT)
			this->resp_handler.giveResponse(this->maintainer[socket], socket);

		socket = this->polls.getNextSocket();
	}
}
