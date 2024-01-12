#ifndef VAR_STUFF_H_
#define VAR_STUFF_H_

// DESCRIPTION + DECLARATIONS
#define VAR // the whole goal of this file
#define VISIT // it's the goal too!
#define OVERLOAD // overload technique for convenient visitor creation
//#define BIGGEST // trait to get the biggest type from parameter pack <-- note: casts void to struct WasVoid
//#define INTEGER_SEQUENCE // alternative to std::integer_sequence of c++14
//#define INVOKE // some traits to check invocables
//#define COMMON // common traits such as contains and returnType
// DESCRIPTION + DECLARATIONS

#ifdef VISIT // defines functions for visiting var_stuff::variant; super primitive pattern matching
#define VAR // VISIT can't exist without VARIANT :)
#define COMMON // VISIT uses returnType, conditionalContains
#endif // VISIT

#ifdef VAR_KINDS // defines option, result
#define VAR // VAR_KINDS use var_stuff::variant as ancestor
#define COMMON // option uses WasVoid
#endif // VARIANT_KINDS 

#ifdef VAR // defines var_stuff::variant
#define BIGGEST // VAR gets the biggest of passed to variant types
#define INDICES // works with variant<...>::getTag(), getIndex<T> etc
#define COMMON // uses contains, getIndex
#include <type_traits> // uses std::conditional, std::is_same
#endif // VAR

#ifdef BIGGEST
#define COMMON // BIGGEST uses unvoid to ease impementation of var_stuff::variant
#include <type_traits> // uses std::conditional
#endif // BIGGEST

#ifdef INVOKE
#include <functional> // INVOKE traits check if passed type is invocable by casting it to std::function
#include <type_traits> // uses std::enable_if, std::is_constructible
#endif // INVOKE

#ifdef COMMON
#include <tuple> // COMMON uses std::tuple_element for indexing into parameter pack
#include <type_traits> // uses std::conditional, std::declval etc.
#endif // COMMON

namespace var_stuff {
	#ifdef COMMON
	struct Fail {}; // may be used for implementing 
	struct WasVoid {};

	template<typename T>
	struct unvoid {
		using type = T;
	};

	template<>
	struct unvoid<void> {
		using type = WasVoid;
	};

	template<typename T, typename... Ts>
	struct contains {
		static constexpr bool value = false;
	};

	template<typename T, typename... Ts>
	struct contains<T, T, Ts...> {
		static constexpr bool value = true;
	};

	template<typename T, typename U, typename... Ts>
	struct contains<T,U,Ts...> : contains<T,Ts...> {};

	template<typename OnContain, typename OnNotContain, typename SearchType, typename... Ts>
	struct conditionalContains {
		using type = typename std::conditional<contains<SearchType, Ts...>::value, OnContain, OnNotContain>::type;
	};

	template<typename Obj, typename... Args>
	struct returnType {
		using type = decltype(std::declval<Obj>()(std::declval<Args>()...));
	};
	#endif // COMMON

	#ifdef INTEGER_SEQUENCE
	template<int... Ints>
	struct pack{};

	template<int... Ints>
	struct _int_seq {};

	template<int res, int n, int nfin, int... Ints>
	struct _int_seq<res, n, nfin, Ints...>
	: _int_seq<n-nfin-1, n-1, nfin, n-1, Ints...> 
	{};
	
	template<int n, int nfin, int... Ints>
	struct _int_seq<0, n, nfin, Ints...> {
		using a = pack<Ints...>;
	};
	
	#endif // INTEGER_SEQUENCE
	
	#ifdef INDICES
	template<int N, typename T, typename... Ts>
	struct Indices {};
	
	template<int N, typename T, typename... Ts>
	struct Indices<N, T, T, Ts...> {
		static constexpr int value = N;
	};
	
	template<int N, typename T, typename U, typename... Ts>
	struct Indices<N, T, U, Ts...>: Indices<N+1, T, Ts...> 
	{};

	template<int N, typename... Ts>
	struct getTypeAt {
		using type = typename std::tuple_element<N, std::tuple<Ts...>>::type;
	};

