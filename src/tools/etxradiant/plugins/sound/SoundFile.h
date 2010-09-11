#ifndef SOUNDFILE_H_
#define SOUNDFILE_H_

#include "isound.h"

#include <boost/shared_ptr.hpp>

namespace sound {

/**
 * Representation of a single sound file.
 */
class SoundFile
: public ISoundFile
{
	// Name of the shader
	std::string _name;

	// The modname (ModResource implementation)
	std::string _modName;
public:

	/**
	 * Constructor.
	 */
	SoundFile(const std::string& name, const std::string& modName = "base"):
	_name(name),
	_modName(modName)
	{
	}

	/**
	 * Return the name of the file.
	 */
	std::string getName() const {
		return _name;
	}

	std::string getModName() const {
		return _modName;
	}
};

/**
 * Shared pointer type.
 */
typedef boost::shared_ptr<SoundFile> SoundFilePtr;

}

#endif /*SOUNDSHADER_H_*/
