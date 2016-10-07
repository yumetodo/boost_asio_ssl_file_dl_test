#pragma once
namespace std_future {
	template<typename T, std::size_t N>
	constexpr std::size_t size(const T(&)[N]) { return N; }
	template <class C>
	constexpr auto size(const C& c) -> decltype(c.size()) { return c.size(); }
}
