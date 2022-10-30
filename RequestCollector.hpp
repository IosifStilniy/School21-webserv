#ifndef REQUESTCOLLECTOR_CPP
# define REQUESTCOLLECTOR_CPP

# include <string>
# include <sys/socket.h>
# include <vector>
# include <map>
# include <queue>
# include <stdexcept>
# include <algorithm>

# include "utils.hpp"

# ifndef KB
#  define KB(x) (x * 1024)
# endif

# ifndef MB
#  define MB(x) (KB(x) * 1024)
# endif

# ifndef BUFSIZE
#  define BUFSIZE KB(4)
# endif

# ifndef HTTP_EOF
#  define HTTP_EOF "\r\n\r\n"
# endif

class RequestCollector
{
	public:
		typedef char								byte_type;
		typedef	std::vector<byte_type>				bytes_type;
		typedef std::queue<bytes_type>				chunks_type;
		typedef bytes_type::const_iterator			bytes_iterator;
		typedef std::map<std::string, std::string>	header_fields;

		struct Request
		{
			chunks_type		chunks;
			header_fields	options;
			size_t			content_length;
			std::string		transfer_encoding;
			bool			is_ready;

			Request(void);

			void	parseHeader(void);
			bool	isFullyReceived(void);
		};

		typedef std::queue<Request>				request_queue;
		typedef	std::map<int, request_queue>	socket_map;

		typedef socket_map::iterator			iterator;
		typedef socket_map::const_iterator		const_iterator;

	private:
		static const std::string	_eof;

		socket_map	_sockets;
		byte_type	_buf[BUFSIZE];
		const std::string &	_ref_eof;

		template <typename Iterator>
		Iterator	_getEOF(Iterator begin, Iterator end)
		{
			return (std::search(begin, end, this->_ref_eof.begin(), this->_ref_eof.end()));
		};

		byte_type *	_splitIncomingStream(Request & request, byte_type * msg_start, byte_type * msg_end);
		bool		_isSplitedEOF(bytes_type & chunk, byte_type * & msg_start, byte_type * msg_end);
		bool		_transferEnded(byte_type * & msg_start, size_t dstnc);
		byte_type *	_chunkedTransferHandler(Request & request, byte_type * msg_start, byte_type * msg_end);

	public:
		RequestCollector(void);
		~RequestCollector();

		Request &	operator[](int socket);

		void	collect(int socket);

		iterator		begin();
		const_iterator	begin()	const;
		iterator		end();
		const_iterator	end()	const;
};

#endif
