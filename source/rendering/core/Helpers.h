#pragma once

#include <comdef.h>

inline void AbortIfFailed(HRESULT hr)
{
	if (FAILED(hr)) {
		_com_error err(hr);
		LOG_ABORT(err.ErrorMessage());
	}
}

// Transition a resource
inline void TransitionResource(WRL::ComPtr<ID3D12GraphicsCommandList2> d3d12CommandList,
	WRL::ComPtr<ID3D12Resource> d3d12Resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier
		= CD3DX12_RESOURCE_BARRIER::Transition(d3d12Resource.Get(), beforeState, afterState);

	d3d12CommandList->ResourceBarrier(1, &barrier);
}

// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
inline std::string GetLastErrorAsString()
{
	// Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string(); // No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	// Ask Win32 to give us the string version of that message ID.
	// The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet
	// know how long the message string will be).
	size_t size
		= FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	// Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	// Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}
