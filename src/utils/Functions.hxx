/// \file Functions.hxx
/// \author G. Christian
/// \brief Defines some functions commonly used in processing data
#ifndef DRAGON_UTILS_FUNCTIONS_HXX
#define DRAGON_UTILS_FUNCTIONS_HXX
#include <functional>
#include "Valid.hxx"


/// Encloses various 'utility' functions
namespace utils {

/// Calculates time-of-flight if both parameters are valid
template <class T1, class T2>
inline double calculate_tof(const T1& t1, const T2& t2)
{
	/*!
	 * \param t1 Later time
	 * \param t2 Earlier time
	 * \returns <c>t1 -t2</c> if both parameters are valid; otherwise: \c dragon::NO_DATA
	 */
	if (utils::is_valid(t1) && utils::is_valid(t2))
		return t1 - t2;
	else
		return dragon::NO_DATA;
}

/// Sums all values in an array, <e>ignoring "null" values</e>
template <class T>
inline double calculate_sum(T begin, T end)
{
	/*!
	 * Example:
	 * \code
	 * double anodes = { 300., 200., 100., 1., 0., -1. }; // dragon::NO_DATA == -1
	 * double sum = utils::sum(anodes, anodes + 6);
	 * // sum = 601.
	 * \endcode
	 * \param begin First element to sum
	 * \param end One past the last element to sum
	 * \tparam T Type contained in the array
	 */
	double sum = 0.;
	for(; begin != end; ++begin) {
		if(is_valid(*begin))
			sum += *begin;
	}
	return sum;
}

/// Fills an array with it's index values
template <class T>
inline void index_fill(T begin, T end, int offset = 0)
{
	/*!
	 * Example:
	 * \code
	 * double arr[4];
	 * vme::index_fill(arr, arr + 4);
	 * // arr = { 0., 1., 2., 3. }
	 * vme::index_fill(arr, arr + 4, 5);
	 * // arr = { 5., 6., 7., 8. }
	 * \endcode
	 * \tparam Type contained in the array
	 * \param begin Beginning of the array (vector, etc.)
	 * \param end End of the array (vector, etc.)
	 * \param offset Optional offset for the filled values
	 */
	for(int i=0; begin != end; ++i, ++begin)
		*begin = i + offset;
}

/// Fills an array with it's index values
template <class T>
inline void index_fill_n(T begin, int n, int offset = 0)
{
	/*!
	 * Example:
	 * \code
	 * double arr[4];
	 * vme::index_fill_n(arr, 4);
	 * // arr = { 0., 1., 2., 3. }
	 * vme::index_fill_n(arr, 4, 5);
	 * // arr = { 5., 6., 7., 8. }
	 * \endcode
	 * \param begin Beginning of the array (vector, etc.)
	 * \param n Number of indices past begin to fill
	 * \param offset Optional offset for the filled values
	 */
	for(int i=0; i< n; ++i, ++begin)
		*begin = i + offset;
}

// Internal classes
namespace {
// Compares index values & sortes relative to another array
template <class T, class Order>
class Isort {
	T values;
	Order compare;
public:
	Isort(T begin, Order compare_):
		values(begin), compare(compare_) { }
	bool operator() (int index1, int index2) {
		return compare(*(values + index1), *(values + index2));
	}
};
// "Default" Isort version for less-than
template <class T>
class IsortLess {
	T values;
public:
	IsortLess(T begin):
		values(begin) { }
	bool operator() (int index1, int index2) {
		return *(values + index1) < *(values + index2);
	}
};
} // namespace

/// Functor for sorting in descending order, taking into account validity
/*!
 * Overloads operator() such that invalid values are always considered "smaller"
 * than any others.
 *
 * Example:
 * \code
 * double data[5] = { -1., 12., 11., -13., -1. }
 * std::sort(data, data+5, utils::greater_and_valid<double>());
 * // data = { 12., 11., -13., -1., -1. }
 * \endcode
 */
template <class T>
struct greater_and_valid {
	bool operator() (const T& lhs, const T& rhs)
		{
			if(!is_valid(lhs)) return false;
			if(!is_valid(rhs)) return true;
			return std::greater<T>()(lhs, rhs);
		}
};

/// Functor for sorting in ascending order, taking into account validity
/*!
 * Overloads operator() such that invalid values are always considered "smaller"
 * than any others.
 *
 * Example:
 * \code
 * double data[5] = { -1., 12., 11., -13., -1. }
 * std::sort(data, data+5, utils::less_and_valid<double>());
 * // data = { -1., -1., -13., 11., 12. }
 * \endcode
 */
template <class T>
struct less_and_valid {
	bool operator() (const T& lhs, const T& rhs)
		{
			if(!is_valid(lhs)) return true;
			if(!is_valid(rhs)) return false;
			return std::less<T>()(lhs, rhs);
		}
};

/// Sort function which returns sorted indices instead of modifying the array
/*!
 * \tparam T type in the array
 * \tparam Order Functor defining the strict weak ordering of the array to sort
 *
 * \param [in] begin Beginning of the array whose sorted values you desire
 * \param [in] end End of the array whose sorted values you desire
 * \param [out] indices Pointer to the beginning of an array in which you will
 * store the sorted indices. <b>This array must contain space for every index.</b>
 * \param [in] order instance of Order class for sorting.
 *
 * Example:
 * \code
 * double data[5] = { 1., 5., -12., 21., -31. };
 * int sortedData[5];
 * utils::index_sort(data, data+5, sortedData, std::greater<double>());
 * for(int i=0; i< 5; ++i) cout << data[ sortedData[i] ] << " ";
 * cout << "\n";
 * // Output:
 * // 21 5 1 -12 -31
 * \endcode
 */
template <class T, class Order>
inline void index_sort(T begin, T end, int* indices, Order order)
{
	ptrdiff_t size = end - begin;
	index_fill(indices, indices + size);
	std::sort(indices, indices + size, 	Isort<T, Order>(begin, order));
}

/// Sort function which returns sorted indices instead of modifying the array
/*!
 * Same as index_sort(), except defaulting to std::less<> for the comparison, i.e.
 * the returned indices sort in ascending order.
 */
template <class T>
inline void index_sort(T begin, T end, int* indices)
{
	ptrdiff_t size = end - begin;
	index_fill(indices, indices + size);
	std::sort(indices, indices + size, 	IsortLess<T> (begin));
}

/// Maps raw vme data into another array
/*!
 * \tparam T Basic type of the output array
 * \tparam M Type of the vme module
 * \param [out] output Array containing the mapped values
 * \param [in] numch Number of channels in the output array
 * \param [in] channels Array containing the channel mapping
 * \param [in] module Vme module class containing the raw data
 * 
 * Example:
 * \code
 * vme::V792 adc;
 * adc.unpack(someEvent, "ADC0"); // adc.data = { 21, 24, 26, ... }
 *
 * int channels = { 2, 0, 1, ... };
 * double output[32];
 * channel_map(output, 32, channels, adc); // output = { 26, 21, 24, ... }
 * \endcode
 */
template <class T, class M>
inline void channel_map(T* output, int numch, const int* channels, const M& module)
{
	for (int i=0; i< numch; ++i) {
		output[i] = module.get_data( channels[i] );
	}
}

/// Maps raw vme data into another array, from an array of possible modules
/*!
 * \tparam T Basic type of the output array
 * \tparam M Type of the vme module
 * \param [out] output Array containing the mapped values
 * \param [in] numch Number of channels in the 
 * \param [in] channels Array containing the channel mapping
 * \param [in] channels Array containing the module mapping
 * \param [in] moduleArr Array of vme modules containing the raw data
 * 
 * Example:
 * \code
 * vme::V792[2] adc;
 * adc[0].unpack(someEvent, "ADC0"); // adc[0].data = { 21, 24, 26, ... }
 * adc[1].unpack(someEvent, "ADC1"); // adc[1].data = { 27, 22, 19, ... }
 *
 * int channels = { 2, 0, 1, ... };
 * int modules = { 0, 1, 0, ... };
 * double output[32];
 * channel_map(output, 32, channels, modules, adc); // output = { 26, 27, 24, ... }
 * \endcode
 */
template <class T, class M>
inline void channel_map(T* output, int numch, const int* channels, const int* modules, const M* moduleArr)
{
	for (int i=0; i< numch; ++i) {
		output[i] = moduleArr[ modules[i] ].get_data( channels[i] );
	}
}


/// Channel maps a single value
template <class T, class M>
inline void channel_map(T& output, int channel, const M& module)
{
	/*!
	 * See array version of channel_map()
	 */
	output = module.get_data(channel);
}

/// Channel maps a single value
template <class T, class M>
inline void channel_map(T& output, int channel, int module, const M* moduleArr)
{
	/*!
	 * See array version of channel_map()
	 */
	output = moduleArr[module].get_data(channel);
}


/// Channel mapping from a plain array, not a module
template <class T>
inline void channel_map_from_array(T* output, int numch, const int* const indices, const T* const input)
{
	/*!
	 * \param [out] output Output array
	 * \param [in] numch Length of the input array
	 * \param [in] indices Array containing mapping values
	 * \param [in] input Array of input data
	 *
	 * Example:
	 * \code
	 * double input[5] = { 1., 2., 3., 4., 5. };
	 * double output[5];
	 * int indices[5] = { 3, 2, 1, 4, 0 };
	 * utils::channel_map_from_array(output, 5, indices, input);
	 * // output = { 4., 3., 2., 5., 1. }
	 * \endcode
	 */
	for (int i=0; i< numch; ++i)
		output[i] = input[ indices[i] ];
}


/// Transforms data within an array
/*!
 * \tparam T Basic type of the output array
 * \tparam TR Type of the transformation, should either be a function returning void
 *  and taking as arguments a \c double& (the value to transform) and an \c int (the
 *  channel number) or a class overloading \c operator() to do the same.
 * \param [out] output Array containing the values to transform
 * \param [in] numch Number of channels in the output array
 * \param [in] transform Instance of the transformation class (or function)
 * 
 * Example:
 * \code
 * class LinearCalibrate {
 *   int slope, offset;
 * public:
 *   LinearCalibrate(int slope_, int offset_):
 *     slope(slope_), offset(offset_) { }
 *   void operator() (double value, int channel)
 *     { value = value * slope_ + offset_; }
 * };
 *
 * vme::V792 adc;
 * adc.unpack(someEvent, "ADC0"); // adc.data = { 21, 24, 26, ... }
 *
 * int channels = { 2, 0, 1, ... };
 * double output[32] = { 26., 21., 24. };
 * transform(output, 32, LinearCalibrate(2., 1.)); // output = { 53., 43., 49, ... }
 * \endcode
 */
template <class T, class TR>
inline void transform(T* output, int numch, TR transform)
{
	for (int i=0; i< numch; ++i) {
		transform(output[i], i);
	}
}

/// Transforms a single value
template <class T, class TR>
inline void transform(T& output, TR transform)
{
	/*!
	 * Same as the array transform(), except operating only on a single value
	 * (passed by reference);
	 */
	transform(output, 0);
}

/// Perform pedestal subtraction on an array
/*!
 * Values below the pedestal are set to vme::NONE
 *
 * \tparam T type of the values in the array
 * \tparam L type of the array length identifier
 * \tparam V variables class, must have public \c pedestal[] field
 *
 * Usage example:
 * \code
 * dragon::AdcVariables<4> vars;
 * vars.pedestal = { 32, 17, 21, 46 }; // (only allowed in C++11, but you get the point)
 * double values = { 13., 16., 21., 444. };
 * utils::pedestal_subtract(values, 4, vars);
 * // values = { -1., -1., 21., 444. }
 * // n.b. dragon::NO_DATA == -1.
 * \endcode
 */
template <class T, class L, class V>
inline void pedestal_subtract(T* array, L length, const V& variables)
{
	for (L i=0; i< length; ++i) {
		if (array[i] < variables.pedestal[i]) {
			array[i] = dragon::NO_DATA;
		}
	}
}

/// Perform pedestal subtraction on a single value
/*!
 * Values below the pedestal are set to vme::NONE
 *
 * \tparam T type of the values in the array
 * \tparam V variables class, must have public \c pedestal field
 *
 * Usage example:
 * \code
 * dragon::AdcVariables<1> vars;
 * vars.pedestal = 32.;
 * double value1 = 100., value2 = 31.;
 * utils::pedestal_subtract(value1, vars);
 * utils::pedestal_subtract(value2, vars);
 * // value1 = 100., value2 = -1.
 * // n.b. dragon::NO_DATA == -1.
 * \endcode
 */
template <class T, class V>
inline void pedestal_subtract(T& value, const V& variables)
{
	if (value < variables.pedestal)
		value = dragon::NO_DATA;
}


/// Perform linear calibration on an array
/*!
 *  New = slope * Old + offset
 *
 * \tparam T type of the values in the array
 * \tparam L type of the array length identifier
 * \tparam V variables class, must have public \c slope[] and \c offset[] fields
 *
 * Usage example:
 * \code
 * dragon::AdcVariables<3> vars;
 * vars.offset = { 0., 1., 2. }; // (only allowed in C++11, but you get the point)
 * vars.slope  = { 2., 1., 0. }; // (only allowed in C++11, but you get the point)
 * double values = { 1., 2., 3. };
 * utils::linear_calibrate(values, 3, vars);
 * // values = { 2., 3., 2. }
 * \endcode
 *
 * \note Any values initially set to dragon::NO_DATA are left untouched
 */
template <class T, class L, class V>
inline void linear_calibrate(T* array, L length, const V& variables)
{
	for (L i=0; i< length; ++i) {
		if(is_valid(array[i]))
			array[i] = variables.offset[i] + array[i] * variables.slope[i];
	}
}

/// Perform linear calibration on a single value
/*!
 *  New = slope * Old + offset
 *
 * \tparam T type of the values in the array
 * \tparam V variables class, must have public \c slope[] and \c offset[] fields
 *
 * See array version for more information.
 * \note Any values initially set to dragon::NO_DATA are left untouched
 */
template <class T,  class V>
inline void linear_calibrate(T& value, const V& variables)
{
	if(is_valid(value))
		value = variables.offset + value * variables.slope;
}


/// Perform quadratic calibration on an array
/*!
 *  New = slope2 * Old*Old + slope * Old + offset
 *
 * \tparam T type of the values in the array
 * \tparam L type of the array length identifier
 * \tparam V variables class, must have public <tt>slope2[]</tt>, <tt>slope[]</tt>, and \c offset[] fields
 *
 * Behaves identically to linear_calibrate(), except performing a quadratic calibration instead
 * \note Any values initially set to dragon::NO_DATA are left untouched
 */
template <class T, class L, class V>
inline void quadratic_calibrate(T* array, L length, const V& variables)
{
	for (L i=0; i< length; ++i) {
		if(is_valid(array[i]))
			array[i] = variables.offset[i] + array[i] * variables.slope[i] + pow(array[i], 2) * variables.slope2[i];
	}
}

/// Perform quadratic calibration on a single value
/*!
 *  New = slope2 * Old*Old + slope * Old + offset
 *
 * \tparam T type of the values in the array
 * \tparam V variables class, must have public <tt>slope2</tt>, <tt>slope</tt>, and \c offset fields
 *
 * See array version for more information.
 * \note Any values initially set to dragon::NO_DATA are left untouched
 */
template <class T, class V>
inline void quadratic_calibrate(T& value, const V& variables)
{
	if(is_valid(value))
		value =	variables.offset + value * variables.slope + pow(value, 2) * variables.slope2;
}


/// Perform nth order polynomial calibration on an array
/*!
 *  New = offset + slope * Old + slope2*Old*Old + slope3*Old*Old*Old + ...
 *
 * \tparam T type of the values in the array
 * \tparam L type of the array length identifier
 * \tparam V variables class, must have public \c coeff[][] fiels
 *
 * Behaves identically to linear_calibrate(), except performing a quadratic calibration instead
 * \note Any values initially set to dragon::NO_DATA are left untouched
 */
template <class T, class L, class V>
inline void polynomial_calibrate(int order, T* array, L length, const V& variables)
{
	for(L i=0; i< length; ++i) {
		if(is_valid(array[i])) {
			T value0 = array[i];
			array[i] = 0;
			for(int j=0; j< order; ++j) {
				array[i] += variables.coeff[j][i] * pow(value0, j);
			}
		}
	}
}

/// Perform nth order polynomial calibration on a single value
/*!
 *  New = offset + slope * Old + slope2*Old*Old + slope3*Old*Old*Old + ...
 *
 * \tparam T type of the values in the array
 * \tparam V variables class, must have public \c coeff[] field
 *
 * See array version for more information.
 * \note Any values initially set to dragon::NO_DATA are left untouched
 */
template <class T, class V>
inline void polynomial_calibrate(int order, T& value, const V& variables)
{
	if(is_valid(value)) {
		T value0 = value;
		value = 0;
		for(int j=0; j< order; ++j) {
			value += variables.coeff[j] * pow(value0, j);
		}
	}
}

} // namespace utils



#endif
