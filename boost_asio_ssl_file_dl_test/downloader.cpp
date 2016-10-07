#include "downloader.hpp"
#include "size.hpp"
#include "downloader_impl.hpp"
#include <iostream>
#include <string>
#include <array>
#include <unordered_map>
#include <boost/asio/ssl.hpp>//require OpenSSL

namespace asio_dl_impl {
	namespace asio = boost::asio;
	using asio::ip::tcp;
	using parameters = std::unordered_map<std::string, std::string>;
	template<typename StreamSocketService>
	void connet(asio::basic_socket<tcp, StreamSocketService>& socket, asio::io_service& io_service, const std::string& host, const std::string& service)
	{
		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(host, service);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;

		// Try each endpoint until we successfully establish a connection.
		boost::system::error_code error = asio::error::host_not_found;
		for (; error && end != endpoint_iterator; ++endpoint_iterator) {
			socket.close();
			socket.connect(*endpoint_iterator, error);
		}
	}
	class invaid_response : public std::runtime_error {
	public:
		explicit invaid_response() : invaid_response("") {}
		explicit invaid_response(const std::string& s) : std::runtime_error("invaid_response: " + s) {}
		explicit invaid_response(const char* s) : invaid_response(std::string(s)) {}
	};
	class status_error : public std::runtime_error {
	public:
		explicit status_error(const std::string& s) : std::runtime_error(s) {}
		explicit status_error(const char* s) : std::runtime_error(s) {}
		explicit status_error(unsigned int status_code, const std::string& s)
			: status_error("Response returned with status code:" + std::to_string(status_code) + "(" + s + ").") {}
	};
	struct download_responce {
		std::string http_version;
		std::uint32_t status_code;
		std::string status_message;
		explicit operator bool() { return require_redirect(status_code); }
		bool operator!() { return !require_redirect(status_code); }
		static constexpr bool require_redirect(std::uint32_t status_code) { return (300 <= status_code && status_code <= 303) || (307 <= status_code && status_code <= 308); }
	};
	template<typename SyncWriteStream>
	download_responce check_status(SyncWriteStream& socket, asio::streambuf& response)
	{
		// Read the response status line.
		asio::read_until(socket, response, "\r\n");

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		if ("HTTP/" != http_version.substr(0, 5)) { throw invaid_response(); }
		std::uint32_t status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (200 != status_code && !download_responce::require_redirect(status_code)) throw status_error(status_code, status_message);
		return download_responce{ http_version.substr(5), static_cast<std::uint8_t>(status_code), std::move(status_message) };
	}
	template<typename SyncWriteStream>
	parameters read_header(SyncWriteStream& socket, asio::streambuf& response)
	{
		// Read response headers, which are terminated by a blank line.
		// Note : read_until over-receive from sockect so that we use only one streambuf.
		asio::read_until(socket, response, "\r\n\r\n");

		// Parse response headers.
		std::istream response_stream(&response);
		parameters re;
		for (std::string header; std::getline(response_stream, header) && header != "\r";) {
			const auto pos = header.find_first_of(':');
			const auto value_front = header.find_first_not_of(' ', pos + 1);
			const auto value_last = header.find_last_not_of("\r\n");
			std::string value = header.substr(value_front, value_last - value_front);
			header.erase(pos);
			re.insert(std::make_pair(std::move(header), std::move(value)));
		}
		return re;
	}
	template<typename SyncReadStream>
	void read_body_data(std::ostream& os, SyncReadStream& socket, boost::asio::streambuf& response) {
		boost::system::error_code ec;
		// Read until EOF, writing data to output as we go.
		while (asio::read(socket, response, asio::transfer_at_least(1)/*少なくても1バイト受信する*/, ec)) {
			os << &response;
			if (ec && ec != boost::asio::error::eof) throw boost::system::system_error(ec);
		}
	}
	template<typename SyncReadStream>
	void read_and_decompress_data(std::ostream& os, SyncReadStream& socket, boost::asio::streambuf& response, const asio_dl_impl::parameters& header) {
		if (!header.count("Content-Encoding")) {
			asio_dl_impl::read_body_data(os, socket, response);
		}
		else {
			const auto content_encoding = asio_dl_impl::to_compress_type(header.at("Content-Encoding"));
			if (asio_dl_impl::compress_type::identity == content_encoding || asio_dl_impl::compress_type::unknown == content_encoding) {
				asio_dl_impl::read_body_data(os, socket, response);
			}
			else {
				std::stringstream ss;
				asio_dl_impl::read_body_data(ss, socket, response);
				try {
					asio_dl_impl::decompress(content_encoding, os, ss);
				}
				catch (const compress_type_error&) {
					os << ss.rdbuf();
				}
			}
		}
	}
}

