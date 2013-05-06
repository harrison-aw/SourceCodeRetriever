/*
 * main.cpp
 *
 *  Created on: May 4, 2013
 *      Author: Tony
 */

#include <iostream>

#include "GitHubLink.h"

using namespace std;
using namespace nnproject;

int main() {

	GitHubLink link;

	try {
		link.downloadCode("lisp", 5000, 10000);
	} catch (GitHubLink::xml_parsing_exception &e) {
		cout << e.what() << endl;
	}

	return 0;
}


