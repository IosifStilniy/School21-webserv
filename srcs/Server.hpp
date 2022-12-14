#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>
# include <utility>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <fcntl.h>

# include "typedefs.hpp"
# include "Polls.hpp"
# include "RequestCollector.hpp"
# include "Maintainer.hpp"

# include "utils.hpp"

class Server
{
	public:
		typedef std::map<std::string, Location>		locations_type;
		typedef ServerSettings						Settings;
		typedef ServerSettings::hosts_ports_type	hosts_ports_type;

	private:
	
	public:
		std::vector<Settings>	settings;
		ft::key_value_type		host_port;
		int						backlog;

		Polls					polls;

		RequestCollector		req_coll;
		Maintainer				maintainer;

		Server(void);
		~Server();

		void	initialize(std::string const & host_port, int proto_num);
		void	startListen(void);
		void	proceed(int timeout = -1);

	private:
		void		_poll(int timeout = -1);
		void		_giveResponse(Maintainer::response_queue & resp_queue, int socket);
		std::string	_formHeader(Response::header_fields & options, int status);
		void		_formPacket(Response & response, Response::bytes_type & packet);
		void		_sendPacket(Response::bytes_type & packet, Response & response, int socket);
};

#endif
