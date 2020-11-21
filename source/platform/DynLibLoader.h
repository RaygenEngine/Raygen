#pragma once

#define LIBLOAD_FUNC(FunctionName) &FunctionName, #FunctionName

class DynLibLoader {

public:
	// Does not open any library.
	DynLibLoader() = default;

	// Automatically opens the library requested. Extension auto added per platform.
	DynLibLoader(const char* filenameNoExt);

	// Loads the library requested. Extension auto added per platform. (This will unload the previous library if one
	// exists)
	void LoadDynLibrary(const char* filenameNoExt);


	[[nodiscard]] bool HasLoadedLibrary() const { return m_platformData != nullptr; }

	// Unloads the library (if loaded)
	void UnloadDynLibrary();

	// Unsafe cast, up to the caller to know the function type
	template<typename T>
	[[nodiscard]] T GetProcAddr(const char* functionName) const
	{
		return (T)InternalGetProcAddr(functionName);
	}

	// Will unload the library if loaded
	~DynLibLoader();


	template<class R, class... Args>
	[[nodiscard]] R (*GetProcAddrExt(const char* functionName) const)(Args...)
	{
		typedef R (*ptr)(Args...);
		return GetProcAddr<ptr>(functionName);
	}

	// Use the provided LIBLOAD_FUNC() macro like this:
	// dynlib.LibLoadFunc(LIBLOAD_FUNC(sum));
	// the macro automatically fills type and name
	template<typename R, class... Args>
	[[nodiscard]] auto LibLoadFunc(R (*dummyPtr)(Args...), const char* functionName) const
	{
		typedef R (*ptr)(Args...);
		return GetProcAddr<ptr>(functionName);
	}

private:
	struct PlatformData;
	void* InternalGetProcAddr(const char* functionName) const;
	PlatformData* m_platformData{ nullptr };
};
