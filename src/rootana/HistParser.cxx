/// \file HistParser.cxx
/// \author G. Christian
/// \brief Implements HistParser.hxx
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <TROOT.h>
#include <TDirectory.h>
#include "utils/definitions.h"
#include "utils/Error.hxx"
#include "Histos.hxx"
#include "Events.hxx"
#include "HistParser.hxx"


// HELPER FUNCTIONS & CLASSES //

namespace {

struct replace_tab { 
	void operator() (char& c)
		{ if (c == '\t') c = ' '; }
};

inline bool contains(const std::string& s, const char* arg)
{
	return s.find(arg) < s.size();
}

inline Int_t get_type(const std::string& spar)
{
	if      (contains(spar, "rootana::gHead"))  return DRAGON_HEAD_EVENT;
	else if (contains(spar, "rootana::gTail"))  return DRAGON_TAIL_EVENT;
	else if (contains(spar, "rootana::gCoinc")) return DRAGON_COINC_EVENT;
	else return -1;
}

inline void throw_bad_line(const std::string& line, int linenum, const char* fname)
{
	std::stringstream error;
	error << "Bad line in file \"" << fname << "\": "<< line << ", line number: " << linenum;
	throw std::invalid_argument (error.str().c_str());
}	

inline void throw_missing_arg(const char* which, int linenum, const char* fname)
{
	std::stringstream error;
	error << "Missing \"" << which << "\" argument in file \"" << fname << "\" at line " << linenum;
	throw std::invalid_argument (error.str().c_str());
}

}


// HIST PARSER //

rootana::HistParser::HistParser(const char* filename, TDirectory* owner):
	fFilename(filename), fFile(filename),
	fLine(""), fLineNumber(0), fDir(""),
	fLastHist(0), fOwner(owner)
{
	/*!
	 *  \param filename Path to the histogram definition file
	 *
	 *  Prints error message if file name is invalid
	 */
	
	if(!is_good()) {
		std::stringstream error;
		error << "Invalid file path: " << filename;
		throw std::invalid_argument(error.str().c_str());
	}
}

bool rootana::HistParser::read_line()
{
	/*!
	 * In addition to reading the line, does the following:
	 */
	std::getline(fFile, fLine);
	fLineNumber++;

	/// - Ignore everything after the first \c #
	size_t ppos = fLine.find('#');
	if (ppos < fLine.size())
		fLine = fLine.substr(0, ppos);

	/// - Replace each tab with a space
	std::for_each(fLine.begin(), fLine.end(), replace_tab());

	/// \returns false if at eof, true otherwise
	return is_good();
}

void rootana::HistParser::handle_dir()
{
	read_line();
	if(!is_good()) throw_missing_arg("DIR:", fLineNumber, fFilename);
	fDir = fLine;
	std::cout << "\n";
	dragon::err::Info("HistParser")	<< "New directory: " << fDir;
}

void rootana::HistParser::handle_hist(const char* type)
{
	int npar = atoi ( std::string(type).substr(2,1).c_str() );
	assert (npar > 0 && npar < 4);
	unsigned lhst, lpar[3];
	std::string shst, spar[3];
	{
		unsigned* lll[2] = { &lhst, lpar };
		std::string* lread[2] = { &shst, spar };

		read_line();
		if(!is_good()) throw_missing_arg("HIST:", fLineNumber, fFilename);
		*(lll[0])   = fLineNumber;
		*(lread[0]) = fLine;

		for (int i=0; i< npar; ++i) {
			read_line();
			if(!is_good()) throw_missing_arg("HIST:", fLineNumber, fFilename);
			unsigned* lll1 = lll[1];
			lll1[i] = fLineNumber;
			std::string* lread1 = lread[1];
			lread1[i] = fLine;
		}
	}

	std::stringstream cmdHst;
	cmdHst << "new " << type << shst << ";";
	TH1* hst = (TH1*)gROOT->ProcessLineFast(cmdHst.str().c_str());
	if(!hst) throw_bad_line(shst, lhst, fFilename);

	std::stringstream cmdParam[3];
	rootana::DataPointer* data[3];
	for (int i=0; i< npar; ++i) {
		cmdParam[i] << "rootana::DataPointer::New(" << spar[i] << ");";
		data[i] = (rootana::DataPointer*)gROOT->ProcessLineFast(cmdParam[i].str().c_str());
		if (!data[i]) throw_bad_line (spar[i], lpar[i], fFilename);
	}

	rootana::HistBase* h = 0;
	switch(npar) {
	case 1: h = new rootana::Hist<TH1D> ((TH1D*)hst, data[0]); break;
	case 2: h = new rootana::Hist<TH2D> ((TH2D*)hst, data[0], data[1]); break;
	case 3: h = new rootana::Hist<TH3D> ((TH3D*)hst, data[1], data[1], data[2]); break;
	default: assert (0 && "Shouldn't get here!");
	}

	Int_t type_code = get_type(spar[0]);
	for(int i=1; i< npar; ++i) {
		Int_t type_i = get_type(spar[i]);
		if(type_i != type_code) {
			dragon::err::Error("HistParser") << "Mixed event types.";
			throw_bad_line (spar[i], lpar[i], fFilename);
		}
	}
	add_hist(h, type_code);
}

