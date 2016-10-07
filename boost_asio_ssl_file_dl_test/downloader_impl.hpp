#pragma once
#if !defined( WINVER ) || ( WINVER < 0x0501 )
#undef  WINVER
#define WINVER 0x0501
#endif
#if !defined( _WIN32_WINNT ) || ( _WIN32_WINNT < 0x0501 )
#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#if defined _MSC_VER && !defined(__clang__)
#pragma warning( push )
//C:\lib\boost_1_61_0\boost/iostreams/copy.hpp(128): warning C4244: '引数': 'std::streamsize' から 'int' への変換です。データが失われる可能性があります。
#pragma warning( disable : 4244 )
#endif
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>//require OpenSSL
#include <boost/optional.hpp>

namespace asio_dl_impl {
	namespace asio = boost::asio;
	using asio::ip::tcp;
	using parameters = std::unordered_map<std::string, std::string>;
	enum class compress_type : std::uint8_t {
		unknown,
		identity,
		deflated,
		gzip,
		bzip2
	};
	class compress_type_error : public std::runtime_error {
	public:
		explicit compress_type_error();
		explicit compress_type_error(const std::string& s);
		explicit compress_type_error(const char* s);
	};
	void decompress(compress_type type, std::ostream & os, std::istream & in);
	compress_type to_compress_type(const std::string & s);
	void make_request_header(asio::streambuf & request, const std::string & server_name, const std::string & get_command, const parameters & param);
	bool verify_certificate_cb(bool preverified, boost::asio::ssl::verify_context & ctx);
	void certificate(asio::ssl::context & context, const boost::optional<std::string>& cert_file);
	class certificate_error : public std::runtime_error {
	public:
		explicit certificate_error();
		explicit certificate_error(const std::string& s);
		explicit certificate_error(const char* s);
	};

}
