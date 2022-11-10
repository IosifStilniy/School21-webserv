#include "Maintainer.hpp"

static std::vector<std::string>	methodsNameInit(void)
{
	std::vector<std::string>	methods;

	methods.push_back("get");
	methods.push_back("post");
	methods.push_back("delete");

	return (methods);
}

const std::vector<std::string> Maintainer::_methods_names = methodsNameInit();

Maintainer::Maintainer(std::vector<ServerSettings> & settings)
	: _settings(settings)
{
	this->_methods[0] = &Maintainer::_get;
	this->_methods[1] = &Maintainer::_post;
	this->_methods[2] = &Maintainer::_delete;
}

Maintainer::~Maintainer()
{
}

void	Maintainer::_get(Request & request, Response & response)
{
	response.readFile();
	
	if (response.status)
		return ;

	if (!response.polls.isGood(response.in) && response.chunks.front().empty())
	{
		response.badResponse(404);
		return ;
	}

	if (response.polls.isGood(response.in))
	{
		if (request.options["TE"].find("chunked") == request.options["TE"].end())
			return ;

		if (!response.options["Transfer-Encoding"].empty())
			response.options["Transfer-Encoding"].append(",");
		response.options["Transfer-Encoding"].append("chunked");
		response.trans_mode = Response::tChunked;
		response.status = 200;
		return ;
	}

	size_t	length = 0;
	
	for (Response::chunks_type::const_iterator chunk = response.chunks.begin(); chunk != response.chunks.end(); chunk++)
		length += chunk->size();

	response.options["Content-Length"] = ft::num_to_string(length);
	response.status = 200;
}

void	Maintainer::_post(request_type & request, Response & response)
{
	response.writeFile(request.chunks);

	if (response.status)
		return ;
	
	if (response.polls.isGood(response.out))
		return ;

	if ((!request.chunks.empty() || request.chunks.front().empty()))
	{
		response.badResponse(500);
		return ;
	}

	close(response.out);
	response.out = -1;

	response.status = 201;
}

void	Maintainer::_delete(request_type & request, Response & response)
{
	static_cast<void>(request);

	if (std::remove(response.mounted_path.c_str()))
	{
		response.badResponse(500);
		return ;
	}

	response.status = 200;
}

void	Maintainer::_dispatchRequest(request_type & request, Response & response)
{
	if (!response.inited)
		response.init(request, this->_settings);

	size_t	i = 0;

	for (; i < Maintainer::_methods_names.size(); i++)
		if (Maintainer::_methods_names[i] == ft::toLower(request.getOnlyValue("method")))
			break ;

	PTR_FUNC(i)(request, response);
}

Maintainer::response_queue &	Maintainer::operator[](int socket)
{
	return (this->_sockets[socket]);
}

void	Maintainer::proceedRequests(RequestCollector & requests)
{
	for (RequestCollector::iterator	start = requests.begin(); start != requests.end(); start++)
	{
		const int &			socket = start->first;
		request_queue &		req_queue = start->second;

		if (req_queue.empty() || !req_queue.front().is_ready)
			continue ;

		response_queue &	responses = this->_sockets[socket];

		if (responses.empty())
			responses.push(Response());

		this->_dispatchRequest(req_queue.front(), responses.back());

		if (!responses.back().status)
			continue ;

		responses.push(Response());
		req_queue.pop();
	}
}

void	Maintainer::erase(const int socket)
{
	this->_sockets.erase(socket);
}

Maintainer::iterator	Maintainer::begin()
{
	return (this->_sockets.begin());
}

Maintainer::const_iterator	Maintainer::begin()	const
{
	return (this->_sockets.begin());
}

Maintainer::iterator	Maintainer::end()
{
	return (this->_sockets.end());
}

Maintainer::const_iterator	Maintainer::end()	const
{
	return (this->_sockets.end());
}
