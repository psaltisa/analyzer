//! \file Adc.cxx
//! \brief Implements Adc.hxx
#include <map>
#include "Adc.hxx"
#include "vme/Vme.hxx"
#include "utils/Bits.hxx"
#include "utils/Error.hxx"
#include "utils/Incrvoid.hxx"

namespace {
const int DATA_BITS    = 0x0; // 0 0 0
const int HEADER_BITS  = 0x2; // 0 1 0
const int FOOTER_BITS  = 0x4; // 0 0 1
const int INVALID_BITS = 0x6; // 0 1 1
typedef void (* AdcUnpacker) (uint32_t, vme::caen::Adc<32>&);
typedef std::map<int, AdcUnpacker> AdcUnpackerMap_t;

inline bool run_adc_unpacker(int which, uint32_t data, vme::caen::Adc<32>& module)
{
	static AdcUnpackerMap_t unpackers;
	if(!unpackers.size()) {
		unpackers.insert(std::make_pair(DATA_BITS, &vme::caen::unpack_adc_data));      
		unpackers.insert(std::make_pair(HEADER_BITS, &vme::caen::unpack_adc_header));  
		unpackers.insert(std::make_pair(FOOTER_BITS, &vme::caen::unpack_adc_footer));  
		unpackers.insert(std::make_pair(INVALID_BITS, &vme::caen::handle_adc_invalid));
	}
	if(!unpackers.count(which)) return false;
	unpackers.find(which)->second (data, module);
	return true;
}

} // namespace

void vme::caen::unpack_adc_data(uint32_t data, Adc<32>& module)
{
	uint16_t ch = (data >> 16) & READ5;
	if(32 < ch) {
		std::cerr << ERR_FILE_LINE;
		err::Error("unpack_adc_data")
			<< "Read a channel number (" << ch << ") which is >= the maximum (32), aborting.\n";
		throw std::invalid_argument("Invalid channel number.");
	}
	module.underflow = (data >> 13) & READ1;
	module.overflow  = (data >> 12) & READ1;
	module.data[ch]  = (data >>  0) & READ12;
}

void vme::caen::unpack_adc_header(uint32_t data, Adc<32>& module)
{
	module.n_present = (data >> 6) & READ8;
}

void vme::caen::unpack_adc_footer(uint32_t data, Adc<32>& module)
{
	module.count = (data >> 0) & READ24;
}

void vme::caen::handle_adc_invalid(uint32_t data, Adc<32>& module)
{
	err::Error("handle_adc_invalid")
		 << "Read INVALID_BITS code from a CAEN ADC output buffer."
		 << ERR_FILE_LINE;
}

bool vme::caen::unpack_adc_buffer(void* address, const char* bank, vme::caen::Adc<32>& module)
{
	bool success;
	try {
		uint32_t* data32 = reinterpret_cast<uint32_t*>(address);
		uint32_t type = (*data32 >> 24) & READ3;
		success = run_adc_unpacker(type, *data32, module);
		if(!success) {
			std::cerr << ERR_FILE_LINE;
			err::Error("unpack_adc_buffer")
				 << "Unknown CAEN ADC buffer type (bits 24, 25, 26 = "
				 << ((*data32 >> 24) & READ1) << ", " << ((*data32 >> 25) & READ1) << ", "
				 << ((*data32 >> 26) & READ1) << ").\n";
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << " MIDAS Bank name: " << bank << "\n\n";
		success = false;
	}
	return success;
}

bool vme::caen::unpack_adc(const dragon::MidasEvent& event, const char* bank, vme::caen::Adc<32>& module)
{
	void* p_bank = NULL;
  int bank_len, bank_type;
  int found = event.FindBank(bank, &bank_len, &bank_type, &p_bank);
  if(!found) return false;

  // Loop over all buffers int the bank
	bool ret = true;
  for(int i=0; i< bank_len; ++i) {
    bool success = unpack_adc_buffer(p_bank, bank, module);
		if(!success) ret = false;
    increment_void(p_bank, bank_type);
  }
  return ret;
}

// for testing //
#ifdef ADC_TEST
int main() {

	vme::caen::V792 qdc;
	vme::reset (qdc);
	std::cout << qdc.data[25] << "\n";
	qdc.data[10] = 123;
	short data[32];
	vme::copy_data(qdc, data);
	std::cout << data[10] << "\n";

	try {
		vme::caen::unpack_adc_data(12, qdc);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << "Bank name: " << "VADC" << "\n\n";
	}
}
#endif
