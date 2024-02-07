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
//#define VAR_THROW_RUNTIME_DEBUG_INFO // if defined, var's exceptions will contain some info about contained type that's messed up 
// DESCRIPTION + DECLARATIONS

#ifdef VISIT // defines functions for visiting var_stuff::variant; super primitive pattern matching
#define VAR // VISIT can't exist without VARIANT :)
#define COMMON // uses returnType
#include <type_traits> // uses std::conditional, std::is_move_constructible
#endif // VISIT

#ifdef VAR_KINDS // defines option, result
#define VAR // VAR_KINDS use var_stuff::variant as ancestor
#define COMMON // option uses WasVoid
#endif // VARIANT_KINDS 

#ifdef VAR // defines var_stuff::variant
#define BIGGEST // VAR gets the biggest of passed to variant types
#define INDICES // works with variant<...>::getTag(), getIndex<T> etc
#define COMMON // uses contains, getIndex, allOf_typed, allOf_valued, boolToType
#include <type_traits> // uses std::conditional, std::is_same
#include <utility> // uses std::move, std::forward for constructors and var::set()
#include <stdexcept> // uses std::logic_error for invalid copies and moves
#include <string> // uses std::string for exception messages
#endif // VAR

#ifdef BIGGEST
#define COMMON // BIGGEST uses unvoid to ease impementation of var_stuff::variant
#include <type_traits> // uses std::conditional
#endif // BIGGEST

#ifdef INVOKE
#include <functional> // INVOKE traits check if passed type is invocable by casting it to std::function <- can do better
#include <type_traits> // uses std::enable_if, std::is_constructible
#endif // INVOKE

#ifdef COMMON
#include <tuple> // COMMON uses std::tuple_element for indexing into parameter pack
#include <type_traits> // uses std::conditional, std::declval etc.
#endif // COMMON

namespace var_stuff {
	#ifdef COMMON
	struct Fail {}; // may be used for var_stuff::result
	struct WasVoid {}; // don't use it. Only for implementation

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

	// allOf_typed_placeholder
	using allOf_typed_plh = WasVoid;

	template<template<typename> class Predicate, bool PrevRes, typename... Ts>
	struct allOf_typed {};

	template<template<typename> class Predicate, typename... Ts>
	struct allOf_typed<Predicate, false, Ts...> {};

	template<template<typename> class Predicate, typename... Ts>
	struct allOf_typed<Predicate, true, Ts...> {
		using type = allOf_typed_plh;
	};

	template<template<typename> class Predicate, typename T, typename... Ts>
	struct allOf_typed<Predicate, true, T, Ts...>
	: allOf_typed<Predicate, Predicate<T>::value, Ts...> {};

	template<template<typename> class Predicate, bool PlaceholderBool, typename... Ts>
	struct allOf_valued {
		static constexpr bool value = false;
	};

	template<template<typename> class Predicate, typename... Ts>
	struct allOf_valued<Predicate, true, Ts...>: allOf_typed<Predicate, true, Ts...> {
		static constexpr bool value = true;
	};
	
	template<bool Evaluate, typename T>
	struct evaluateIfTrue {
		using type = allOf_typed_plh;
	};
	
	template<typename T>
	struct evaluateIfTrue<true, T> {
		using type = typename T::type;
	};
	
	template<bool Bool>
	struct boolToType {
		using type = typename std::conditional<Bool, std::true_type, std::false_type>::type;
	};

	template<typename T, typename U>
	struct rmrf_is_same: std::is_same<typename std::remove_reference<T>::type, typename std::remove_reference<U>::type>
	{};

	#endif // COMMON

