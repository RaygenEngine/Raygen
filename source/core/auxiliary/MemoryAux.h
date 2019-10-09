#pragma once

#include <vector>
#include <algorithm>

// TODO: create a smart pointer container with dereference iterators
// template <class BaseIterator>
// class DereferenceIterator : public BaseIterator
//{
// public:
//	using ValueType = typename BaseIterator::value_type::element_type;
//	using Pointer = ValueType*;
//	using Reference = ValueType&;
//
//	DereferenceIterator(const BaseIterator& other)
//		: BaseIterator(other)
//	{
//	}
//
//	ValueType operator*() const { return *(this->BaseIterator::operator*()); }
//	Pointer operator->() const { return this->BaseIterator::operator*().get(); }
//	Pointer operator[](size_t n) const { return *(this->BaseIterator::operator[](n)); }
//};
//
// template <typename Iterator>
// DereferenceIterator<Iterator> dereference_iterator(Iterator t) { return DereferenceIterator<Iterator>(t); }
//
// template <typename SmartPtr>
// class SmartContainer
//{
// public:
//	SmartContainer() = default;
//
//	// give ownership to smart container
//	void Add(SmartPtr foo) { smartPtrs.push_back(std::move(foo)); }
//
//	DereferenceIterator<typename std::vector<SmartPtr>::iterator> begin() { return
// dereference_iterator(smartPtrs.begin()); } 	DereferenceIterator<typename std::vector<SmartPtr>::iterator> end() {
// return dereference_iterator(smartPtrs.end()); }
//
//	DereferenceIterator<typename std::vector<SmartPtr>::iterator> begin() const { return
// dereference_iterator(smartPtrs.begin()); } 	DereferenceIterator<typename std::vector<SmartPtr>::iterator> end() const
// { return dereference_iterator(smartPtrs.end()); }
//
//	DereferenceIterator<typename std::vector<SmartPtr>::iterator> cbegin() const { return
// dereference_iterator(smartPtrs.begin()); } 	DereferenceIterator<typename std::vector<SmartPtr>::iterator> cend()
// const { return dereference_iterator(smartPtrs.end()); }
//
// private:
//	std::vector<SmartPtr> smartPtrs;
//};

namespace utl {

template<class T>
[[nodiscard]] constexpr typename std::remove_reference<T>::type&& force_move(T const& t) noexcept = delete;

template<class T>
[[nodiscard]] constexpr typename std::remove_reference<T>::type&& force_move(T&& t) noexcept
{
	return std::move(t);
}
} // namespace utl
