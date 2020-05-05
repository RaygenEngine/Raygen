#pragma once

#include "assets/AssetUpdateInfo.h"
#include "assets/PodHandle.h"

// Base of templated pod editor
struct PodEditorBase {
	static void CommitUpdate(size_t uid, AssetUpdateInfo&& info);
};

// Pod Editor is a class that wraps around an editing operation for a pod and simplifies
// concurrency, pod access while  editing, gpu updating and more
// This is what should be used for pods instead of a const_cast in most cases.
// DOC: provide a full guide for PodEditor class when interface is stable


// Advanced pod editor that may or may not actually edit the pod.
template<CONC(CAssetPod) PodType>
struct OptionalPodEditor : PodEditorBase {
protected:
	bool didEdit{ false };
	bool inEditRegion{ false };
	bool needsCommit{ false };
	PodHandle<PodType> pod;

public:
	OptionalPodEditor(PodHandle<PodType> handle)
		: pod(handle)
	{
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
template<CONC(CAssetPod) PodType>
struct PodEditor {
private:
	OptionalPodEditor<PodType> optionalEditor;

public:
	PodType* pod;

	PodEditor(PodHandle<PodType> handle)
		: optionalEditor(handle)
	{
		pod = optionalEditor.BeginOptionalEditRegion();
		optionalEditor.MarkEdit();
	}

	// Return the editable pointer of the pod. Lock is already done during construction
	[[nodiscard]] PodType* GetEditablePtr() const { return pod; }
	[[nodiscard]] AssetUpdateInfo& GetUpdateInfoRef() { return optionalEditor.info; }


	~PodEditor() { optionalEditor.CommitForGpu(); }
};

/*

Multithreaded Proof of concept Example: (NOT IN USE, FOR FUTURE REFERENCE)
#include "AssetManager.h"
#include "PodIncludes.h"

int example()
{
	PodHandle<Image> handle;


	PodEditor<Image> editor(handle);


	//
	// Case Critical Lock
	//
	{
		Image* image = editor.CriticalLock(); // (Maybe blocking) Inform the engine that this asset will be in
											  // invalid state for the scope of this editor. (eg: Resizing an image
											  // where there are race conditions even for simple observers)

		image->width = 128;
		// DO NOT READ here concurrently. Invalid Pod State
		image->data = {}; //
		editor.Unlock();
	}


	//
	// Case Realtime Observer & Editor (eg property editor)
	//
	{
		// Let others take editing priority,
		if (editor.CheckRealtimeEdit()) {
			Image* image = editor.RealtimeLock(); // Editing mode.
		}
		else if (editor.CheckRealtimeRead()) {
			const Image* image = editor.RealtimeRead(); // Read Only mode.
		}
		else {
			// Nothing. Asset could be in an invalid state due to a critical lock editor in another thread.
		}
	}
}
*/
