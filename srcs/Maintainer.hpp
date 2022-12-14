#ifndef MAINTAINER_HPP
# define MAINTAINER_HPP

# include <vector>
# include <map>
# include <string>
# include <queue>
# include <fstream>
# include <cstdio>

# include "utils.hpp"

# include "Response.hpp"
# include "RequestCollector.hpp"
# include "exceptions.hpp"

# define PTR_FUNC(i) ((this->*(this->_methods[i])))

class Maintainer
{
	public:
		typedef	RequestCollector::request_queue		request_queue;
		typedef std::queue<Response>				response_queue;
		typedef	std::map<int, response_queue>		socket_map;
		typedef socket_map::iterator				iterator;
		typedef socket_map::const_iterator			const_iterator;

	private:
		socket_map						_sockets;
		std::vector<ServerSettings> &	_settings;

		Maintainer(void);

		static const std::vector<std::string>	_methods_names;

		void	_dispatchRequest(Request & request, Response & response);
		
		void	_head(Request & request, Response & response);
		void	_get(Request & request, Response & response);
		void	_put(Request & request, Response & response);
		void	_post(Request & request, Response & response);
		void	_delete(Request & request, Response & response);

		void	(Maintainer::*_methods[5])(Request & request, Response & response);

	public:
		Maintainer(std::vector<ServerSettings> & settings);
		~Maintainer();

		response_queue &	operator[](int socket);

		void	proceedRequests(RequestCollector & requests);
		void	erase(const int socket);

		iterator		begin();
		const_iterator	begin()	const;
		iterator		end();
		const_iterator	end()	const;
};

#endif
