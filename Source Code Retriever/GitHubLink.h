/*
 * GitHubLink.h
 *
 *  Created on: May 4, 2013
 *      Author: Tony
 */

#ifndef GITHUBLINK_H_
#define GITHUBLINK_H_

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>

namespace nnproject {

class GitHubLink {
public:
	GitHubLink();
	virtual ~GitHubLink();

	void downloadCode(std::string file_extension, unsigned int size_lower_bound, unsigned int size_upper_bound);

	class xml_conversion_exception: public std::runtime_error {
	public:
		explicit xml_conversion_exception(const std::string &what_arg): std::runtime_error(what_arg) {};
	};

	class xml_parsing_exception: public std::runtime_error {
	public:
		explicit xml_parsing_exception(const std::string &what_arg): std::runtime_error(what_arg) {};
	};

	class download_exception: public std::runtime_error {
	public:
			explicit download_exception(const std::string &what_arg): std::runtime_error(what_arg) {};
	};

protected:
	std::string getUrl() const;
	void getRawHtml(const std::string &url);
	void download(const std::string &url);

	static std::string convertHtmlToXml(std::string raw_html);
	static std::vector<std::string> getCodeUrls(std::string xhtml);

	static size_t writeRawHtml(char *buf, size_t size, size_t nmemb, void *p);
	static bool isTitleParagragh(xmlNodePtr node);
	static std::string getRawUrl(const xmlChar *url_path);
	static void parseForTitleUrls(xmlNodePtr node, std::vector<std::string> &title_urls);

	std::string file_extension;
	unsigned long size_lower_bound;
	unsigned long size_upper_bound;
	unsigned int current_page;

	std::string raw_html;
};

} /* namespace srcret */
#endif /* GITHUBLINK_H_ */
