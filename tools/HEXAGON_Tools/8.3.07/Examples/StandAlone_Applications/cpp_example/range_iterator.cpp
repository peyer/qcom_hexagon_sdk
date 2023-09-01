/*
  This is a simple range iterator testcase
  to demonstrate hexagon-clang compiler c++11
  extension support.

  This example should be built with hexagon tools
  version 8.0 or higher.
*/
#include <stdio.h>

using namespace std;

struct irange { int First; int Last; };
struct irange_iterator {
	int Val;

	// minimalistic forward-iterator implementation
	int operator*()                            { return Val;          }
	bool operator!=(irange_iterator const & r) { return Val != r.Val; }
	irange_iterator operator++()               { return {++Val};      }
};

// begin() and end() are invoked by range-based for loop
irange_iterator begin(irange const & r) { return { r.First }; }
irange_iterator end(irange const & r) { return { r.Last + 1 }; }

int main() {
	for (auto v : irange{10,20})
		printf("value in irange = %d \n", v);
}
