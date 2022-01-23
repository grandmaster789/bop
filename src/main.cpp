#include <iostream>

#include "raw_string_stream.h"

int main() {
	bop::RawStringStream rss;

	rss << "testing " << 123 << " " << &rss << " " << 34.5;

	std::cout << rss.c_str() << '\n';

	std::cout << "Ping\n";
}