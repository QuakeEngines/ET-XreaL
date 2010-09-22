#ifndef _ICOMMANDSYSTEM_H_
#define _ICOMMANDSYSTEM_H_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include "math/Vector2.h"
#include "math/Vector3.h"

#include "imodule.h"

#include "string/string.h"

namespace cmd {

// Use these to define argument types
enum ArgumentTypeFlags
{
	ARGTYPE_VOID		= 0,
	ARGTYPE_STRING		= 1 << 0,
	ARGTYPE_INT			= 1 << 1,
	ARGTYPE_DOUBLE		= 1 << 2,
	ARGTYPE_VECTOR3		= 1 << 3,
	ARGTYPE_VECTOR2		= 1 << 4,
	// future types go here
	ARGTYPE_OPTIONAL	= 1 << 16,
};

// One command argument, provides several getter methods
class Argument
{
	std::string _strValue;
	double _doubleValue;
	int _intValue;
	Vector3 _vector3Value;
	Vector2 _vector2Value;

	// The type flags
	std::size_t _type;

public:
	Argument() :
		_doubleValue(0),
		_intValue(0),
		_vector3Value(0,0,0),
		_vector2Value(0,0),
		_type(ARGTYPE_VOID)
	{}

	Argument(const char* str) :
		_strValue(str),
		_doubleValue(strToDouble(str)),
		_intValue(strToInt(str)),
		_vector3Value(Vector3(str)),
		_vector2Value(Vector2(str)),
		_type(ARGTYPE_STRING)
	{
		tryNumberConversion();
		tryVectorConversion();
	}

	// String => Argument constructor
	Argument(const std::string& str) :
		_strValue(str),
		_doubleValue(strToDouble(str)),
		_intValue(strToInt(str)),
		_vector3Value(Vector3(str)),
		_vector2Value(Vector2(str)),
		_type(ARGTYPE_STRING)
	{
		tryNumberConversion();
		tryVectorConversion();
	}

	// Double => Argument constructor
	Argument(const double d) :
		_strValue(doubleToStr(d)),
		_doubleValue(d),
		_intValue(static_cast<int>(d)),
		_vector3Value(d,d,d),
		_vector2Value(d,d),
		_type(ARGTYPE_DOUBLE)
	{
		// Enable INT flag if double value is rounded
		if (lrint(_doubleValue) == _intValue) {
			_type |= ARGTYPE_INT;
		}
	}

	// Int => Argument constructor
	Argument(const int i) :
		_strValue(intToStr(i)),
		_doubleValue(static_cast<double>(i)),
		_intValue(i),
		_vector3Value(i,i,i),
		_vector2Value(i,i),
		_type(ARGTYPE_INT|ARGTYPE_DOUBLE) // INT can be used as DOUBLE too
	{}

	// Vector3 => Argument constructor
	Argument(const Vector3& v) :
		_strValue(doubleToStr(v[0]) + " " + doubleToStr(v[1]) + " " + doubleToStr(v[2])),
		_doubleValue(v.getLength()),
		_intValue(static_cast<int>(v.getLength())),
		_vector3Value(v),
		_vector2Value(v[0], v[1]),
		_type(ARGTYPE_VECTOR3)
	{}

	// Vector2 => Argument constructor
	Argument(const Vector2& v) :
		_strValue(doubleToStr(v[0]) + " " + doubleToStr(v[1]) + " " + doubleToStr(v[2])),
		_doubleValue(v.getLength()),
		_intValue(static_cast<int>(v.getLength())),
		_vector3Value(v[0], v[1], 0),
		_vector2Value(v),
		_type(ARGTYPE_VECTOR2)
	{}

	// Copy Constructor
	Argument(const Argument& other) :
		_strValue(other._strValue),
		_doubleValue(other._doubleValue),
		_intValue(other._intValue),
		_vector3Value(other._vector3Value),
		_vector2Value(other._vector2Value),
		_type(other._type)
	{}

	std::size_t getType() const {
		return _type;
	}

	std::string getString() const {
		return _strValue;
	}

	int getInt() const {
		return _intValue;
	}

	double getDouble() const {
		return _doubleValue;
	}

	Vector3 getVector3() const {
		return _vector3Value;
	}

	Vector2 getVector2() const {
		return _vector2Value;
	}

private:
	void tryNumberConversion() {
		// Try to cast the string value to numbers
		try {
			_intValue = boost::lexical_cast<int>(_strValue);
			// cast succeeded
			_type |= ARGTYPE_INT;
		}
		catch (boost::bad_lexical_cast) {}

		try {
			_doubleValue = boost::lexical_cast<double>(_strValue);
			// cast succeeded
			_type |= ARGTYPE_DOUBLE;
		}
		catch (boost::bad_lexical_cast) {}
	}
	
