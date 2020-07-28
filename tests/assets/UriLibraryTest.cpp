#include "Test.h"

#include "assets/UriLibrary.h"
#include "assets/AssetRegistry.h"

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

TEST("Export path specification test")
{
	using detail::GenerateExportDependencyPath;

	auto cwd = fs::current_path();
	auto notcwd = cwd.parent_path();

	PathReferenceType resultType{};
	fs::path resultPath;
	fs::path exporteePath;
	fs::path dependencyPath;


	// Spec case 1:
	SECT("case 1")
	{
		exporteePath = "/xy/export.exp";
		dependencyPath = "/xy/reference.exp";

		resultType = GenerateExportDependencyPath(exporteePath, dependencyPath, resultPath);

		REQ(resultType == PathReferenceType::FileRelative);
		REQ(resultPath.generic_string() == "reference.exp");
	}

	// Spec case 2:
	SECT("case 2a")
	{
		exporteePath = notcwd / "xy/export.exp";
		dependencyPath = notcwd / "xy/sub1/sub2/reference.exp";

		resultType = GenerateExportDependencyPath(exporteePath, dependencyPath, resultPath);

		REQ(resultType == PathReferenceType::FileRelative);
		REQ(resultPath.generic_string() == "sub1/sub2/reference.exp");
	}

	// case 2b
	SECT("case 2b")
	{
		exporteePath = cwd / "xy/export.exp";
		dependencyPath = cwd / "xy/sub1/sub2/reference.exp";

		resultType = GenerateExportDependencyPath(exporteePath, dependencyPath, resultPath);

		REQ(resultType == PathReferenceType::FileRelative);
		REQ(resultPath.generic_string() == "sub1/sub2/reference.exp");
	}

	// Spec case 3:
	SECT("case 3")
	{
		exporteePath = cwd / "folder/sub1/export.exp";
		dependencyPath = cwd / "folder/sub2/reference.exp";

		resultType = GenerateExportDependencyPath(exporteePath, dependencyPath, resultPath);

		REQ(resultType == PathReferenceType::WorkingDir);
		REQ(resultPath.generic_string() == "folder/sub2/reference.exp");
	}

	// Spec case 4:
	SECT("case 4")
	{
		exporteePath = notcwd / "somewhere/export.exp";
		dependencyPath = cwd / "folder/reference.exp";

		resultType = GenerateExportDependencyPath(exporteePath, dependencyPath, resultPath);

		REQ(resultType == PathReferenceType::WorkingDir);
		REQ(resultPath.generic_string() == "folder/reference.exp");
	}

	// Spec case 5:
	SECT("case 5")
	{
		exporteePath = notcwd / "somewhere/export.exp";
		dependencyPath = notcwd / "elsewhere/reference.exp";

		resultType = GenerateExportDependencyPath(exporteePath, dependencyPath, resultPath);

		REQ(resultType == PathReferenceType::FullPath);
		REQ(resultPath.generic_string() == fs::absolute(dependencyPath).generic_string());
	}
}
