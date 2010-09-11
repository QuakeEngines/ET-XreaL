#ifndef APPLICATIONCONTEXT_H_
#define APPLICATIONCONTEXT_H_

#include "imodule.h"
#include <vector>

namespace module {

class ApplicationContextImpl :
	public ApplicationContext
{
	// The app + home paths
	std::string _appPath;
	std::string _homePath;
	std::string _settingsPath;
	std::string _bitmapsPath;
	
	// The path where the .map files are stored
	std::string _mapsPath;

	// Command line arguments
	std::vector<std::string> _cmdLineArgs;

	// A function pointer to a global error handler, used for ASSERT_MESSAGE
	ErrorHandlingFunction _errorHandler;
	
public:
	/**
	 * Initialises the context with the arguments given to main().
	 */
	void initialise(int argc, char* argv[]);

	// Initialises the function handling pointers for debug builds
	void initErrorHandler();
	
	/**
	 * Return the application path of the current Radiant instance.
	 */
	virtual const std::string& getApplicationPath() const;
	
	/**
	 * Return the settings path of the current Radiant instance.
	 */
	virtual const std::string& getSettingsPath() const;
	
	/** 
	 * Return the path where the application's bitmaps are stored.
	 */
	virtual const std::string& getBitmapsPath() const;

	const ArgumentList& getCmdLineArgs() const;	
	
	// Return the global stream references
	virtual std::ostream& getOutputStream() const;
	virtual std::ostream& getWarningStream() const;
	virtual std::ostream& getErrorStream() const;

	// Exports/deletes the paths to/from the registry
	virtual void savePathsToRegistry() const;

	virtual const ErrorHandlingFunction& getErrorHandlingFunction() const;
	
private:
	// Sets up the bitmap path and settings path
	void initPaths();

	// Initialises the arguments
	void initArgs(int argc, char* argv[]);
};

} // namespace module

#endif /*APPLICATIONCONTEXT_H_*/
