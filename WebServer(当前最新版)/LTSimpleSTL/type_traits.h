//@Author: Lin Tao
//@Email: putaopu@qq.com

#pragma once
//���ͷ�ļ�������ȡ������Ϣ
//�ǵ�һ����д��ͷ�ļ���

#include <type_traits>//��Щû��ģ�庯����ʱû��ʵ��

namespace LT
{

	
	//������ʽʵ�֣�
	//struct true_type{};
	//struct false_type{};

	//c++11֮���ṩ���µ�ʵ��
	
	template <typename T, T Var>
	struct integral_constant
	{
		using type = T;
		using value_type = integral_constant;

		static constexpr T value = Var;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; } // C++14 ��
	};
	
	
	template <bool Val>
	using bool_constant = integral_constant<bool, Val>;

	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;
	
	//������Ƕ��������û����ʾtrue��false�����������ﶨ��ṹ���ʾtrue_type��false_type��
	//ȱʡ�����, ��Щ���Զ�������ص�ֵ, ��������������ϸ�����, ����ģ���ػ�, 
	//����ϸ�������趨�����ֹ۵�ֵ.�ȷ�����int���͵Ķ���ģ���ػ�:

	//�µ�ʵ����c++11֮���ṩ��true_type��false_type�����е����������б�ʱ�����õ��µĴ���true false��ʵ��
	

	//-------------------------------��ȡ����----------------------------------------
	template<class T>
	struct _type_traits
	{
		typedef false_type		has_trivial_default_counstructor;
		typedef false_type		has_trivial_copy_constructor;
		typedef false_type		has_trivial_assignment_operator;
		typedef false_type		has_trivial_destructor;
		typedef false_type		is_POD_type;//������������
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
	struct _type_traits<wchar_t> //wchar_t��C/C++���ַ����ͣ���һ����չ�Ĵ洢��ʽ��wchar_t������Ҫ���ڹ��ʻ������ʵ���У���������ͬ��unicode���롣unicode������ַ�һ����wchar_t���ʹ洢
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


	//------------------------------ָ��------------------------------
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

	//****************************************************���ͷֱ溯��**********************************************************
	//�ֱ��Ƿ���pair����
	template<class T1, class T2>
	struct pair;
	
	template<class T>
	struct is_pair:false_type{};

	template<class T1, class T2>
	struct is_pair<pair<T1,T2>> :true_type {};

	//�ֱ��һ���б��Ƿ�Ϊ�棬Ϊ�����ṩһ�����ͣ������ṩ����
	template < bool, typename T = void>
		struct enable_if {};
	    template < typename T>
		struct enable_if<true, T> {
		  using type = T;
	};


	//is_convertible:�ж��Ƿ���Խ�������ת��
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
	//******************************************************���ʹ�����*******************************************************
	//remove_cv:ȥ�����͵�const
	template <class T> struct remove_cv { typedef T type; };
	template <class T> struct remove_cv<T const> { typedef T type; };
	template <class T> struct remove_cv<T volatile> { typedef T type; };
	template <class T> struct remove_cv<T const volatile> { typedef T type; };
	
}