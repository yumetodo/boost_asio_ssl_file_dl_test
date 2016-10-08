#include "downloader.hpp"
#include <iostream>
#include <fstream>
#include <string>
//Host: cybozulive.com
//User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:46.0) Gecko/20100101 Firefox/46.0
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
//Accept-Language: ja,en-US;q=0.7,en;q=0.3
//Accept-Encoding: gzip, deflate, br
//Referer: https://cybozulive.com/2_86347/top/top?ref=icon
//Cookie: __ctc=0oDq21Y/AOybmxPUAwz6Ag==; __utma=224468744.1265407494.1446969580.1463456404.1463583198.38; __utmz=224468744.1446969580.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none); JSESSIONID=8D532B92C96A564F16B9F466B566F5E8; __utmc=224468744
//Connection: keep-alive

int main()
{
	{
		std::cout << "SSL Test" << std::endl;
		downloader()
		.download(
			std::cout, 
			"https://gist.githubusercontent.com/yumetodo/3515d60ff3743d57ac58/raw/2f94b4e7a0bfe979b4d45588dfdccdc33eb1ee2d/CMakelists_Boost_asio_cpp11_buffers_reference_counted.txt", 
			{}
		);
		std::cout << "Redirect Test" << std::endl;
		std::ofstream out("out.html");
		downloader().download(out, "http://www.google.com/", {});
	}
	//{
	//	std::string server_name = "demo.lizardtech.com";

	//	std::string get_command = "/lizardtech/iserv/ows?SERVICE=WMS&REQUEST=GetMap&";
	//	get_command += "LAYERS=LACounty,&STYLES=&";
	//	get_command += "BBOX=314980.5,3624089.5,443200.5,3861209.5&";
	//	get_command += "SRS=EPSG:26911&FORMAT=image/gif&HEIGHT=300&WIDTH=600";

	//	std::ofstream out_file("image.gif", std::ios::out | std::ios::binary);
	//	downloader dl;
	//	dl.download_nossl(out_file, server_name, get_command, {});
	//}
	//{
	//	const std::string uri = "api.foursquare.com";
	//	const std::string oauth_token = "********";
	//	const std::string host = "api.foursquare.com";
	//	const std::string api_request = "/v2/users/self";
	//	downloader dl("cert.pem");
	//	dl.download_ssl(std::cout, host, api_request + "?oauth_token=" + oauth_token, {});
	//}
	return 0;
}