void rootana::HistParser::handle_summary()
{
	unsigned lhst, lpar, lnum;
	std::string shst, spar, snum;
	{
		unsigned* lll[3] = { &lhst, &lpar, &lnum };
		std::string* lread[3] = { &shst, &spar, &snum };
		for (int i=0; i< 3; ++i) {
			read_line();
			if(!is_good()) throw_missing_arg("SUMMARY:", fLineNumber, fFilename);
			*(lll[i])   = fLineNumber;
			*(lread[i]) = fLine;
		}
	}

	std::stringstream cmdData;
	cmdData << "rootana::DataPointer::New(" << spar << ", " << snum << ");";
	rootana::DataPointer* data = (rootana::DataPointer*)gROOT->ProcessLineFast(cmdData.str().c_str());
	if (!data) throw_bad_line(spar, lpar, fFilename);

	std::stringstream cmdHist;
	cmdHist << "new TH1D" << shst << ";";
	TH1D* hst = (TH1D*)gROOT->ProcessLineFast(cmdHist.str().c_str());
	if (!hst) throw_bad_line(shst, lhst, fFilename);

	rootana::SummaryHist* h = new rootana::SummaryHist(hst, data);
	assert(h);

	Int_t type = get_type(spar);
	add_hist(h, type);
}

void rootana::HistParser::handle_cut()
{
	if (!fLastHist) {
		std::stringstream err;
		err << "CUT: line without a prior histogram, in file " << fFilename << " at line " << fLineNumber;
		throw(std::invalid_argument(err.str().c_str()));
	}

	if(!read_line()) throw_missing_arg("CUT:", fLineNumber, fFilename);
	


	std::cout << "\t\t";
	dragon::err::Info("HistParser")
		<< "Applying cut condition: " << fLine << " to histogram " << fLastHist->name();
}

void rootana::HistParser::add_hist(rootana::HistBase* hst, Int_t type)
{
	rootana::EventHandler::Instance()->AddHisto(hst, type, fOwner, fDir.c_str());
	fLastHist = hst;

	std::cout << "\t";
	dragon::err::Info("HistParser")
		<< "Adding histogram " << hst->name() << " to directory " << fDir;
}

void rootana::HistParser::run()
{
	while (read_line()) {
		if      (contains(fLine, "DIR:"))     handle_dir();
		else if (contains(fLine, "CUT:"))     handle_cut();
		else if (contains(fLine, "TH1D:"))    handle_hist("TH1D");
		else if (contains(fLine, "TH2D:"))    handle_hist("TH2D");
		else if (contains(fLine, "TH3D:"))    handle_hist("TH3D");
		else if (contains(fLine, "SUMMARY:")) handle_summary();
		else continue;
	}
	std::cout << "\n";
	dragon::err::Info("rootana::HistParser")
		<< "Done creating histograms from file " << fFilename << std::endl;
}		



#include <TSystem.h>
void rootana::Cut::Create(std::list<std::string>& lines)
{

// int GetPathInfo(const char* path, Long_t* id, Long_t* size, Long_t* flags, Long_t* modtime)
// returns 1 if no file, 0 otherwise

	if(lines.empty()) return;
	int ncuts = 0;

	{
		std::ofstream ofs("Cuts.C");
		ofs << "#include \"rootana/Cut.hxx\"\n";
		ofs << "namespace rootana {\n\n";
	
		for (std::list<std::string>::iterator it = lines.begin(); it!= lines.end(); ++it) {
			ofs << "struct Cut" << ncuts++ << ": public rootana::Cut { bool operator() () const {\n";
			ofs << "  return " << *it << ";\n";
			ofs << "} };\n\n";
		}
		ofs << "}\n";
	}
	
	gSystem->AddIncludePath("-I\"/home/dragon/packages/dragon/analyzer/src\"");
	gROOT->ProcessLine(".L Cuts.C+");
	gSystem->Exec("rm -f Cuts_C.d");

	std::vector<rootana::Cut*> vCuts;
	for (int i=0; i< ncuts; ++i) {
		std::stringstream cmd; cmd << "new rootana::Cut" << i << "();";
		rootana::Cut* cut = (rootana::Cut*)gROOT->ProcessLineFast(cmd.str().c_str());
		assert (cut);
		vCuts.push_back(cut);
		std::string out = (*cut)() ? "true" : "false";
		std::cout << out << "\n";
	}

	gHead.bgo.q[0] = 4000; gHead.bgo.q[1] = 4001;
	for (int i=0; i< ncuts; ++i) {
		std::string out = (*(vCuts[i]))() ? "true" : "false";
		std::cout << out << "\n";
	}
}

void rootana::Cut::Test() {
	std::list<std::string> lines_;
	lines_.push_back("gHead.bgo.q[0] < 3000");
	lines_.push_back("gHead.bgo.q[0] > 300");
	lines_.push_back("gHead.bgo.q[1] > 3000");
	Create(lines_);
}

