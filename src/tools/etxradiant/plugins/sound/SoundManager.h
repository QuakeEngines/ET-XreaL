#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include "SoundFile.h"
#include "SoundPlayer.h"

#include "isound.h"
#include "parser/DefTokeniser.h"

#include <map>

namespace sound {

/**
 * SoundManager implementing class.
 */
class SoundManager :
	public ISoundManager
{
	typedef std::map<std::string, SoundFilePtr> SoundFileMap;
	SoundFileMap _soundFiles;

	SoundFilePtr _emptySound;

	// The helper class for playing the sounds
	boost::shared_ptr<SoundPlayer> _soundPlayer;

public:
	/**
	 * Main constructor.
	 */
	SoundManager();

	/**
	 * Enumerate sound shaders.
	 */
	void forEachSoundFile(SoundFileVisitor& visitor) const;

	/** greebo: Returns the soundfile with the name <shaderName>
	 */
	const ISoundFilePtr getSoundFile(const std::string& shaderName);

	/** greebo: Plays the sound file. Tries to resolve the filename's
	 * 			extension by appending .ogg or .wav and such.
	 */
	virtual bool playSound(const std::string& fileName);

	/** greebo: Stops the playback immediately.
	 */
	virtual void stopSound();

	void addSoundFile(const std::string& fileName);

	// RegisterableModule implementation
	virtual const std::string& getName() const;
	virtual const StringSet& getDependencies() const;
	virtual void initialiseModule(const ApplicationContext& ctx);
};
typedef boost::shared_ptr<SoundManager> SoundManagerPtr;

}

#endif /*SOUNDMANAGER_H_*/
