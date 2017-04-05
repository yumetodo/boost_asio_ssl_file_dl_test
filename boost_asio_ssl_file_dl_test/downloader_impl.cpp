#include "downloader_impl.hpp"
#include "size.hpp"
#include <iostream>
#include <boost/asio/ssl.hpp>//require OpenSSL
#include <boost/iostreams/filtering_streambuf.hpp>
#if defined _MSC_VER && !defined(__clang__)
#	pragma warning( push )
//C:\lib\boost_1_61_0\boost/iostreams/copy.hpp(128): warning C4244: '引数': 'std::streamsize' から 'int' への変換です。データが失われる可能性があります。
#	pragma warning( disable : 4244 )
#endif
#include <boost/iostreams/copy.hpp>
#if defined _MSC_VER && !defined(__clang__)
#	pragma warning( pop ) 
#endif
#if defined _MSC_VER && !defined(__clang__)
#	pragma warning( push )
//c:\lib\boost_1_61_0\boost\iostreams\detail\streambuf\indirect_streambuf.hpp(108): warning C4702: 制御が渡らないコードです。
#	pragma warning( disable : 4702 )
//d:\lib\boost_1_63_0\boost\iostreams\filter\zlib.hpp(384) : warning C4706 : 条件式の比較値は、代入の結果になっています。
#	pragma warning( disable : 4706 )
#endif
#include <boost/iostreams/filter/zlib.hpp>
#if defined _MSC_VER && !defined(__clang__)
#	pragma warning( pop ) 
#endif
#if defined _MSC_VER && !defined(__clang__)
#	pragma warning( push )
//c:\lib\boost_1_61_0\boost\iostreams\filter\gzip.hpp(508): warning C4456: 'c' を宣言すると、以前のローカル宣言が隠蔽されます
#	pragma warning( disable : 4456 )
//c:\lib\boost_1_61_0\boost\iostreams\filter\gzip.hpp(463): warning C4458: 'traits_type' を宣言すると、クラス メンバーが隠蔽されます
#	pragma warning( disable : 4458 )
#endif
#include <boost/iostreams/filter/gzip.hpp>
#if defined _MSC_VER && !defined(__clang__)
#	pragma warning( pop ) 
#endif
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
namespace asio_dl_impl {
	namespace asio = boost::asio;
	using asio::ip::tcp;
	template<typename Decompressor>void decompress_impl(std::ostream& os, std::istream& in, Decompressor&& decompressor) {
		using namespace boost::iostreams;
		filtering_streambuf<input> tmp;
		tmp.push(std::forward<Decompressor>(decompressor));
		tmp.push(in);
		boost::iostreams::copy(tmp, os);
	}
	void decompress_deflated(std::ostream& os, std::istream& in) {
		thread_local boost::iostreams::zlib_decompressor decompressor;
		decompress_impl(os, in, decompressor);
	}
	//http://www.boost.org/doc/libs/1_62_0/libs/iostreams/doc/classes/gzip.html
	void decompress_gzip(std::ostream& os, std::istream& in) {
		thread_local boost::iostreams::gzip_decompressor decompressor;
		decompress_impl(os, in, decompressor);
	}
	//http://www.boost.org/doc/libs/1_62_0/libs/iostreams/doc/classes/bzip2.html
	void decompress_bzip2(std::ostream& os, std::istream& in) {
		thread_local boost::iostreams::bzip2_decompressor decompressor;
		decompress_impl(os, in, decompressor);
	}
	void decompress(compress_type type, std::ostream& os, std::istream& in) {
		switch (type) {
			case compress_type::deflated: decompress_deflated(os, in); break;
			case compress_type::gzip: decompress_gzip(os, in); break;
			case compress_type::bzip2: decompress_bzip2(os, in); break;
			default:
				throw compress_type_error("unkown compress type");
		}
	}
	compress_type to_compress_type(const std::string& s) {
		static const std::unordered_map<std::string, compress_type> table = {
			{ "identity", compress_type::identity },
			{ "deflated", compress_type::deflated },
			{ "gzip", compress_type::gzip },
			{ "bzip2", compress_type::bzip2 }
		};
		return (table.count(s)) ? table.at(s) : compress_type::unknown;
	}

	compress_type_error::compress_type_error() : compress_type_error("") {}

	compress_type_error::compress_type_error(const std::string & s) : std::runtime_error("compress_type_error: " + s) {}

	compress_type_error::compress_type_error(const char * s) : compress_type_error(std::string(s)) {}

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
			+ concat_key_value(4, "gzip, deflate, bzip2"s)
			+ concat_key_value(5, "close"s);
		//additional header
		for (auto&& p : param) if (none_of(default_keys, [&p](const string& s) { return p.first == s; })) request_stream << p.first + sep + p.second + CRLF;
		//terminate
		request_stream << CRLF;
	}

	certificate_error::certificate_error() : certificate_error("") {}

	certificate_error::certificate_error(const std::string & s) : std::runtime_error("certificate_error: " + s) {}

	certificate_error::certificate_error(const char * s) : certificate_error(std::string(s)) {}

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
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, static_cast<int>(std_future::size(subject_name)));
		std::cout
			<< "Verifying " << subject_name << std::endl
			<< "Verification status : " << preverified << std::endl
			<< "Function : " << __func__ << " ----------------- Line : " << __LINE__ << std::endl;
		return preverified;
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