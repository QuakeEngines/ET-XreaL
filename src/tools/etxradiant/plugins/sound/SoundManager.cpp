#include "SoundManager.h"
#include "SoundFileLoader.h"

#include "ifilesystem.h"
#include "archivelib.h"
#include "generic/callback.h"
#include "parser/DefTokeniser.h"

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <stdlib.h> // for atoi

namespace sound
{

// Constructor
SoundManager::SoundManager() :
	_emptySound(new SoundFile("", ""))
{}

// Enumerate shaders
void SoundManager::forEachSoundFile(SoundFileVisitor& visitor) const {
	for (SoundFileMap::const_iterator i = _soundFiles.begin(); i != _soundFiles.end(); ++i)
	{
		visitor.visit(i->second);
	}
}

bool SoundManager::playSound(const std::string& fileName) {
	// Make a copy of the filename
	std::string name = fileName;

	// Try to open the file as it is
	ArchiveFilePtr file = GlobalFileSystem().openFile(name);
	std::cout << "Trying: " << name << "\n";
	if (file != NULL) {
		// File found, play it
		std::cout << "Found file: " << name << "\n";
		if (_soundPlayer) _soundPlayer->play(*file);
		return true;
	}

	std::string root = name;
	// File not found, try to strip the extension
	if (name.rfind(".") != std::string::npos) {
		root = name.substr(0, name.rfind("."));
	}

	// Try to open the .ogg variant
	name = root + ".ogg";
	std::cout << "Trying: " << name << "\n";
	file = GlobalFileSystem().openFile(name);
	if (file != NULL) {
		std::cout << "Found file: " << name << "\n";
		if (_soundPlayer) _soundPlayer->play(*file);
		return true;
	}

	// Try to open the file with .wav extension
	name = root + ".wav";
	std::cout << "Trying: " << name << "\n";
	file = GlobalFileSystem().openFile(name);
	if (file != NULL) {
		std::cout << "Found file: " << name << "\n";
		if (_soundPlayer) _soundPlayer->play(*file);
		return true;
	}

	// File not found
	return false;
}

void SoundManager::stopSound() {
	if (_soundPlayer) _soundPlayer->stop();
}

const ISoundFilePtr SoundManager::getSoundFile(const std::string& fileName) {
	SoundFileMap::const_iterator found = _soundFiles.find(fileName);

	// If the name was found, return it, otherwise return an empty shader object
	return (found != _soundFiles.end()) ? found->second : _emptySound;
}

void SoundManager::addSoundFile(const std::string& fileName) {

	//_soundFiles[fileName] = SoundFilePtr(new SoundFile(fileName, modName));

	std::pair<SoundFileMap::iterator, bool> result = _soundFiles.insert(
				SoundFileMap::value_type(
					fileName,
					SoundFilePtr(new SoundFile(fileName))
				)
			);

	if (!result.second) {
		globalErrorStream() << "[SoundManager]: SoundFile with name "
			<< fileName << " already exists." << std::endl;
	}
}

const std::string& SoundManager::getName() const {
	static std::string _name(MODULE_SOUNDMANAGER);
	return _name;
}

const StringSet& SoundManager::getDependencies() const {
	static StringSet _dependencies;

	if (_dependencies.empty()) {
		_dependencies.insert(MODULE_VIRTUALFILESYSTEM);
	}

	return _dependencies;
}

void SoundManager::initialiseModule(const ApplicationContext& ctx) 
{
	globalOutputStream() << "SoundManager::initialiseModule called\n";
	// Pass a SoundFileLoader to the filesystem
	SoundFileLoader loader(*this);
	GlobalFileSystem().forEachFile(
		SOUND_FOLDER,			// directory
		"*", 				// required extension
		makeCallback1(loader),	// loader callback
		99						// max depth
	);

 	globalOutputStream() << _soundFiles.size() << " sound files found." << std::endl;

    // Create the SoundPlayer if sound is not disabled
    const ApplicationContext::ArgumentList& args = ctx.getCmdLineArgs();
    ApplicationContext::ArgumentList::const_iterator found(
        std::find(args.begin(), args.end(), "--disable-sound")
    );
    if (found == args.end())
    {
        globalOutputStream() << "SoundManager: initialising sound playback"
                             << std::endl;
        _soundPlayer = boost::shared_ptr<SoundPlayer>(new SoundPlayer);
    }
    else
    {
        globalOutputStream() << "SoundManager: sound ouput disabled" 
                             << std::endl;
    }
}

} // namespace sound
