#ifndef SOUNDFILELOADER_H_
#define SOUNDFILELOADER_H_

#include "SoundManager.h"

#include "ifilesystem.h"
#include "iarchive.h"

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>

namespace sound
{

/**
 * Sound directory name.
 */
const char* SOUND_FOLDER = "sound/";
const char* WAV_EXTENSION = ".wav";
const char* OGG_EXTENSION = ".ogg";


/**
 * Loader class passed to the GlobalFileSystem to load sound files
 */
class SoundFileLoader
{
	// SoundManager to populate
	SoundManager& _manager;

public:

	// Required type
	typedef const std::string& first_argument_type;

	/**
	 * Constructor. Set the sound manager reference.
	 */
	SoundFileLoader(SoundManager& manager)
	: _manager(manager)
	{ }

	/**
	 * Functor operator.
	 */
	void operator() (const std::string& fileName) {

		// Test the extension. If it is not matching any of the known extensions,
		// not interested
		if (boost::algorithm::iends_with(fileName, WAV_EXTENSION) ||
			boost::algorithm::iends_with(fileName, OGG_EXTENSION))
		{
			//ArchiveFilePtr file = GlobalFileSystem().openFile(fileName);

			//if(file)
			{
				_manager.addSoundFile(fileName);
			}
			/*
			else
			{
				std::cerr << "[sound] Warning: unable to open \""
						  << fileName << "\"" << std::endl;
			}
			*/
		}
	}
};

}

#endif /*SOUNDFILELOADER_H_*/