void downloader::redirect(std::ostream & out_file, const asio_dl_impl::parameters & header, const std::string & get_command, const asio_dl_impl::parameters & param) {
	if (!header.count("Location")) throw asio_dl_impl::invaid_response("Cannot redirect.");
	std::string url = header.at("Location");
	const auto front_pos_get_param = get_command.find_first_of('?');
	if (std::string::npos != front_pos_get_param) url += get_command.substr(front_pos_get_param);//extend get param (ex. ?hl=ja&ie=UTF-8#)
	download(out_file, url, param);
}

void downloader::download(std::ostream & out_file, const std::string & url, const asio_dl_impl::parameters & param)
{
	constexpr char split_str[] = "://";
	const auto last_pos_service = url.find(split_str);
	const auto front_pos_server_name = last_pos_service + std_future::size(split_str) - 1;
	const auto front_pos_get_command = url.find_first_of('/', front_pos_server_name);
	const std::string service = url.substr(0, last_pos_service);
	const std::string server_name = url.substr(front_pos_server_name, front_pos_get_command - front_pos_server_name);
	const std::string get_command = url.substr(front_pos_get_command);
	if ("http" == service) {
		download_nossl(out_file, server_name, get_command, param);
	}
	else if ("https" == service) {
		download_ssl(out_file, server_name, get_command, param);
	}
	else {
		throw std::runtime_error("no supported service type:" + service);
	}
}

void downloader::download_ssl(std::ostream & out_file, const std::string & server_name, const std::string & get_command, const asio_dl_impl::parameters & param)
{
	namespace asio = boost::asio;
	using asio::ip::tcp;
	asio::io_service io_service;
	asio::ssl::context context(io_service, asio::ssl::context::sslv23);
	asio::ssl::stream<tcp::socket> ssl_stream(io_service, context);
	asio_dl_impl::certificate(context, cert_file_);
	asio_dl_impl::connet(ssl_stream.lowest_layer(), io_service, server_name, "https");
	ssl_stream.handshake(asio::ssl::stream_base::client);

	//requestを作成する
	asio::streambuf request;
	asio_dl_impl::make_request_header(request, server_name, get_command, param);

	//リクエストの送信
	asio::write(ssl_stream, request);

	//recieve
	asio::streambuf response;
	const bool no_redirect = !asio_dl_impl::check_status(ssl_stream, response);
	const auto header = asio_dl_impl::read_header(ssl_stream, response);
	if (!no_redirect) redirect(out_file, header, get_command, param);
	asio_dl_impl::read_and_decompress_data(out_file, ssl_stream, response, header);
}

void downloader::download_nossl(std::ostream & out_file, const std::string & server_name, const std::string & get_command, const asio_dl_impl::parameters & param)
{
	namespace asio = boost::asio;
	using asio::ip::tcp;
	asio::io_service io_service;

	tcp::socket socket(io_service);
	asio_dl_impl::connet(socket, io_service, server_name, "http");

	asio::streambuf request;
	asio_dl_impl::make_request_header(request, server_name, get_command, param);

	// Send the request.
	asio::write(socket, request);

	// Read the response status line.
	//recieve
	boost::asio::streambuf response;
	const bool no_redirect = !asio_dl_impl::check_status(socket, response);
	const auto header = asio_dl_impl::read_header(socket, response);
	if (!no_redirect) redirect(out_file, header, get_command, param);
	asio_dl_impl::read_and_decompress_data(out_file, socket, response, header);
}
