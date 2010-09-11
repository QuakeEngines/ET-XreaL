#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "itextstream.h"
#include "Executable.h"

namespace cmd {

class Command :
	public Executable
{
	// The actual function to call
	Function _function;

	// The number and types of arguments to use
	Signature _signature;

public:
	Command(const Function& function, const Signature& signature) :
		_function(function),
		_signature(signature)
	{}

	Signature getSignature() {
		return _signature;
	}

	virtual void execute(const ArgumentList& args) {
		// Check arguments
		if (_signature.size() < args.size()) {
			// Too many arguments, that's for sure
			globalErrorStream() << "Cannot execute command: Too many arguments. " 
				<< "(max. " << _signature.size() << " arguments required)" << std::endl;
			return;
		}

		// Check matching arguments
		ArgumentList::const_iterator arg = args.begin();
		for (Signature::const_iterator cur = _signature.begin(); cur != _signature.end(); ++cur) {

			std::size_t curFlags = *cur;
			bool curIsOptional = ((curFlags & ARGTYPE_OPTIONAL) != 0);

			// If arguments have run out, all remaining parts of the signature must be optional
			if (arg == args.end()) {
				// Non-optional arguments will cause an error
				if (!curIsOptional) {
					globalErrorStream() << "Cannot execute command: Missing arguments. " << std::endl;
					return;
				}
			}
			else {
				// We have incoming arguments to match our signature
				if ((curFlags & arg->getType()) == 0) {
					// Type mismatch
					globalErrorStream() << "Cannot execute command: Type mismatch at argument: " 
						<< arg->getString() << std::endl;
					return;
				}
			}

			// Increase argument iterator if possible
			if (arg != args.end()) {
				++arg;
			}
		}

		// Checks passed, call the command
		_function(args);
	}
};
typedef boost::shared_ptr<Command> CommandPtr;

} // namespace cmd

#endif /* _COMMAND_H_ */
