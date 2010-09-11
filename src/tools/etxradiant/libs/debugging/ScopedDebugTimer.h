#ifndef SCOPEDDEBUGTIMER_H_
#define SCOPEDDEBUGTIMER_H_

#if defined(_MSC_VER) || defined(_WINDOWS_)
   #include <time.h>
   #include <windows.h>
   #if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
         struct timeval 
         {
            long tv_sec;
            long tv_usec;
         };
   #endif 
#else
   #include <sys/time.h>
#endif 

#if defined(_MSC_VER) || defined(_WINDOWS_)
   inline int gettimeofday(struct timeval* tv, void*) 
   {
      union {
         long long ns100;
         FILETIME ft;
      } now;
     
      GetSystemTimeAsFileTime (&now.ft);
      tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
      tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
     return (0);
   }
#endif

#include <string>
#include "stream/textfilestream.h"

namespace {
	
	const double MILLION = 1000000.0;

	/**
	 * Operator- for timeval structures.
	 * 
	 * @returns
	 * Double-precision float representing the difference in seconds between
	 * the two times.
	 */
	double operator-(const timeval& l, const timeval& r) {
		
		// Convert timevals to double
		double dl = (double) l.tv_sec + ((double) l.tv_usec / MILLION);
		double dr = (double) r.tv_sec + ((double) r.tv_usec / MILLION);
		
		return dl - dr;
	}
}

/**
 * Debugging class to time a particular event. The clock is saved during
 * construction, and the time difference calculated at destruction.
 */
class ScopedDebugTimer
{
private:
	
	// Start time
	timeval _s;
	
	// Name of operation
	std::string _op;
	
	// Show FPS?
	bool _fps;
	
public:
	
	/**
	 * Constructor. Set the name of the operation to be printed out on 
	 * destruction.
	 * 
	 * @param name
	 * The name of the operation.
	 * 
	 * @param showFps
	 * If true, a nominal FPS value will be calculated for the given operation
	 * time.
	 */
	ScopedDebugTimer(const std::string& name, bool showFps = false)
	: _op(name), _fps(showFps)
	{
		// Save start time
		gettimeofday(&_s, NULL);
	}
	
	/**
	 * Destructor. Prints out the time of the operation.
	 */
	~ScopedDebugTimer() {
		
		// Get the current time
		timeval end;
		gettimeofday(&end, NULL);
		
		// Calculate duration
		double duration = end - _s;
		
		globalOutputStream() << "[ScopedDebugTimer] \"" << _op 
			<< "\" in " << duration << " seconds";

		if (_fps) {
			globalOutputStream() << " (" << (1.0 / duration) << " FPS)";
		}

		globalOutputStream() << std::endl;
	}
};

#endif /*SCOPEDDEBUGTIMER_H_*/
