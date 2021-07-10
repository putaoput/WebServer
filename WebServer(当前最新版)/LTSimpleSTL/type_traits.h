//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//这个头文件用于提取类型信息
//是第一个编写的头文件，

#include <type_traits>//有些没有模板函数暂时没有实现

namespace LT
{

	
	//这是老式实现，
	//struct true_type{};
	//struct false_type{};

	//c++11之后提供了新的实现
	
	template <typename T, T Var>
	struct integral_constant
	{
		using type = T;
		using value_type = integral_constant;

		static constexpr T value = Var;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; } // C++14 起
	};
	
	
	template <bool Val>
	using bool_constant = integral_constant<bool, Val>;

	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;
	
	//由于内嵌类型申明没法表示true和false，我们在这里定义结构体表示true_type和false_type。
	//缺省情况下, 这些特性都依照最保守的值, 接下来再依据详细的情况, 利用模版特化, 
	//对详细的类型设定更加乐观的值.比方内置int类型的定义模版特化:

	//新的实现用c++11之后提供的true_type和false_type，进行迭代器类型判别时，会用到新的带有true false的实现
	

	//-------------------------------萃取类型----------------------------------------
	template<class T>
	struct _type_traits
	{
		typedef false_type		has_trivial_default_counstructor;
		typedef false_type		has_trivial_copy_constructor;
		typedef false_type		has_trivial_assignment_operator;
		typedef false_type		has_trivial_destructor;
		typedef false_type		is_POD_type;//基本数据类型
	};

	template<>
	struct _type_traits<bool>
	{
		typedef true_type		has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<char>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<unsigned char>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<signed char>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<wchar_t> //wchar_t是C/C++的字符类型，是一种扩展的存储方式。wchar_t类型主要用在国际化程序的实现中，但它不等同于unicode编码。unicode编码的字符一般以wchar_t类型存储
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<short>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<unsigned short>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<int>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<unsigned int>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<long>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<unsigned long>
	{
		typedef true_type	    has_trivial_default_counstructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	template<>
	struct _type_traits<long long>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<unsigned long long>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<float>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<double>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<long double>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};


	//------------------------------指针------------------------------
	template<class T>
	struct _type_traits<T*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<class T>
	struct _type_traits<const T*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<char*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<unsigned char*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<signed char*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<const char*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<const unsigned char*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};
	template<>
	struct _type_traits<const signed char*>
	{
		typedef true_type		has_trivial_default_constructor;
		typedef true_type		has_trivial_copy_constructor;
		typedef true_type		has_trivial_assignment_operator;
		typedef true_type		has_trivial_destructor;
		typedef true_type		is_POD_type;
	};

	//****************************************************类型分辨函数**********************************************************
	//分辨是否是pair类型
	template<class T1, class T2>
	struct pair;
	
	template<class T>
	struct is_pair:false_type{};

	template<class T1, class T2>
	struct is_pair<pair<T1,T2>> :true_type {};

	//分辨第一个判别是否为真，为真则提供一个类型，否则不提供类型
	template < bool, typename T = void>
		struct enable_if {};
	    template < typename T>
		struct enable_if<true, T> {
		  using type = T;
	};


	//is_convertible:判断是否可以进行类型转换
		template <typename T> true_type __test(T);

		template <typename T> false_type __test(...);

		template <typename From,
			typename To,
			bool = std::is_void<From>::value,
			bool = std::is_void<To>::value>
			struct is_convertible : decltype(__test<To>(std::declval<From>())) {};

		template <typename From, typename To>
		struct is_convertible<From, To, true, true> : true_type {};

		template <typename From, typename To>
		struct is_convertible<From, To, false, true> : false_type {};

		template <typename From, typename To>
		struct is_convertible<From, To, true, false> : false_type {};
	//******************************************************类型处理函数*******************************************************
	//remove_cv:去掉类型的const
	template <class T> struct remove_cv { typedef T type; };
	template <class T> struct remove_cv<T const> { typedef T type; };
	template <class T> struct remove_cv<T volatile> { typedef T type; };
	template <class T> struct remove_cv<T const volatile> { typedef T type; };
	
}