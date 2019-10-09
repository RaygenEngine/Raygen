#pragma once

template<typename F>
struct return_type_impl;

template<typename R, typename... Args>
struct return_type_impl<R(Args...)> {
	using type = R;
};

template<typename R, typename... Args>
struct return_type_impl<R(Args..., ...)> {
	using type = R;
};

template<typename R, typename... Args>
struct return_type_impl<R (*)(Args...)> {
	using type = R;
};

template<typename R, typename... Args>
struct return_type_impl<R (*)(Args..., ...)> {
	using type = R;
};

template<typename R, typename... Args>
struct return_type_impl<R (&)(Args...)> {
	using type = R;
};

template<typename R, typename... Args>
struct return_type_impl<R (&)(Args..., ...)> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...)> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...)> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...)&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...)&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) &&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) &&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) const> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) const> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) const&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) const&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) const&&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) const&&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) volatile> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) volatile> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) volatile&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) volatile&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) volatile&&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) volatile&&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) const volatile> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) const volatile> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) const volatile&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) const volatile&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args...) const volatile&&> {
	using type = R;
};

template<typename R, typename C, typename... Args>
struct return_type_impl<R (C::*)(Args..., ...) const volatile&&> {
	using type = R;
};

template<typename T, typename = void>
struct return_type : return_type_impl<T> {
};

template<typename T>
struct return_type<T, decltype(void(&T::operator()))> : return_type_impl<decltype(&T::operator())> {
};

template<typename T>
using return_type_t = typename return_type<T>::type;


template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {
};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {
};

template<typename Vector>
constexpr bool is_vector_v = is_specialization<std::decay_t<Vector>, std::vector>::value;


template<typename V, typename C>
constexpr bool is_vector_of_base()
{
	if constexpr (is_vector_v<V>) { // NOLINT
		return std::is_base_of_v<C, typename V::value_type> || std::is_same_v<C, typename V::value_type>;
	}
	return false;
}

template<typename V, typename C>
constexpr bool is_vector_of_base_ptr()
{
	if constexpr (is_vector_v<V>) {
		if (std::is_pointer_v<typename V::value_type>) {
			using ElementNoPtr = typename std::remove_pointer<typename V::value_type>::type;
			return std::is_base_of_v<C, ElementNoPtr> || std::is_same_v<C, ElementNoPtr>;
		}
		return false;
	}
	return false;
}

template<typename Vector, typename ContaineeReq>
constexpr bool is_vector_of_base_v = is_vector_of_base<Vector, ContaineeReq>();

template<typename Vector, typename ContaineeReq>
constexpr bool is_vector_of_base_ptr_v = is_vector_of_base_ptr<Vector, ContaineeReq>();

namespace detail {
template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
struct detector {
	using value_t = std::false_type;
	using type = Default;
};

template<class Default, template<class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
	using value_t = std::true_type;
	using type = Op<Args...>;
};

} // namespace detail

template<class Default, template<class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

#define DECLARE_HAS_FUNCTION_DETECTOR(FuncName)                                                                        \
	template<typename T, typename = void>                                                                              \
	struct Has##FuncName : std::false_type {                                                                           \
	};                                                                                                                 \
                                                                                                                       \
	template<typename T>                                                                                               \
	struct Has##FuncName<T, std::enable_if_t<std::is_member_function_pointer<decltype(&T::FuncName)>::value>>          \
		: std::true_type {                                                                                             \
	};                                                                                                                 \
                                                                                                                       \
	template<typename T>                                                                                               \
	constexpr bool HasV##FuncName = Has##FuncName<T>::value;