	template<typename T, typename... Ts>
	auto getIndex() -> decltype(Indices<0, T, Ts...>::value) { 
		return Indices<0, T, Ts...>::value; 
	}
	#endif // INDICES
	
	#ifdef BIGGEST
	template<typename...>
	struct biggest{};
	
	template<typename T1>
	struct biggest<T1> {
		using type = typename unvoid<T1>::type;
	};
	
	template<typename T1, typename T2>
	struct biggest<T1,T2> {
		using T1U = typename unvoid<T1>::type;
		using T2U = typename unvoid<T2>::type;
		using type = typename std::conditional<sizeof(T1U) >= sizeof(T2U), T1U, T2U>::type;
	};
	
	template<typename T1, typename T2, typename... Ts>
	struct biggest<T1, T2, Ts...> {
		using type = typename biggest< typename biggest<T1,T2>::type, typename biggest<T2,Ts...>::type>::type;
	};
	#endif // BIGGEST
	
	#ifdef VAR
	template<typename... Ts>
	class var {
	private:	
		char _content[sizeof(typename biggest<Ts...>::type)];
		int _tag;
		
	public:
		template<typename T>
		var(const T& value) {
			set(value);
		}
		var(const var& other) = default;
		var(var&& other) = default;

		template<typename T>
		auto getIndex() -> decltype(getIndex<T,Ts...>()) {
			return ::var_stuff::getIndex<T, Ts...>();
		}
		
		auto getTag() -> decltype(_tag) {
			return _tag;
		}

		template<typename T>
		bool is() { return _tag == getIndex<T>(); }

		template<int Ind>
		bool is() { return _tag == Ind; }

		template<typename T>
		T* get() {
			return (T*)this->_content;
		}

		template<typename T>
		T* getIf() {
			return is<T>() ? get<T>() : nullptr;
		}

		template<typename T>
		auto set(const T& value) -> decltype(_tag){
			*(T*)_content = value;
			return (_tag = getIndex<T>());
		}
	};

	template<typename T, typename... Ts>
	struct toTypeIfVarContains {
		using type = typename std::conditional<contains<T, Ts...>::value, T, var<Ts...>>::type;
	};

	template<typename T, typename... Ts>
	struct failIfContains {
		using type = typename std::conditional<contains<T, Ts...>::value, Fail, T>::type;
	};
	
	template<typename Obj, typename... Args>
	struct failIfReturnVoid {
		using _returnType = typename returnType<Obj, Args...>::type;
		using type = typename std::conditional<std::is_same<void, _returnType>::value, Fail, _returnType >::type;
	};

	#endif // VAR


	#ifdef INVOKE
	template<typename T, typename Ret = void, typename... Args>
	struct is_invocable {
		using type = typename std::enable_if<
			std::is_constructible<std::function<Ret(Args...)>, T>::value,
			T>::type;
	};

	
	template<typename T, typename Ret = void, typename... Args>
	struct is_invocable_gran {};

	template<typename T>
	struct is_invocable_gran<T,void>: is_invocable<T, void> {};

	template<typename T, typename Ret>
	struct is_invocable_gran<T, Ret> {
		using type = typename is_invocable<T, Ret>::type;
	};
	template<typename T, typename Ret, typename Arg>
	struct is_invocable_gran<T, Ret, Arg> {
		using type = typename is_invocable<T, Ret, Arg>::type;
	};

	template<typename T, typename Ret, typename Arg, typename Arg1, typename... Args>
	struct is_invocable_gran<T, Ret, Arg, Arg1, Args...> {
		using type = typename is_invocable_gran<
				typename is_invocable<T, Ret, Arg>::type,
				Ret,
				Arg1,
				Args...
			>::type;
	};
	#endif // INVOKE
	
	#ifdef VISIT
	template<typename Ret, typename Visitor, typename Variant, int NStart, typename... Args>
	struct visitJumper {
		static typename conditionalContains<void, Ret, void, Ret>::type jump(Visitor vis, Variant& var) {
		}
	};

