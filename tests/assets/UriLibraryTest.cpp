#include "Test.h"

#include "assets/UriLibrary.h"
#include <unordered_map>

using namespace std::literals;

TEST("DetectCountFromPath test")
{
	std::string_view out;
	std::string p1 = "Mesh_13";
	size_t num = uri::DetectCountFromPath(p1, out);
	REQ(num == 13);
	REQ(out.compare("Mesh_") == 0);

	std::string p2 = "";
	num = uri::DetectCountFromPath(p2, out);
	REQ(num == 0);
	REQ(out.compare("") == 0);

	std::string p3 = "9849";
	num = uri::DetectCountFromPath(p3, out);
	REQ(num == 0);
	REQ(out.compare("9849") == 0);


	p3 = "Kappa";
	num = uri::DetectCountFromPath(p3, out);
	REQ(num == 0);
	REQ(out.compare("Kappa") == 0);
}
