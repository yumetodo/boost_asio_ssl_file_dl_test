#pragma once
#if !defined( WINVER ) || ( WINVER < 0x0501 )
#undef  WINVER
#define WINVER 0x0501
#endif
#if !defined( _WIN32_WINNT ) || ( _WIN32_WINNT < 0x0501 )
#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
namespace asio_dl_impl {
	using parameters = std::unordered_map<std::string, std::string>;
}
class downloader {
private:
	boost::optional<std::string> cert_file_;
public:
	downloader() = default;
	downloader(const std::string& cert_file) : cert_file_(cert_file) {}
	downloader(std::string&& cert_file) : cert_file_(std::move(cert_file)) {}
	downloader(const char* cert_file) : cert_file_(cert_file) {}
	void set_certificate_file(const std::string& cert_file) { this->cert_file_ = cert_file; }
	void set_certificate_file(std::string&& cert_file) { this->cert_file_ = std::move(cert_file); }
	void set_certificate_file(const char* cert_file) { this->cert_file_ = cert_file; }
	void redirect(std::ostream& out_file, const asio_dl_impl::parameters& header, const std::string& get_command, const asio_dl_impl::parameters& param);
	void download_ssl(std::ostream& out_file, const std::string& server_name, const std::string& get_command, const asio_dl_impl::parameters& param);
	void download_nossl(std::ostream& out_file, const std::string& server_name, const std::string& get_command, const asio_dl_impl::parameters& param);
};
