/*!
 * \file Cut.hxx
 * \author G. Christian
 * \brief Defines an abstract "cut" class for rootana histograms.
 */
#ifndef ROOTANA_CUT_HXX
#define ROOTANA_CUT_HXX
#include <list>
#include "Globals.h"

namespace rootana {

/// Abstract cut (gate) class
/*!
 *  Derived classes should implement the operator() to set a cut condition.
 *  Typically one want to make cuts depending on values of the global gCoinc, gHead, & gTail,
 *  so there is no accomodation for parameter arguments as these can be used directly.
 */
class Cut {
public:
	/// Empty
	Cut()
		{
			/*!
			 * \note Non-empty constructors in derived classes will invalidate
			 * the intended usage of rootana::HistBase::set_cut()
			 */
		}

	/// Empty
	virtual ~Cut() { }

	/// Determines cut condition
	virtual bool operator() () const = 0;

	static void Create(std::list<std::string>& lines);
	static void Test();
};

/// \todo Add support for non-rectangular condidions (TCutG).

} // namespace rootana



#endif
