#pragma once

struct SceneCompBase;
class Entity;


template<typename T>
concept CComponent = true;

namespace componentdetail {
// clang-format off

// Utility detectors
template<typename T> concept CDirtableComp = requires { typename T::Dirty; };
template<typename T> concept CCreateDestoryComp = requires { typename T::Create; typename T::Destroy; };


template<typename T> concept CTickableComp = requires (T t) { { t.Tick(1.f) }; };
template<typename T> concept CBeginPlayComp = requires (T t){ t.BeginPlay(); };
template<typename T> concept CEndPlayComp = requires (T t){ t.EndPlay(); };
template<typename T> concept CSelfEntityMember = requires (T t) { { t.self } -> std::convertible_to<Entity>; };

template<typename T> concept CScriptlikeComp = CTickableComp<T> || CBeginPlayComp<T> || CEndPlayComp<T>;

// clang-format on
} // namespace componentdetail

template<typename T>
concept CSceneComp = requires
{
	CComponent<T>;
	typename T::RenderSceneType;
	componentdetail::CDirtableComp<T>;
	componentdetail::CCreateDestoryComp<T>;
};

template<typename T>
concept CScriptlikeComp
	= CComponent<
		  T> && (componentdetail::CTickableComp<T> || componentdetail::CBeginPlayComp<T> || componentdetail::CEndPlayComp<T>);
