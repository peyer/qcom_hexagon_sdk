#include <string>

// make callable from C code
extern "C" {
	void bar(const char* a, char* b);
}

void bar(const char* a, char* b)
{
	std::string c = (a);
	std::string d = std::string(a) + std::string(" world");
	strcpy(b, d.c_str());
}