//! \file Vme.hxx
//! \brief Defines classes relevant to unpacking data from VME modules.
#ifndef  DRAGON_VME_HXX
#define  DRAGON_VME_HXX
#include <stdint.h>
#include <cstring>

/// Encloses all VME related classes
namespace vme {

/// "Empty" code used when no data is present in a channel.
#ifdef VME_NO_DATA
const int16_t NONE = VME_NO_DATA;
#else
const int16_t NONE = -1;
#endif

#ifndef __MAKECINT__
/// Reset a VME module data to default values (vme::NONE).
/// \tparam T The module type being reset. Must have an array called "data" with N entries
/// \tparam N The template argument of \e T
/// \param [out] module The module you want to reset
template <template <int N_CH> class T, int N>
void reset(T<N>& module)
{
	for(int i=0; i< N; ++i)
		 module.data[i] = vme::NONE;
}

/// Reset a VME module data to default values (vme::NONE).
/// \tparam T The module type being reset. Must have an array called "data" with N entries
/// \tparam N The template argument of \e T
/// \param [out] module The module you want to reset
template <class T, int N>
void reset(T& module)
{
	for(int i=0; i< N; ++i)
		 module.data[i] = vme::NONE;
}

/// Copy VME module data to an external array
/// \tparam T The module type being reset. Must have an array called "data" with N entries
/// \tparam N The template argument of \e T
/// \param [in] module Module from which you are copying data
/// \param [out] destination Array to which you are copying the data
/// \warning No bounds checking done; \e destination must point to an array of length >= \e N
template <template <int N_CH> class T, int N>
void copy_data(const T<N>& module, int16_t* destination)
{
  int nbytes = N * sizeof(int16_t);
  memcpy(reinterpret_cast<void*>(destination), reinterpret_cast<const void*>(module.data), nbytes);
}
#endif // #ifndef __MAKECINT__

}


// Functions to check if data values are valid (!= vme::NONE).
namespace {

template <class T>
inline bool is_valid(const T& t)
{
	return t != vme::NONE;
} 

template <class T1, class T2>
inline bool is_valid(const T1& t1, const T2& t2)
{
	return (t1 != vme::NONE && t2 != vme::NONE);
} 

template <class T1, class T2, class T3>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE);
}

template <class T1, class T2, class T3, class T4>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE && t4 != vme::NONE);
}

template <class T1, class T2, class T3, class T4, class T5>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE && t4 != vme::NONE && t5 != vme::NONE);
}

template <class T1, class T2, class T3, class T4, class T5,
					class T6>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5,
	                   const T6& t6)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE && t4 != vme::NONE && t5 != vme::NONE &&
		      t6 != vme::NONE);
}

template <class T1, class T2, class T3, class T4, class T5,
					class T6, class T7>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5,
	                   const T6& t6, const T7& t7)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE && t4 != vme::NONE && t5 != vme::NONE &&
		      t6 != vme::NONE && t7 != vme::NONE);
}

template <class T1, class T2, class T3, class T4, class T5,
					class T6, class T7, class T8>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5,
	                   const T6& t6, const T7& t7, const T8& t8)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE && t4 != vme::NONE && t5 != vme::NONE &&
		      t6 != vme::NONE && t7 != vme::NONE && t8 != vme::NONE);
}

template <class T1, class T2, class T3, class T4, class T5,
					class T6, class T7, class T8, class T9>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5,
	                   const T6& t6, const T7& t7, const T8& t8, const T9& t9)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE && t4 != vme::NONE && t5 != vme::NONE &&
		      t6 != vme::NONE && t7 != vme::NONE && t8 != vme::NONE && t9 != vme::NONE);
}

template <class T1, class T2, class T3, class T4, class T5,
					class T6, class T7, class T8, class T9, class T10>
inline bool is_valid(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5,
	                   const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10)
{
	return (t1 != vme::NONE && t2 != vme::NONE && t3 != vme::NONE && t4 != vme::NONE && t5 != vme::NONE &&
		      t6 != vme::NONE && t7 != vme::NONE && t8 != vme::NONE && t9 != vme::NONE && t10 != vme::NONE);
}

template <class T>
inline bool is_valid(T* const tArray, int len)
{
	for(int i=0; i< len; ++i) {
		if(tArray[i] == vme::NONE) {
			return false;
		}
	}
	return true;
}

}


#endif