	template<typename Ret, typename Visitor, typename Variant, int NStart, typename Arg0>
	struct visitJumper<Ret, Visitor, Variant, NStart, Arg0> {
		static typename conditionalContains<void, Ret, void, Ret>::type jump(Visitor vis, Variant& var) {
			//if(NStart == var.getTag())
			return vis(*var.template get<Arg0>());
		}
	};

#define vjumperSwitchCase(var, order) case NStart+order: return vis(*var.template get<Arg##order>())
	template<typename Ret, typename Visitor, typename Variant, int NStart, typename Arg0, typename Arg1>
	struct visitJumper<Ret, Visitor, Variant, NStart, Arg0, Arg1> {
		static typename conditionalContains<void, Ret, void, Ret>::type jump(Visitor vis, Variant& var) {
			switch(var.getTag()) {
				vjumperSwitchCase(var,0); vjumperSwitchCase(var,1);
				// in these functions we cannot provide 'default: return' because var_stuff::visit can return non-default-constructible object
				// default: return typename std::conditional<std::is_same<void, Ret>::value, void, Fail>::type{};
			}
		}
	};

	template<typename Ret, typename Visitor, typename Variant, int NStart, typename Arg0, typename Arg1, typename Arg2>
	struct visitJumper<Ret, Visitor, Variant, NStart, Arg0, Arg1, Arg2> {
		static typename conditionalContains<void, Ret, void, Ret>::type jump(Visitor vis, Variant& var) {
			switch(var.getTag()) {
				vjumperSwitchCase(var,0); vjumperSwitchCase(var,1); vjumperSwitchCase(var,2);
			}
		}
	};

	template<typename Ret, typename Visitor, typename Variant, int NStart, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
	struct visitJumper<Ret, Visitor, Variant, NStart, Arg0, Arg1, Arg2, Arg3> {
		static typename conditionalContains<void, Ret, void, Ret>::type jump(Visitor vis, Variant& var) {
			switch(var.getTag()) {
				vjumperSwitchCase(var,0); vjumperSwitchCase(var,1); vjumperSwitchCase(var,2);
				vjumperSwitchCase(var,3);
			}
		}
	};

	template<typename Ret, typename Visitor, typename Variant, int NStart, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	struct visitJumper<Ret, Visitor, Variant, NStart, Arg0, Arg1, Arg2, Arg3, Arg4> {
		static typename conditionalContains<void, Ret, void, Ret>::type jump(Visitor vis, Variant& var) {
			switch(var.getTag()) {
				vjumperSwitchCase(var,0); vjumperSwitchCase(var,1); vjumperSwitchCase(var,2);
				vjumperSwitchCase(var,3); vjumperSwitchCase(var,4);
			}
		}
	};

	template<typename Visitor, typename T, typename... Ts >
	typename returnType<Visitor, T>::type visit(Visitor vis, var<T, Ts...> variant) {
		return visitJumper<typename returnType<Visitor, T>::type, Visitor, var<T, Ts...>, 0, T, Ts...>
			::jump(vis, variant);
	}
	#endif // VISIT
	
	#ifdef VAR_KINDS
	template<typename T>
	struct option: var_stuff::variant<WasVoid, T> {};
	
	template<typename ErrType, typename SuccessType>
	struct result: var_stuff::variant<ErrType, SuccessType> {};
	#endif // VAR_KINDS

	#ifdef OVERLOAD
	template <class... Fs>
	struct overload;
	
	template <class F0, class... Frest>
	struct overload<F0, Frest...> : F0, overload<Frest...>
	{
	    overload(F0 f0, Frest... rest) : F0(f0), overload<Frest...>(rest...) {}
	
	    using F0::operator();
	    using overload<Frest...>::operator();
	};
	
	template <class F0>
	struct overload<F0> : F0
	{
	    overload(F0 f0) : F0(f0) {}
	
	    using F0::operator();
	};
	
	template <class... Fs>
	auto make_overload(Fs... fs)
	{
	    return overload<Fs...>(fs...);
	}
	#endif // OVERLOAD
} // namespace var_stuff

#endif // VAR_STUFF_H_
