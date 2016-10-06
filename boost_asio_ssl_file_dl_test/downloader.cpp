#include "downloader.hpp"
#include <iostream>
#include <string>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <boost/asio/ssl.hpp>//require OpenSSL
#include <boost/algorithm/cxx11/none_of.hpp>

#if defined _MSC_VER && !defined(__clang__)
#pragma warning( push )
//C:\lib\boost_1_61_0\boost/iostreams/copy.hpp(128): warning C4244: '引数': 'std::streamsize' から 'int' への変換です。データが失われる可能性があります。
#pragma warning( disable : 4244 )
#endif
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#if defined _MSC_VER && !defined(__clang__)
#pragma warning( pop ) 
#endif

namespace asio_dl_impl {
	namespace asio = boost::asio;
	using asio::ip::tcp;
	using parameters = std::unordered_map<std::string, std::string>;
	template<typename Decompressor>void decompress(std::ostream& os, std::istream& in, Decompressor&& decompressor) {
		using namespace boost::iostreams;
		filtering_streambuf<input> tmp;
		tmp.push(std::forward<Decompressor>(decompressor));
		tmp.push(in);
		boost::iostreams::copy(tmp, os);
	}
	void decompress_deflated(std::ostream& os, std::istream& in) { decompress(os, in, boost::iostreams::zlib_decompressor()); }
	//http://www.boost.org/doc/libs/1_42_0/libs/iostreams/doc/classes/gzip.html
	void decompress_gzip(std::ostream& os, std::istream& in) { decompress(os, in, boost::iostreams::gzip_decompressor()); }
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
	void make_request_header(asio::streambuf& request, const std::string& server_name, const std::string& get_command, const parameters& param)
	{
		using namespace std::string_literals;
		using std::string;
		using boost::algorithm::none_of;
		const string sep = ": "s;
		const string CRLF = "\r\n"s;
		std::array<string, 6> default_keys{ { "Host"s, "User-Agent"s, "Accept"s, "Accept-Language"s, "Accept-Encoding"s, "Connection"s } };
		auto concat_key_value = [&default_keys, &sep, &CRLF, &param](std::size_t i, const string& default_value) {
			return default_keys[i] + sep + ((param.count(default_keys[i])) ? param.at(default_keys[i]) : default_value) + CRLF;
		};

		std::ostream request_stream(&request);
		//minimam header
		request_stream <<
			"GET " + get_command + " HTTP/1.0" + CRLF
			+ default_keys[0] + sep + server_name + CRLF
			+ concat_key_value(1, "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:46.0) Gecko/20100101 Firefox/46.0"s)
			+ concat_key_value(2, "*/*"s)
			+ concat_key_value(3, "ja,en-US;q=0.7,en;q=0.3"s)
			+ concat_key_value(4, "gzip, deflate"s)
			+ concat_key_value(5, "close"s);
		//additional header
		for (auto&& p : param) if (none_of(default_keys, [&p](const string& s) { return p.first == s; })) request_stream << p.first + sep + p.second + CRLF;
		//terminate
		request_stream << CRLF;
	}
	class certificate_error : public std::runtime_error {
	public:
		explicit certificate_error() : certificate_error("") {}
		explicit certificate_error(const std::string& s) : std::runtime_error("certificate_error: " + s) {}
		explicit certificate_error(const char* s) : certificate_error(std::string(s)) {}
	};
	bool verify_certificate_cb(bool preverified, boost::asio::ssl::verify_context& ctx)
	{
		using std::int32_t;
		std::cout << "Function : " << __func__ << " ----------------- Line : " << __LINE__ << std::endl;
		X509_STORE_CTX *cts = ctx.native_handle();
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		std::cout << "CTX ERROR : " << cts->error << std::endl;

		int32_t depth = X509_STORE_CTX_get_error_depth(cts);
		std::cout << "CTX DEPTH : " << depth << std::endl;

		switch (cts->error)
		{
			case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
				throw certificate_error("X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT");
				break;
			case X509_V_ERR_CERT_NOT_YET_VALID:
			case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
				throw certificate_error("Certificate not yet valid!!");
				break;
			case X509_V_ERR_CERT_HAS_EXPIRED:
			case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
				throw certificate_error("Certificate expired..");
				break;
			case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
				std::cerr << "Self signed certificate in chain!!!" << std::endl;
				preverified = true;
				break;
			default:
				break;
		}
		char subject_name[256];
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, static_cast<int>(std::size(subject_name)));
		std::cout
			<< "Verifying " << subject_name << std::endl
			<< "Verification status : " << preverified << std::endl
			<< "Function : " << __func__ << " ----------------- Line : " << __LINE__ << std::endl;
		return preverified;
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
			: status_error("Response returned with status code:" + std::to_string(status_code) + '(' + s + ").") {}
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
	download_responce check_status(SyncWriteStream& socket)
	{
		// Read the response status line.
		asio::streambuf response;
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
	parameters read_header(SyncWriteStream& socket)
	{
		// Read the response headers, which are terminated by a blank line.
		asio::streambuf response;
		asio::read_until(socket, response, "\r\n\r\n");

		// Process the response headers.
		std::istream response_stream(&response);
		parameters re;
		for (std::string header; std::getline(response_stream, header) && header != "\r";) {
			const auto pos = header.find_first_of(':');
			const auto value_front = (' ' == header[pos]) ? pos + 1 : pos;
			std::string value = header.substr(value_front, header.find_last_not_of("\r\n"));
			header.erase(pos);
			re.insert(std::make_pair(std::move(header), std::move(value)));
		}
		return re;
	}
	void certificate(asio::ssl::context& context, const boost::optional<std::string>& cert_file)
	{
		if (cert_file) {
			context.load_verify_file(*cert_file);
			boost::system::error_code ec;
			context.set_default_verify_paths(ec);
		}
		else {
			context.set_default_verify_paths();
		}
		context.set_verify_mode(asio::ssl::verify_peer);
		context.set_verify_callback([](bool preverified, asio::ssl::verify_context& ctx) { return verify_certificate_cb(preverified, ctx); });
	}
}

void downloader::redirect(std::ostream & out_file, const asio_dl_impl::parameters & header, const std::string & get_command, const asio_dl_impl::parameters & param) {
	if (!header.count("Location")) throw asio_dl_impl::invaid_response("Cannot redirect.");
	std::string service = header.at("Location");
	const char split_str[] = "://";
	const auto pos = service.find(split_str);
	std::string server_name = service.substr(pos + std::size(split_str));
	service.erase(pos);
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
	bool no_redirect = !asio_dl_impl::check_status(ssl_stream);
	const auto header = asio_dl_impl::read_header(ssl_stream);
	if (!no_redirect) redirect(out_file, header, get_command, param);

	// Read until EOF, writing data to output as we go.
	asio::streambuf response;
	while (asio::read(ssl_stream, response, asio::transfer_at_least(1)/*少なくても1バイト受信する*/)) {
		out_file << &response;
	}
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
	bool no_redirect = !asio_dl_impl::check_status(socket);
	const auto header = asio_dl_impl::read_header(socket);
	if (!no_redirect) redirect(out_file, header, get_command, param);

	// Read until EOF, writing data to output as we go.
	boost::asio::streambuf response;
	while (asio::read(socket, response, asio::transfer_at_least(1))) {
		out_file << &response;
	}
}
