#ifndef DRAGON_HXX
#define DRAGON_HXX
#include "Bgo.hxx"

/// Encloses all DRAGON classes
namespace dragon {

/// A complete Dragon event
class Dragon {
public:
// ===== Detector data ===== //
   /// Bgo detector
	 Bgo bgo;

// ===== Methods ===== //
	 /// Constructor, initialize data
	 Dragon();

   /// Destructor, nothing to do
	 ~Dragon() { }

	 /// Reset all modules
	 void reset();

	 /// Set variable values from an odb file
	 void set_variables(const char* odb_file);

	 /// Unpack midas event data
	 void unpack(TMidasEvent& event);

   /// Calibrate data from a midas event
	 void calibrate();
};


} // namespace dragon


#endif