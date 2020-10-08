#pragma once
#include "assets/AssetUpdateInfo.h"

// Base of templated pod editor
struct PodEditorBase {
	static void CommitUpdate(size_t uid, AssetUpdateInfo&& info);
};

// Pod Editor is a class that wraps around an editing operation for a pod and simplifies
// concurrency, pod access while  editing, gpu updating and more
// This is what should be used for pods instead of a const_cast in most cases.
// DOC: provide a full guide for PodEditor class when interface is stable


// Advanced pod editor that may or may not actually edit the pod.
template<CAssetPod PodType>
struct OptionalPodEditor : PodEditorBase {
protected:
	bool didEdit{ false };
	bool inEditRegion{ false };
	bool needsCommit{ false };
	PodHandle<PodType> pod;

public:
	OptionalPodEditor(PodHandle<PodType>& handle)
		: pod(handle)
	{
		// TODO:
		// CLOG_ABORT(handle.IsDefault(), "");
	}
	~OptionalPodEditor() { CommitForGpu(); }

	AssetUpdateInfo info;

	PodType* BeginOptionalEditRegion()
	{
		inEditRegion = true;
		return const_cast<PodType*>(pod.Lock()); // TODO: get through asset manager
	}
	void MarkEdit()
	{
		CLOG_ABORT(!inEditRegion, "Editing pod outside of edit region!");
		didEdit = true;
		needsCommit = true;
	}

protected:
	void EndEditRegion()
	{
		if (!inEditRegion) {
			return;
		}
		inEditRegion = false;

		if (!didEdit) {
			return;
		}
		needsCommit = true;
	}

public:
	void CommitForGpu()
	{
		EndEditRegion();
		if (needsCommit) {
			PodEditorBase::CommitUpdate(pod.uid, std::move(info));
			needsCommit = false;
		}
	}
};

// The simplest possible editor for spontaneous use in stack. (eg during a scene build operation)
// Uses RAII for editing in its scope and always submits instantly when going out of scope.
template<CAssetPod PodType>
struct PodEditor {
private:
	OptionalPodEditor<PodType> optionalEditor;

public:
	PodType* pod;

	PodEditor(PodHandle<PodType>& handle)
		: optionalEditor(handle)
	{
		pod = optionalEditor.BeginOptionalEditRegion();
		optionalEditor.MarkEdit();
	}

	// Return the editable pointer of the pod. Lock is already done during construction
	[[deprecated, nodiscard]] PodType* GetEditablePtr() const { return pod; }
	[[nodiscard]] AssetUpdateInfo& GetUpdateInfoRef() { return optionalEditor.info; }


	~PodEditor() { optionalEditor.CommitForGpu(); }

	PodType* operator*() const { return pod; }
	PodType* operator->() const { return pod; }
};