	#ifdef INTEGER_SEQUENCE // not used here.
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
	constexpr auto getIndex() -> decltype(Indices<0, T, Ts...>::value) { 
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

	// ..Jump structs help to properly manage copies, moves, destructors
	template<typename T>
	struct DestroyJump {
		static void jump(void* const obj) {
		//      |-------| it evaluates only this part for non-class types
			((T*)obj)->T::~T();
		// doesn't even throw compile-time error! Amazing!
		}
	};

	// this thing allows failed assert to print useful info.
	// Yes, invalid ...Jumps fail at runtime.
	#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
	#define __PRETTY_FUNCTION__ __FUNCSIG__
	#elif !defined(__PRETTY_FUNCTION__)
	#define __PRETTY_FUNCTION__ __func__
	#endif
	
	// removing debug info from exceptions at release mode
	#if defined(NDEBUG) && !defined(VAR_THROW_RUNTIME_DEBUG_INFO)
	#define PRETTY_FUNCTION ""
	#else
	#define PRETTY_FUNCTION __PRETTY_FUNCTION__
	#endif

	// prints info about contained in var type, that's not suited for an operation
	void jumpThrowError_(const std::string& opName, const std::string& funcName) {
		auto errMsg = std::string("T type in templated jump can't do operation: ") 
			+ opName + " in function: " + funcName + '\n';
		throw std::logic_error(errMsg);
	}

	template<typename T, bool Enable = false>
	struct CopyJump {
		static void jump(void* const dest, void const* const obj) {
			jumpThrowError_("Copy assign", PRETTY_FUNCTION);
		}
	};

	template<typename T>
	struct CopyJump<T, true> {
		static void jump(void* const dest, void const* const obj) {
			//*(T*)dest = *(T*)obj;
			new(dest) T(*(T*)obj);
		}
	};

	template<typename T, bool Enable = false>
	struct MoveJump {
		static void jump(void* const dest, void* const obj) {
			jumpThrowError_("Move assign", PRETTY_FUNCTION);
		}
	};

	template<typename T>
	struct MoveJump<T, true> {
		static void jump(void* const dest, void* const obj) {
		//	*(T*)dest = std::move(*(T*)obj);
			new(dest) T(std::move(*(T*)obj));
		}
	};

	// used for hinting var_stuff::var to (not) check types contained at compile-time
	template<typename EnableCopyCheck_t = std::true_type, typename EnableMoveCheck_t = std::true_type, typename ContainedType = WasVoid>
	struct ParamToCheck {
		// prevents you from using default type for ContainedType
		using CheckedType = typename std::conditional<not std::is_same<ContainedType, WasVoid>::value, ContainedType, void>::type;
		CheckedType&& param;
		static constexpr EnableCopyCheck_t EnableCopyCheck{};
		static constexpr EnableMoveCheck_t EnableMoveCheck{};
		
		// ParamToCheck(ContainedType&& param_) : param(std::forward<ContainedType>(param_)) {}
	};
	
	// wrapper for forwarding reference. Turns bool expression into std::true/false_type
	template<bool EnableCopyCheck, bool EnableMoveCheck, typename ContainedType>
	auto pcheck(ContainedType&& value) -> ParamToCheck<
		typename boolToType<EnableCopyCheck>::type,
		typename boolToType<EnableMoveCheck>::type,
		ContainedType&&
	>
	{
		return ParamToCheck<
			typename boolToType<EnableCopyCheck>::type, 
			typename boolToType<EnableMoveCheck>::type, 
			ContainedType&&
		>{std::forward<ContainedType>(value)};
	}
	// wrapper for forwarding reference, disables copy and move compile-time checks
	template<typename ContainedType>
	auto nocheck(ContainedType&& value) -> ParamToCheck<std::false_type, std::false_type, ContainedType&&> {
		return ParamToCheck<std::false_type, std::false_type, ContainedType&&>{std::forward<ContainedType>(value)};
	}
	// wrapper for forwarding reference, enables copy and move compile-time checks
	// reduntant for var because it does checks by default. Just for the sake of explicitness
	template<typename ContainedType>
	auto check(ContainedType&& value) -> ParamToCheck<std::true_type, std::true_type, ContainedType&&> {
		return ParamToCheck<std::true_type, std::true_type, ContainedType&&>{std::forward<ContainedType>(value)};
	}
	// only copy check
	template<typename ContainedType>
	auto ccheck(ContainedType&& value) -> ParamToCheck<std::true_type, std::false_type, ContainedType&&> {
		return ParamToCheck<std::true_type, std::false_type, ContainedType&&>{std::forward<ContainedType>(value)};
	}
	// only move check
	template<typename ContainedType>
	auto mcheck(ContainedType&& value) -> ParamToCheck<std::false_type, std::true_type, ContainedType&&> {
		return ParamToCheck<std::false_type, std::true_type, ContainedType&&>{std::forward<ContainedType>(value)};
	}
	
	template<typename... Ts>
	class var;
	template<typename Visitor, typename U, typename... Us>
	typename returnType<Visitor, U>::type visit(Visitor&& vis, var<U, Us...>& variant);

	template<typename... Ts>
	class var {
	protected:
		using destroyJump_t = decltype(&DestroyJump<int>::jump);
		using copyJump_t = decltype(&CopyJump<int>::jump);
		using moveJump_t = decltype(&MoveJump<int>::jump);
		static constexpr destroyJump_t m_destroyJumps[] = { DestroyJump<Ts>::jump... };
		static constexpr copyJump_t m_copyJumps[] = { &CopyJump<Ts, std::is_copy_constructible<Ts>::value>::jump... };
		static constexpr moveJump_t m_moveJumps[] = { &MoveJump<Ts, std::is_move_constructible<Ts>::value>::jump... };
		char m_content[sizeof(typename biggest<Ts...>::type)];
		int m_tag;
	
		void destroyContent() {
			m_destroyJumps[getTag()](m_content);
		}

		void copy_(const var& other) {
			m_tag = other.m_tag;
			m_copyJumps[getTag()](m_content, other.m_content);
		}
		
		void move_(var&& other) {
			m_tag = other.m_tag;
			m_moveJumps[getTag()](m_content, other.m_content);
		}

		template<typename T>
		auto set_(T&& value) -> decltype(m_tag) {
			using rmrf_T = typename std::remove_reference<T>::type;
			
			//*(rmrf_T*)m_content = std::forward<T>(value);
			*(rmrf_T*)m_content = static_cast<typename std::conditional<std::is_move_constructible<rmrf_T>::value, rmrf_T&&, const rmrf_T&&>::type>(
				value
			);
			return (m_tag = this->getIndex<rmrf_T>());
		}

		var() {}
	public:
		// METHOD GROUP: COPY CTOR
		// always checking 
		var(const var& other) {
			copy_(other);
		}
		
		// conditional checking
		// this thing is meant to be called so: ...(var_stuff::check(other)); OR ...(var_stuff::nocheck(other));
		template<
			typename EnableCopyCheck_t, // default types aren't here;
			typename EnableMoveCheck_t  // otherwise, ambiguity between var(var&) and this ctor would be
		>
		var(ParamToCheck<EnableCopyCheck_t, EnableMoveCheck_t, const var&> other) {
			// TODO: check if these check-variables (but not the very checks!) are removed at -O3 level, at least.
			/* We're using these variables instead of static_cast(EXPRESSION_THAT_EVALUATES_TO_FALSE)
			   because it doesn't stop compilation... I still don't understand why. Something related to DR 2518;
			   you can read about it here: 
			   https://stackoverflow.com/questions/44059557/whats-the-right-way-to-call-static-assertfalse
			*/
			typename evaluateIfTrue<EnableCopyCheck_t::value, allOf_typed<std::is_copy_constructible, true, Ts...>>::type 
			check_for_copy{};
			typename evaluateIfTrue<EnableMoveCheck_t::value, allOf_typed<std::is_move_constructible, true, Ts...>>::type 
			check_for_move{};
			
			copy_(other.param);
		}
		
		// METHOD GROUP: MOVE CTOR
		// always checking 
		var(var&& other) {
			move_(std::move(other));
		}
		
		// conditional checking
		template<
			typename EnableCopyCheck_t, // default types aren't here;
			typename EnableMoveCheck_t  // otherwise, ambiguity between var(var&&) and this ctor would be
		>
		var(ParamToCheck<EnableCopyCheck_t, EnableMoveCheck_t, var&&> other) {
			typename evaluateIfTrue<EnableCopyCheck_t::value, allOf_typed<std::is_copy_constructible, true, Ts...>>::type 
			check_for_copy{};
			typename evaluateIfTrue<EnableMoveCheck_t::value, allOf_typed<std::is_move_constructible, true, Ts...>>::type 
			check_for_move{};
			
			move_(std::move(other.param));
		}
		
		// METHOD GROUP: CTOR FROM CONTAINED TYPE
		// always checking
		template<typename T>
		var(T&& value) {
			set_(std::forward<T>(value));
		}
		
		// conditional checking
		template<
			typename T,
			typename EnableCopyCheck_t, 
			typename EnableMoveCheck_t
		>
		var(ParamToCheck<EnableCopyCheck_t, EnableMoveCheck_t, T&&> value) {
			typename evaluateIfTrue<EnableCopyCheck_t::value, allOf_typed<std::is_copy_constructible, true, Ts...>>::type 
			check_for_copy{};
			typename evaluateIfTrue<EnableMoveCheck_t::value, allOf_typed<std::is_move_constructible, true, Ts...>>::type 
			check_for_move{};
			
			set_(std::forward<T>(value.param));
		}
		
		// METHOD GROUP: COPY ASSIGNMENT OPERATOR
		var& operator=(const var& other) {
			destroyContent();
			copy_(other);
			return *this;
		}
		
		// conditional checking
		template<
			typename EnableCopyCheck_t,
			typename EnableMoveCheck_t
		>
		var& operator=(ParamToCheck<EnableCopyCheck_t, EnableMoveCheck_t, var&> other) {
			typename evaluateIfTrue<EnableCopyCheck_t::value, allOf_typed<std::is_copy_constructible, true, Ts...>>::type 
			check_for_copy{};
			typename evaluateIfTrue<EnableMoveCheck_t::value, allOf_typed<std::is_move_constructible, true, Ts...>>::type 
			check_for_move{};
			
			destroyContent();
			copy_(std::forward<var&>(other.param));
			return *this;
		}
		
		// METHOD GROUP: MOVE ASSIGNMENT OPERATOR
		var& operator=(var&& other) {
			destroyContent();
			move_(std::move(other));
			return *this;
		}
		
		// conditional checking
		template<
			typename EnableCopyCheck_t,
			typename EnableMoveCheck_t
		>
		var& operator=(ParamToCheck<EnableCopyCheck_t, EnableMoveCheck_t, var&&> other) {
			typename evaluateIfTrue<EnableCopyCheck_t::value, allOf_typed<std::is_copy_constructible, true, Ts...>>::type 
			check_for_copy{};
			typename evaluateIfTrue<EnableMoveCheck_t::value, allOf_typed<std::is_move_constructible, true, Ts...>>::type 
			check_for_move{};
			
			destroyContent();
			move_(std::forward<var&&>(other.param));
			return *this;
		}
		
		// METHOD GROUP: ASSIGNMENT OPERATOR FROM CONTAINED TYPE
		// always checking
		template<typename T>
		var& operator=(T&& value) {
			destroyContent();	
			set_(std::forward<T>(value));
			return *this;
		}
		
		// conditional checking
		template<
			typename T,
			typename EnableCopyCheck_t, 
			typename EnableMoveCheck_t
		>
		var& operator=(ParamToCheck<EnableCopyCheck_t, EnableMoveCheck_t, T&&> value) {
			typename evaluateIfTrue<EnableCopyCheck_t::value, allOf_typed<std::is_copy_constructible, true, Ts...>>::type 
			check_for_copy{};
			typename evaluateIfTrue<EnableMoveCheck_t::value, allOf_typed<std::is_move_constructible, true, Ts...>>::type 
			check_for_move{};
		
			destroyContent();	
			set_(std::forward<T>(value.param));
			return *this;
		}

		~var() {
			destroyContent();
		}

		template<typename T>
		auto getIndex() -> decltype(getIndex<T,Ts...>()) const {
			return ::var_stuff::getIndex<T, Ts...>();
		}
		
		auto getTag() -> decltype(m_tag) const {
			return m_tag;
		}

		template<typename T>
		bool is() const { return m_tag == getIndex<T>(); }

		template<int Ind>
		bool is() const { return m_tag == Ind; }

		template<typename T>
		T* get() const {
			return (T*)this->m_content;
		}

		template<typename T>
		T* getIf() const {
			return is<T>() ? get<T>() : nullptr;
		}

		template<typename T>
		auto set(const T&& value) -> decltype(m_tag) {
			destroyContent();
			return set_(std::forward<T>(value));
		}
		
		template<typename Visitor, typename U, typename... Us>
		friend typename returnType<Visitor, U>::type visit(Visitor&& vis, var<U, Us...>& variant);
	};


	// Still don't understand why i have to write this
	template<typename... Ts> constexpr typename var<Ts...>::destroyJump_t 
		var<Ts...>::m_destroyJumps[];
	template<typename... Ts> constexpr typename var<Ts...>::copyJump_t 
		var<Ts...>::m_copyJumps[];
	template<typename... Ts> constexpr typename var<Ts...>::moveJump_t 
		var<Ts...>::m_moveJumps[];

	#endif // VAR


	#ifdef INVOKE
	// INVOKE is not needed here, but why not have it? You can still turn in on and off
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
	template<typename Obj, typename Ret, typename Arg>
	struct visitJump {
		/* Function that returns by copy when ONLY copy is available; otherwise, returns by move */
		static Ret jump(void* const obj, void* const arg) {
			// TODO: check if it's a pessimization
			return static_cast<typename std::conditional<std::is_move_constructible<Ret>::value, Ret&&, const Ret&&>::type>(
				((Obj*)obj)->operator()(*static_cast<Arg*>(arg))
			);
		}
	};
	
	template<typename Obj, typename Arg>
	struct visitJump<Obj, void, Arg> {
		static void jump(void* const obj, void* const arg) {
			return ((Obj*)obj)->operator()(*static_cast<Arg*>(arg));
		};
	};
	
	// visits any object that has operator() receiving Us as separate arguments.
	// e.g. Visitor(Us[0]), Visitor(Us[1]), ...
	template<typename Visitor, typename U, typename... Us>
	typename returnType<Visitor, U>::type visit(Visitor&& vis, var<U, Us...>& variant) {
		// ah... i have to explain
		
		/* extracts return value type from method Visitor::operator()(U);
		   this also demands that every other Visitor::operator() returns the same type.
		*/
		using Ret = typename returnType<Visitor, U>::type;
		/* we aren't allowed to have void&& (why not for the sake of generics...),
		   so we use one little trick to sooth the compiler - unvoid.
		   Nevertheless, if return type is void we still want to have it,
		   so the check 'rmrf_is_same' just gets us this void without hurting the rest of the templated 'expression'
		*/
		using RetMoveOrCopy = typename std::conditional<
			rmrf_is_same<Ret, void>::value, 
			void, 
			typename std::conditional<
				std::is_move_constructible<Ret>::value, 
				typename unvoid<Ret>::type&&, 
				const typename unvoid<Ret>::type&&
			>::type
		>::type;
		//foo(std::declval<RetMoveOrCopy>());
		
		/* These are just jumps which knows to what pointer argument must be casted;
		   for sure, you've seen this technique above the file already.
		*/
		static constexpr Ret(*jumps[])(void* const, void* const) = {
			visitJump<Visitor, Ret, U>::jump, visitJump<Visitor, Ret, Us>::jump...
		};
		
		/* why static_cast? because if we cast to 'const T&&' it is guaranteed (sorta) to call copy constructor;
		   if we cast to T&&, move constructor is called.
		*/
		return static_cast<RetMoveOrCopy>(
			jumps[variant.getTag()](&vis, variant.m_content)
		);
	}
	#endif // VISIT
	#ifdef VAR_KINDS
	template<typename T>
	struct option: var_stuff::var<WasVoid, T> {};
	
	template<typename ErrType, typename SuccessType>
	struct result: var_stuff::var<ErrType, SuccessType> {};
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
	auto make_overload(Fs... fs) -> overload<Fs...>
	{
	    return overload<Fs...>(fs...);
	}
	#endif // OVERLOAD
} // namespace var_stuff

#endif // VAR_STUFF_H_
