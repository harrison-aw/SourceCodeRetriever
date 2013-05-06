/*
 * GitHubLink.cpp
 *
 *  Created on: May 4, 2013
 *      Author: Tony
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include <curl/curl.h>

#include <tidy.h>
#include <buffio.h>

#include "GitHubLink.h"

using namespace std;

namespace nnproject {

GitHubLink::GitHubLink() {
}

GitHubLink::~GitHubLink() {
}

void GitHubLink::downloadCode(string file_extension, unsigned int size_lower_bound, unsigned int size_upper_bound) {
	this->file_extension = file_extension;
	this->size_lower_bound = size_lower_bound;
	this->size_upper_bound = size_upper_bound;

	getRawHtml(getUrl());
	vector<string> code_urls(getCodeUrls(convertHtmlToXml(raw_html)));

	for (vector<string>::iterator it = code_urls.begin(); it != code_urls.end(); ++it)
		download(*it);
}

std::string GitHubLink::getUrl() const {
	stringstream ss;
	ss << "https://github.com/search?type=Code&p=" << current_page
			<< "&q=extension%3A" << file_extension
			<< "+size%3A" << size_lower_bound << ".." << size_upper_bound;
	return ss.str();
}

void GitHubLink::getRawHtml(const std::string &url) {
	raw_html = "";

	CURL *curl;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeRawHtml);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

	curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void GitHubLink::download(const string &url) {
	cout << "Downloading: " << url << endl;
	getRawHtml(url);
	cout << "Contents: " << endl << raw_html << endl;

	string filename = url.substr(url.find_last_of('/') + 1);
	ofstream f(filename.c_str());
	if (f) {
		f.write(raw_html.c_str(), raw_html.size());
		f.close();
		cout << filename << " written" << endl;
	} else {
		throw download_exception("Unable to download: " + url);
	}
}


std::string GitHubLink::convertHtmlToXml(std::string raw_html) {
	TidyDoc tdoc = tidyCreate();

	Bool ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
	int rc = -1;
	TidyBuffer errbuf = {0};
	TidyBuffer output = {0};
	if (ok)
		rc = tidySetErrorBuffer(tdoc, &errbuf);
	if (rc >= 0)
		rc = tidyParseString(tdoc, raw_html.c_str());
	if (rc >= 0)
		rc = tidyRunDiagnostics(tdoc);
	if (rc > 1)
		rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
	if (rc >= 0)
		rc = tidySaveBuffer(tdoc, &output);

	stringstream ssoutput;
	if (rc >= 0) {
		if (rc > 0) {
			ssoutput << output.bp;
		}
	} else {
		cout << "A severe error (" << rc << ") occurred." << endl;
	}

	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);

	return ssoutput.str();
}

std::vector<std::string> GitHubLink::getCodeUrls(std::string xhtml) {
	xmlDocPtr doc = xmlReadMemory(xhtml.c_str(), xhtml.size(), "noname.xml", NULL, 0);
	if (doc == NULL)
		throw xml_parsing_exception("unable to parse data");

	xmlNodePtr root_element = xmlDocGetRootElement(doc);
	if (root_element == NULL)
		throw xml_parsing_exception("could not get root element");

	vector<string> title_urls;
	parseForTitleUrls(root_element, title_urls);
	return title_urls;
}

size_t GitHubLink::writeRawHtml(char *buf, size_t size, size_t nmemb, void *p) {
	for (size_t c = 0; c < size * nmemb; ++c) {
		static_cast<GitHubLink*>(p)->raw_html.push_back(buf[c]);
	}
	return size * nmemb;
}


bool GitHubLink::isTitleParagragh(xmlNodePtr node) {
	const xmlChar *name = node->name;

	if (xmlStrEqual(name, BAD_CAST("p")) == 1) {
		xmlChar *class_attr = xmlGetProp(node, BAD_CAST("class"));
		if (class_attr != NULL) {
			bool found = xmlStrEqual(class_attr, BAD_CAST("title")) == 1;
			xmlFree(class_attr);
			if (found)
				return true;
		}
	}
	return false;
}

string GitHubLink::getRawUrl(const xmlChar *url_path) {
	xmlChar *pre_blob = xmlStrdup(url_path);

	xmlChar *blob = const_cast<xmlChar*>(xmlStrstr(pre_blob, BAD_CAST("/blob")));
	xmlChar *post_blob = blob + 5;

	*blob = '\0';
	xmlChar *no_blob = xmlStrncatNew(pre_blob, post_blob, -1);
	*blob = '\'';

	string raw_url(reinterpret_cast<char*>(no_blob));

	xmlFree(pre_blob);
	xmlFree(no_blob);

	return "https://raw.github.com" + raw_url;
}

void GitHubLink::parseForTitleUrls(xmlNodePtr node, vector<string> &title_urls) {
	if (isTitleParagragh(node)) {
		unsigned int a_tags = 0;
		for (xmlNodePtr child = node->children; child != NULL; child = child->next) {
			if (xmlStrEqual(child->name, BAD_CAST("a"))) {
				if (++a_tags == 2) {
					xmlChar *url_path = xmlGetProp(child, BAD_CAST("href"));
					title_urls.push_back(getRawUrl(url_path));
					xmlFree(url_path);
				}
			}
		}
	}

	if (node->children != NULL)
		parseForTitleUrls(node->children, title_urls);
	if (node->next != NULL)
		parseForTitleUrls(node->next, title_urls);
}

} /* namespace srcret */
