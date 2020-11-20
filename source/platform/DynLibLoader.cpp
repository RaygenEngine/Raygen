#include "DynLibLoader.h"

// TODO: Implement the rest of the platforms properly
#if defined(_WIN32)
#	include <Windows.h>
#endif


struct DynLibLoader::PlatformData {
#if defined(__linux__) || defined(__APPLE__)
	void* library;
#elif defined(_WIN32)
	HMODULE library;
#else
#	error unsupported platform
#endif
};

DynLibLoader::DynLibLoader(const char* filenameNoExt)
{
	LoadDynLibrary(filenameNoExt);
}

void DynLibLoader::LoadDynLibrary(const char* filenameNoExt)
{
	if (m_platformData) {
		UnloadDynLibrary();
	}

#if defined(__linux__)
	const char* platformExt = ".so";
#elif defined(__APPLE__)
	const char* platformExt = ".dylib";
#elif defined(_WIN32)
	const char* platformExt = ".dll";
#endif

	std::string filename = fmt::format("{}{}", filenameNoExt, platformExt);

	m_platformData = new DynLibLoader::PlatformData();

#if defined(__linux__)
	m_platformData->library = dlopen(filename.c_str(), RTLD_NOW | RTLD_LOCAL);
#elif defined(__APPLE__)
	m_platformData->library = dlopen(filename.c_str(), RTLD_NOW | RTLD_LOCAL);
#elif defined(_WIN32)
	m_platformData->library = LoadLibrary(filename.c_str());
#endif
	assert(m_platformData->library && "Failed to open library");
}

void DynLibLoader::UnloadDynLibrary()
{
	if (m_platformData) {
#if defined(__linux__) || defined(__APPLE__)
		dlclose(m_platformData->library);
#elif defined(_WIN32)
		FreeLibrary(m_platformData->library);
#endif
		delete m_platformData;
	}
}

DynLibLoader::~DynLibLoader()
{
	UnloadDynLibrary();
}


void* DynLibLoader::InternalGetProcAddr(const char* functionName) const
{
	assert(m_platformData && "Get Proc Addr called without having an open library");
#if defined(__linux__) || defined(__APPLE__)
	return dlsym(m_library, functionName);
#elif defined(_WIN32)
	return GetProcAddress(m_platformData->library, functionName);
#endif
}
