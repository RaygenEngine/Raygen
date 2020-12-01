#pragma once
#include <stack>


template<typename T>
concept CSceneElem = true;

struct SceneCollectionBase {
	virtual ~SceneCollectionBase() = default;

protected:
	friend struct Scene;

	// Typeless pointers to elements.
	std::vector<void*> elements;

	size_t elementResize{ 0 };
	std::vector<size_t> condensedLocation;

	void UpdateElementSize()
	{
		elements.resize(elementResize);
		condensedLocation.resize(elementResize);
	}
};

namespace vl {
struct TopLevelAs;
}
//
template<CSceneElem T>
struct SceneCollection : public SceneCollectionBase {
	// provides sequential access to the valid elements with unspecified order (non uid order)
	[[nodiscard]] auto begin() const { return condensed.cbegin(); }
	[[nodiscard]] auto end() const { return condensed.cend(); }

	[[nodiscard]] size_t size() const { return condensed.size(); }
	[[nodiscard]] bool empty() const { return condensed.empty(); }

private:
	friend struct Scene;
	friend struct vl::TopLevelAs; // TODO: remove this when toplevelas is refactored


	T* Get(size_t uid) { return reinterpret_cast<T*>(elements[uid]); }
	void Set(size_t uid, T* value) { elements[uid] = reinterpret_cast<void*>(value); }


	// condensed vector of pointers (ie no gaps. order is unspecified but preserved internally to allow deletion)
	std::vector<T*> condensed;
	std::vector<size_t> condensedToUid;


	// UID manager section (game thread)
	std::stack<size_t> gaps;
	size_t nextUid{ 0 };

	// Game Thread
	size_t GetNextUid()
	{
		if (gaps.empty()) {
			auto uid = nextUid;
			elementResize = ++nextUid;
			return uid;
		}
		auto top = gaps.top();
		gaps.pop();
		return top;
	}

	// Game Thread
	void RemoveUid(size_t uid) { gaps.push(uid); }

	// Scene Thread
	// Efficiently removes a uid from the condensed vector & updates the relevant condensedLocation information
	void SwapPopFromCondensed(size_t Uid)
	{
		size_t cIndex = condensedLocation[Uid];
		size_t backUid = condensedToUid.back();

		condensed[cIndex] = condensed.back();
		condensed.pop_back();

		condensedToUid[cIndex] = backUid;
		condensedToUid.pop_back();

		condensedLocation[backUid] = cIndex;
	}

	// CHECK: Correct destructor
	// Currently, this struct can be instantiated with a fwd declaration of the content. Adding the proper destructor
	// here would make this impossible, therefore we manually delete everything in scene destructor currently.
};
