#pragma once


class ShaderRegistry {
public:
	struct KNode;
	static constexpr std::string_view c_shaderDir = "engine-data/spv/";
	static constexpr std::string_view c_shaderIncludeDir = "includes/";

	using NSet = std::unordered_set<KNode*>;

	ShaderRegistry(ShaderRegistry const&) = delete;
	ShaderRegistry(ShaderRegistry&&) = delete;
	ShaderRegistry& operator=(ShaderRegistry const&) = delete;
	ShaderRegistry& operator=(ShaderRegistry&&) = delete;


	struct KNode {

		// when true this node is a true shader (= no depndees, .vert/.frag etc file)
		bool isLeaf{ false };

		// PERF: Crappy pointer tree implementation, refactor to cont. memory with indicies
		std::unordered_set<KNode*> directDependencies;
		std::unordered_set<KNode*> directDependees;


		BasePodHandle pod;

		PodHandle<ShaderStage> GetLeafPod() const;
		PodHandle<ShaderHeader> GetHeaderPod() const;

		// Assumes pod is already loaded
		const std::string& GetCode() const;


		//// Mark all direct
		// void MarkDirty(NSet& workingStack, NSet& knownDirty)
		//{
		//	processedCode = {};
		//	for (auto dependee : directDependees) {
		//		if (knownDirty.insert(dependee).second) {
		//			workingStack.insert(dependee);
		//			processedCode = {};
		//		}
		//	}
		//}

		// std::string processedCode;

		// bool CanPreprocessCode(NSet& knownDirty)
		//{
		//	for (auto dirty : knownDirty) {
		//		if (directDependencies.contains(dirty)) {
		//			// We cannot compile this yet.
		//			return false;
		//		}
		//	}
		//	return true;
		//}

		// Assumes all direct dependencies have been process
		// void GenerateProcessedCode();

		// NSet DetectNewDependencies();

		// Update all dependencies and dependees and code. (called on init)
		void FullyCacheSelf();
	};


public:
	// Expects names relative to the shader directory.
	static KNode* FindOrAdd(const std::string& shaderFilename) { return Get().FindOrAdd_Internal(shaderFilename); }


	static void OnEdited(BasePodHandle baseHandle);

private:
	static std::string NormalizeFilenameForSearch(std::string_view filename);

	KNode* FindOrAdd_Internal(const std::string& shaderFilename, bool forceRecache = false);

	// Attempt to find in asset manager or import from disk.
	KNode* Add(const std::string& filename);

	ShaderRegistry() = default;
	static ShaderRegistry& Get();

	std::unordered_map<std::string, KNode*> m_tree;


	// void OnEdited(KNode* node);

	bool isSelfIterating{ false };

	~ShaderRegistry();
};


#include "ShaderRegistry.impl.h"
