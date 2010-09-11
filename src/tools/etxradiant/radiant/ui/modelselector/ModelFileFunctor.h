#ifndef MODELFILEFUNCTOR_H_
#define MODELFILEFUNCTOR_H_

#include "gtkutil/VFSTreePopulator.h"
#include "gtkutil/ModalProgressDialog.h"
#include "imainframe.h"
#include "EventRateLimiter.h"

#include "string/string.h"
#include <boost/algorithm/string/predicate.hpp>

namespace ui
{

/* CONSTANTS */
namespace {
	const char* ASE_EXTENSION = ".ase";
	const char* LWO_EXTENSION = ".lwo";
	const char* MD5MESH_EXTENSION = ".md5mesh";
	const char* MD3_EXTENSION = ".md3";
}

/**
 * Functor object to visit the global VFS and add model paths to a VFS tree
 * populator object.
 */
class ModelFileFunctor 
{
	// VFSTreePopulators to populate
	gtkutil::VFSTreePopulator& _populator;
	gtkutil::VFSTreePopulator& _populator2;

	// Progress dialog and model count
	gtkutil::ModalProgressDialog _progress;
	int _count;

    // Event rate limiter for progress dialog
    EventRateLimiter _evLimiter;
	
public:
	
	typedef const std::string& first_argument_type;

	// Constructor sets the populator
	ModelFileFunctor(gtkutil::VFSTreePopulator& pop, gtkutil::VFSTreePopulator& pop2) : 
		_populator(pop),
		_populator2(pop2),
		_progress(GlobalMainFrame().getTopLevelWindow(), "Loading models"),
		_count(0),
		_evLimiter(50)
	{
		_progress.setText("Searching");
	}

	// Functor operator
	void operator() (const std::string& file) {

		// Test the extension. If it is not matching any of the known extensions,
		// not interested
		if (boost::algorithm::iends_with(file, LWO_EXTENSION) ||
			boost::algorithm::iends_with(file, ASE_EXTENSION) ||
			boost::algorithm::iends_with(file, MD5MESH_EXTENSION) ||
			boost::algorithm::iends_with(file, MD3_EXTENSION))
		{
			_count++;

			_populator.addPath(file);
			_populator2.addPath(file);
			
			if (_evLimiter.readyForEvent()) 
            {
				_progress.setText(boost::lexical_cast<std::string>(_count)
								  + " models loaded");
			}
		}
	}
};

}

#endif /*MODELFILEFUNCTOR_H_*/