	void tryVectorConversion() {
		// Use a stringstream to parse the string
        std::stringstream strm(_strValue);
        strm << std::skipws;

		// Try converting the first two values
        strm >> _vector2Value.x();
        strm >> _vector2Value.y();

		if (!strm.fail()) {
			_type |= ARGTYPE_VECTOR2;

			// Try to parse the third value
			strm >> _vector3Value.z();

			if (!strm.fail()) {
				// Third value successfully parsed
				_type |= ARGTYPE_VECTOR3;
				// Copy the two values from the parsed Vector2
				_vector3Value.x() = _vector2Value.x();
				_vector3Value.y() = _vector2Value.y();
			}
		}
	}
};

typedef std::vector<Argument> ArgumentList;

/**
 * greebo: A command target must take an ArgumentList argument, like this:
 *
 * void doSomething(const ArgumentList& args);
 *
 * This can be both a free function and a member function.
 */
typedef boost::function<void (const ArgumentList&)> Function;

// A command signature consists just of arguments, no return types
class Signature :
	public std::vector<std::size_t> 
{
public:
	Signature()
	{}

	// Additional convenience constructors
	Signature(std::size_t type1) {
		push_back(type1);
	}

	Signature(std::size_t type1, std::size_t type2) {
		push_back(type1);
		push_back(type2);
	}

	Signature(std::size_t type1, std::size_t type2, std::size_t type3) {
		push_back(type1);
		push_back(type2);
		push_back(type3);
	}

	Signature(std::size_t type1, std::size_t type2, std::size_t type3, std::size_t type4) {
		push_back(type1);
		push_back(type2);
		push_back(type3);
		push_back(type4);
	}
};

/**
 * greebo: Auto-completion information returned by the CommandSystem
 * when the user is entering a partial command.
 */
struct AutoCompletionInfo
{
	// The command prefix this info is referring to
	std::string prefix;

	// The candidaes, alphabetically ordered, case-insensitively
	typedef std::vector<std::string> Candidates;
	Candidates candidates;
};

class ICommandSystem :
	public RegisterableModule
{
public:

	class Visitor
	{
	public:
		// destructor
		virtual ~Visitor() {}
		// Gets invoked for each command
		virtual void visit(const std::string& commandName) = 0;
	};

	/** 
	 * Visit each command/bind using the given walker class.
	 */
	virtual void foreachCommand(Visitor& visitor) = 0;

	/**
	 * greebo: Declares a new command with the given signature.
	 */
	virtual void addCommand(const std::string& name, Function func, 
							const Signature& signature = Signature()) = 0;

	/** 
	 * Remove a named command.
	 */
	virtual void removeCommand(const std::string& name) = 0;

	/**
	 * greebo: Define a new statement, which consists of a name and a
	 * string to execute.
	 *
	 * Consider this as some sort of macro.
	 *
	 * @statementName: The name of the statement, e.g. "exportASE"
	 * @string: The string to execute.
	 * @saveStatementToRegistry: when TRUE (default) this statement/bind
	 * is saved to the registry at program shutdown. Pass FALSE if you
	 * don't want to let this statement persist between sessions.
	 */
	virtual void addStatement(const std::string& statementName, 
							  const std::string& string,
							  bool saveStatementToRegistry = true) = 0;

	/**
	 * Returns the signature for the named command or bind. Statements
	 * always have an empty signature.
	 */
	virtual Signature getSignature(const std::string& name) = 0;

	/**
	 * greebo: Executes the given string as if the user had typed it
	 * in the command console. The passed string can be a sequence of
	 * statements separated by semicolon ';' characters. Each statement
	 * can have zero or more arguments, separated by spaces.
	 *
	 * It is possible to pass string arguments by using
	 * double- or single-quote characters.
	 * e.g. "This; string; will be; treated as a whole".
	 *
	 * The last command needs not to be delimited by a semicolon.
	 *
	 * Example: nudgeLeft; nudgeRight -1 0 0; write "Bla! Test"
	 */
	virtual void execute(const std::string& input) = 0;

	/**
	 * Execute the named command with the given arguments.
	 */
	virtual void executeCommand(const std::string& name) = 0;
	virtual void executeCommand(const std::string& name, const Argument& arg1) = 0;
	virtual void executeCommand(const std::string& name, const Argument& arg1, const Argument& arg2) = 0;
	virtual void executeCommand(const std::string& name, const Argument& arg1, const Argument& arg2, const Argument& arg3) = 0;

	// For more than 3 arguments, use this method to pass a vector of arguments
	virtual void executeCommand(const std::string& name, 
								 const ArgumentList& args) = 0;

	/**
	 * greebo: Returns autocompletion info for the given prefix.
	 */
	virtual AutoCompletionInfo getAutoCompletionInfo(const std::string& prefix) = 0;
};
typedef boost::shared_ptr<ICommandSystem> ICommandSystemPtr;

} // namespace cmd

const std::string MODULE_COMMANDSYSTEM("CommandSystem");

// This is the accessor for the commandsystem
inline cmd::ICommandSystem& GlobalCommandSystem() {
	// Cache the reference locally
	static cmd::ICommandSystem& _cmdSystem(
		*boost::static_pointer_cast<cmd::ICommandSystem>(
			module::GlobalModuleRegistry().getModule(MODULE_COMMANDSYSTEM)
		)
	);
	return _cmdSystem;
}

#endif /* _ICOMMANDSYSTEM_H_ */
