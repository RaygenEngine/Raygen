#include "Test.h"

#include "core/StringHashing.h"
#include <unordered_map>

using namespace std::literals;

// NOTE:
// C++20: only MSVC > 19.23 at: 23/2/2020
// https://wg21.link/P0919R3

TEST("Case sensitive hash & equal")
{
	using namespace str;

	Hash hash{};
	Equal equal{};

	REQ(hash("abc") != hash("AbC"));
	REQ(hash("abc"sv) == hash("abc"));

	REQ(equal("abc"sv, "abc") == true);
	REQ(equal("abc", std::string("")) == false);

	REQ(hash("") == hash(""sv));

	REQ(equal("", "") == true);
}

TEST("Case insensitive hash & equal")
{
	using namespace str;

	HashInsensitive hash{};
	EqualInsensitive equal{};

	REQ(hash("abc") == hash("AbC"));
	REQ(hash("a"sv) != hash("abc"));

	REQ(equal("abc"sv, "ABc") == true);
	REQ(equal("abc", std::string("")) == false);

	REQ(hash("") == hash(""sv));

	REQ(equal("", "") == true);
}

TEST("Case sensitive unordered map")
{
	std::unordered_map<std::string, int, str::Hash, str::Equal> map;

	map.emplace("a", 1);

	REQ(map.find("A"sv) == map.end());
	REQ(map.find("a"sv) != map.end());
}


TEST("Case insesnitive unordered map")
{
	std::unordered_map<std::string, int, str::HashInsensitive, str::EqualInsensitive> map;

	map.emplace("a", 1);

	REQ(map.find("A"sv) != map.end());
	REQ(map.find("a"sv) != map.end());
	REQ(map.find(""sv) == map.end());
}

TEST("String_view map sensitive")
{
	// Be extremelly carefull with this, ALL of the string_views must live longer than the map
	std::unordered_map<std::string_view, int, str::Hash, str::Equal> map;

	std::string_view a = "a"sv;
	const char* empty = "";
	const char* capA = "A";
	const char* another = "a";

	// map.emplace("a"sv, 2); // THIS WOULD BE AN ERROR
	map.insert_or_assign(a, 1);
	map.insert_or_assign(capA, 2);
	map.insert_or_assign(another, 2);


	REQ(map.count("A") == 1);
	REQ(map.count("a") == 1);
	REQ(map.count(empty) == 0);
}

TEST("String_view map insensitive")
{
	// Watch note in case sensitive
	std::unordered_map<std::string_view, int, str::HashInsensitive, str::EqualInsensitive> map;


	const char* aa = "BA";
	std::string_view a = "a"sv;
	std::string_view another = { aa + 1, 1 }; // "A"

	map.insert_or_assign(a, 1);
	map.insert_or_assign(another, 2);

	REQ(map.size() == 1);

	REQ(map.count("A") == 1);
	REQ(map.count("a") == 1);

	REQ(map.count("BA") == 0);
}
