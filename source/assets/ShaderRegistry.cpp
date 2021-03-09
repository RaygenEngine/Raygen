#include "ShaderRegistry.h"

#include "assets/AssetImporterManager.h"
#include "assets/PodEditor.h"
#include "assets/pods/ShaderHeader.h"
#include "assets/pods/ShaderStage.h"
#include "assets/util/shadergen/ShaderPreprocess.h"

const std::string& ShaderRegistry::KNode::GetCode() const
{
	if (!isLeaf) {
		return GetHeaderPod().Lock()->code;
	}
	return GetLeafPod().Lock()->code;
}

void ShaderRegistry::KNode::FullyCacheSelf()
{
	// Slow remove & re-add, this can be avoided
	for (auto dependency : directDependencies) {
		dependency->directDependees.erase(this);
	}
	directDependencies.clear();

	auto& code = GetCode();

	std::vector<std::tuple<int32, std::string, ShaderRegistry::KNode*>> outIncludes;

	processedCode = shd::PreprocessCode(code, outIncludes);

	for (auto&& [line, filename, node] : outIncludes) {
		directDependencies.insert(node);
		node->directDependees.insert(this);
	}
}

ShaderRegistry::~ShaderRegistry()
{
	for (auto&& [str, ptr] : m_tree) {
		delete ptr;
	}
}

void ShaderRegistry::OnEdited(BasePodHandle baseHandle)
{
	// Remember callstack to not call this on subsequent calls (maybe we should just recursively edit?)
	if (Get().isSelfIterating) {
		return;
	}
	auto entry = AssetRegistry::GetEntry(baseHandle);


	// Find or update cached stuff. The whole upper part of the tree will be properly updated and cached when this
	// function returns.
	auto filename = ShaderRegistry::NormalizeFilenameForSearch(entry->metadata.originalImportLocation);
	KNode* initialNode;
	{
		initialNode = Get().FindOrAdd_Internal(filename, true);
	}

	if (!initialNode) {
		return;
	}

	initialNode->FullyCacheSelf();

	if (initialNode->isLeaf) {
		return;
	}
	// Go down the tree and compile all leaves from here. (Later we should cache everything on each step to paste them)
	Get().isSelfIterating = true;
	// @HACK: explicitly export the header to filesystem here because we need to use external include still and the
	// header will contain the old code otherwise
	AssetRegistry::SaveToDisk(baseHandle);

	NSet addedSet;
	NSet workingList;

	workingList.insert(initialNode);
	addedSet.insert(initialNode);
	while (!workingList.empty()) {
		KNode* node = *workingList.begin();
		workingList.erase(node);

		if (node->isLeaf) {
			PodEditor ed(node->GetLeafPod());

			TextCompilerErrors errors;
			int32 total = 0;
			for (auto c : node->processedCode) {
				total += c == '\n' ? 1 : 0;
			}
			// LOG_REPORT("Compiling Shader Characters: {} Lines: {}", node->processedCode.size(), total);
			if (!ed->Compile(errors, node->processedCode)) {
				const auto& name = AssetRegistry::GetEntry(node->pod)->name;

				std::string errString;
				// TODO: should have a generic way to report these errors? maybe we already have?
				// TODO: Decode headers
				for (auto& [line, error] : errors.errors) {
					errString += fmt::format("{}: Line {:>3}: {}", name, line, error);
				}

				LOG_ERROR(
					"Errors when compiling shader from header edit: {} \nHeader that caused recompile was: "
					"{}\n=================\n{}",
					AssetRegistry::GetPodUri(node->pod), entry->name, errString);
			}
		}
		else {
			// LOG_REPORT("Updating Header File: {}", AssetRegistry::GetPodUri(node->pod));
			for (auto dep : node->directDependees) {
				if (addedSet.insert(dep).second) {
					workingList.insert(dep);
				}
			}
		}
	}
	Get().isSelfIterating = false;
}

std::string ShaderRegistry::NormalizeFilenameForSearch(std::string_view filename)
{
	if (filename.starts_with(c_shaderDir)) {
		filename = filename.substr(c_shaderDir.size());
		if (filename.starts_with(c_shaderIncludeDir)) {
			return std::string{ filename.substr(c_shaderIncludeDir.size()) };
		}
	}
	return std::string{ filename };
}

ShaderRegistry::KNode* ShaderRegistry::FindOrAdd_Internal(const std::string& shaderFilename, bool forceRecache)
{
	if (shaderFilename.empty()) {
		LOG_ERROR(
			"Empty filename requested form shader registry. Currently not supported. (All shaders are expected to be "
			"disk assets at this time)");
	}

	if (m_tree.contains(shaderFilename)) {
		auto ptr = m_tree[shaderFilename];
		if (forceRecache) {
			ptr->FullyCacheSelf();
		}
		return ptr;
	}

	return Add(shaderFilename);
}

ShaderRegistry::KNode* ShaderRegistry::Add(const std::string& filename)
{
	if (filename.ends_with(".glsl")) {
		auto path = fs::path(c_shaderDir) / fs::path(c_shaderIncludeDir) / filename;

		auto handle = AssetRegistry::GetFromImportPathVerified<ShaderHeader>(path.generic_string());
		if (handle.IsDefault()) {

			AssetImporterManager->SetPushPath("gen-data/shaders/includes/");
			handle = AssetImporterManager->ImportRequest<ShaderHeader>(path);
			AssetImporterManager->PopPath();

			if (handle.IsDefault()) {
				LOG_WARN("Failed to find shader include: {}", path.generic_string());
				return nullptr;
			}
		}

		auto createdNode = new ShaderRegistry::KNode();
		createdNode->isLeaf = false;
		createdNode->pod = handle;
		createdNode->FullyCacheSelf();
		m_tree[filename] = createdNode;
		return createdNode;
	}

	// Non header file:

	auto path = fs::path(c_shaderDir) / filename;
	auto handle = AssetRegistry::GetFromImportPathVerified<ShaderStage>(path.generic_string());
	if (handle.IsDefault()) {
		AssetImporterManager->SetPushPath("gen-data/shaders/");
		handle = AssetImporterManager->ImportRequest<ShaderStage>(path.generic_string());
		AssetImporterManager->PopPath();
		if (handle.IsDefault()) {
			LOG_ERROR("Attempted to add shader file: {}. Not found in known assets or on disk.", path.generic_string());
			return nullptr;
		}
	}

	auto createdNode = new ShaderRegistry::KNode();
	createdNode->isLeaf = true;
	createdNode->pod = handle;
	createdNode->FullyCacheSelf();
	m_tree[filename] = createdNode;
	return createdNode;
}
