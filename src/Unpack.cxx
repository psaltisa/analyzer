///
/// \file Unpack.cxx
/// \author G. Christian
/// \brief Implements Unpack.hxx
///
#include "utils/definitions.h"
#include "midas/Event.hxx"
#include "midas/Database.hxx"
#include "TStamp.hxx"
#include "Dragon.hxx"
#include "Unpack.hxx"




// ============ class dragon::Unpacker ================ //

dragon::Unpacker::Unpacker(dragon::Head* head,
													 dragon::Tail* tail,
													 dragon::Coinc* coinc,
													 dragon::Scaler* schead,
													 dragon::Scaler* sctail,
													 dragon::RunParameters* runpar,
													 tstamp::Diagnostics* tsdiag,
													 bool singlesMode):
	fCoincWindow(kCoincWindowDefault),
	fQueue(0),
	fHead(head),
	fTail(tail),
	fCoinc(coinc),
	fHeadScaler(schead),
	fTailScaler(sctail),
	fRunpar(runpar),
	fDiag(tsdiag)
{
	if(!singlesMode)
		fQueue.reset(new tstamp::OwnedQueue<dragon::Unpacker>(kQueueTimeDefault*1e6, this));
}

dragon::Unpacker::~Unpacker()
{
	;
}

void dragon::Unpacker::FlushQueue(int flushTime)
{
	fQueue->Flush(flushTime, fDiag);
}

size_t dragon::Unpacker::FlushQueueIterative()
{
	/// \returns The size of the queue after removing the front event
	return fQueue->FlushIterative(fDiag);
}

void dragon::Unpacker::HandleBor(const char* dbname)
{
	/// - Reset head, tail scalers; run parameters; and timestamp diagnostics.
	fHeadScaler->reset();
	fTailScaler->reset();
	fRunpar->reset();
	fDiag->reset();

	/// - Read variables from the database corresponding to \e dbname (skip if dbname is NULL)
	if (dbname) {
		fHead->set_variables(dbname);
		fTail->set_variables(dbname);
		fCoinc->set_variables(dbname);
		fHeadScaler->set_variables(dbname, "head");
		fTailScaler->set_variables(dbname, "tail");
	}
}

void dragon::Unpacker::UnpackHead(const midas::Event& event)
{
	fHead->reset();       /// - Reset the class to default values.
	fHead->unpack(event); /// - Read raw data from the MIDAS event.
	fHead->calculate();   /// - Calculate abstract parameters.
	fUnpacked.push_back(DRAGON_HEAD_EVENT);
}

void dragon::Unpacker::UnpackTail(const midas::Event& event)
{
	fTail->reset();       /// - Reset the class to default values.
	fTail->unpack(event); /// - Read raw data from the MIDAS event.
	fTail->calculate();   /// - Calculate abstract parameters.
	fUnpacked.push_back(DRAGON_TAIL_EVENT);
}

void dragon::Unpacker::UnpackCoinc(const midas::CoincEvent& event)
{
	fCoinc->reset();       /// - Reset the class to default values.
	fCoinc->unpack(event); /// - Read raw data from the MIDAS event.
	fCoinc->calculate();   /// - Calculate abstract parameters.
	fUnpacked.push_back(DRAGON_COINC_EVENT);
}

void dragon::Unpacker::UnpackHeadScaler(const midas::Event& event)
{
	fHeadScaler->unpack(event); /// - Read scaler data from the midas event
	fUnpacked.push_back(DRAGON_HEAD_SCALER);
}

void dragon::Unpacker::UnpackTailScaler(const midas::Event& event)
{
	fTailScaler->unpack(event); /// - Read scaler data from the midas event
	fUnpacked.push_back(DRAGON_TAIL_SCALER);
}

void dragon::Unpacker::UnpackRunParameters(const midas::Database& db)
{
	fRunpar->read_data(&db); /// - Calculate run parameters from an ODB dump event
	fUnpacked.push_back(DRAGON_RUN_PARAMETERS);
}

std::vector<int32_t> dragon::Unpacker::UnpackMidasEvent(void* header, char* data)
{
	fUnpacked.clear();
	midas::Event::Header* evtHeader = reinterpret_cast<midas::Event::Header*>(header);

	switch (evtHeader->fEventId)
	{
	case DRAGON_HEAD_EVENT:
		{
			if(IsSinglesMode()) {
				midas::Event event(header, data, evtHeader->fDataSize);
				UnpackHead(event);
			}
			else {
				midas::Event event(header, data, evtHeader->fDataSize, fHead->variables.bk_tsc, GetCoincWindow());
				fQueue->Push(event, fDiag);
			}
			break;
		}
	case DRAGON_TAIL_EVENT:
		{
			if(IsSinglesMode()) {
				midas::Event event(header, data, evtHeader->fDataSize);
				UnpackTail(event);
			}
			else {
				midas::Event event(header, data, evtHeader->fDataSize, fTail->variables.bk_tsc, GetCoincWindow());
				fQueue->Push(event, fDiag);
			}
			break;
		}
	case DRAGON_HEAD_SCALER:
		{
			midas::Event event(header, data, evtHeader->fDataSize);
			UnpackHeadScaler(event);
			break;
		}
	case DRAGON_TAIL_SCALER:		
		{
			midas::Event event(header, data, evtHeader->fDataSize);
			UnpackTailScaler(event);
			break;
		}
	case MIDAS_BOR:
		{
			midas::Database db(data, evtHeader->fDataSize);
			UnpackRunParameters(db);
			break;
		}
	case MIDAS_EOR:
		{
			midas::Database db(data, evtHeader->fDataSize);
			UnpackRunParameters(db);
			break;
		}
	default:
		{
			utils::Warning("UnpackBuffer") << "Unkonwn event ID: " << evtHeader->fEventId;
			break;
		}
	}
	/// \returns The result of GetUnpackedCodes() after this event
	return fUnpacked;
}


void dragon::Unpacker::Process(const midas::Event& event)
{
	switch (event.GetEventId())
	{
	case DRAGON_HEAD_EVENT:
		UnpackHead(event);
		break;

	case DRAGON_TAIL_EVENT:
		UnpackTail(event);
		break;

	default:
		utils::Error("utils::Unpacker::Process")
			<< "Unknown event id: " << event.GetEventId() << ", skipping...\n";
		break;
	}
}

void dragon::Unpacker::Process(const midas::Event& event1, const midas::Event& event2)
{
	midas::CoincEvent coincEvent(event1, event2);

	if (coincEvent.fHeavyIon == 0 ||	coincEvent.fGamma == 0) {
		dragon::utils::Error("utils::unpacker::Process")
			<< "Invalid coincidence event, skipping...\n";
		return;
	}

	UnpackCoinc(coincEvent);
}

void dragon::Unpacker::Process(tstamp::Diagnostics*)
{
	fUnpacked.push_back(DRAGON_TSTAMP_DIAGNOSTICS);
}
