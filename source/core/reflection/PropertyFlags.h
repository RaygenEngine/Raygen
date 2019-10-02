#pragma once
namespace PropertyFlags
{
	using Type = uint64;
	
	constexpr Type NoSave	= (1 << 0);
	constexpr Type NoLoad	= (1 << 1);

	// == NoSave | NoLoad, should probably be used everywhere instead of NoSave/NoLoad
	constexpr Type Transient = NoSave | NoLoad;

	constexpr Type NoEdit	= (1 << 2);
	constexpr Type Color	= (1 << 3);
	constexpr Type Multiline = (1 << 4);

	// Relevant on asset pods only.
	// When set: it is not an error if this property is not found when loading.
	constexpr Type OptionalPod = (1 << 5);
	
	// Relevant on vectors.
	// When set: This vector is not resizable in editor.
	constexpr Type VecNoResize = (1 << 6);

	template<typename... Flags>
	constexpr PropertyFlags::Type Pack(Flags ...f)
	{
		static_assert((std::is_same_v<PropertyFlags::Type, Flags> && ...));
		return (f | ...);
	}

	static PropertyFlags::Type Pack()
	{
		return 0;
	}
}