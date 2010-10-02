
____  ___                     .____     
\   \/  /______   ____ _____  |    |    
 \     /\_  __ \_/ __ \\__  \ |    |    
 /     \ |  | \/\  ___/ / __ \|    |___ 
/___/\  \|__|    \___  >____  /_______ \
      \_/            \/     \/        \/

_________________________________________


ET-XreaL Readme - http://sourceforge.net/projects/xreal

Thank you for downloading ET-XreaL.



_______________________________________

CONTENTS
_______________________________



This file contains the following sections:

	1) SYSTEM REQUIREMENTS

	2) LICENSE

	3) GENERAL NOTES
	
	4) GETTING THE SOURCE CODE AND MEDIA

	5) COMPILING ON WIN32 WITH VISUAL C++ 2008 EXPRESS EDITION

	6) COMPILING ON GNU/LINUX



___________________________________

1) SYSTEM REQUIREMENTS
__________________________



Minimum system requirements:

	CPU: 2 GHz Intel compatible
	System Memory: 512MB
	Graphics card: GeForce 6600 GT or any other Shader Model 3.0 compatible GFX card 

Recommended system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 1024MB+
	Graphics card: Geforce 8800 GT, ATI HD 4850 or higher. 




_______________________________

2) LICENSE
______________________

See COPYING.txt for all the legal stuff.



_______________________________

3) GENERAL NOTES
______________________

If you use a Nvidia card then keep the Nvidia Threaded Optimzation ON !!!


This release does not contain Enemy Territory's game data, the game data is still
covered by the original EULA and must be obeyed as usual.

Wolfenstein: Enemy Territory is a free release, and can be downloaded from
http://www.splashdamage.com/content/wolfenstein-enemy-territory-barracks

Install the latest version of the game for your platform to get the game data.


For Techies Only: 
If you want to test the engine with an OpenGL >= 3.2 core profile enabled then start it with: 
	
	> +set r_glCoreProfile 1

____________________________________________

4) GETTING THE SOURCE CODE AND NEW MEDIA
___________________________________

This project's SourceForge.net Git repository can be checked out through Git with the following instruction set: 

	> git clone --recursive git://xreal.git.sourceforge.net/gitroot/xreal/ET-XreaL



___________________________________________________________________

5) COMPILING ON WIN32 WITH VISUAL C++ 2008 EXPRESS EDITION
__________________________________________________________

1. Download and install the Visual C++ 2008 Express Edition.
2. Use the VC9 solutions to compile what you need:
	ET-XreaL/src/engine/wolf.sln
	ET-XreaL/src/tools/etxmap/etxmap.sln
	ET-XreaL/src/tools/etxradiant/tools/vcprojects/ETXRadiant.sln

	You need additional Win32 dependencies to build the ETXRadiant.
	Copy them from the DarkRadiant Subversion repository:
	
	> svn export -r5702 https://darkradiant.svn.sourceforge.net/svnroot/darkradiant/trunk/w32deps/   ET-XreaL/src/tools/etxradiant/w32deps

__________________________________

6) COMPILING ON GNU/LINUX
_________________________

You need the following dependencies in order to compile XreaL with all features:

 
On Debian or Ubuntu:

	> apt-get install libxxf86dga-dev libxxf86vm-dev libglu1-mesa-dev
	
On Fedora

	> yum install libXxf86dga-devel libXxf86vm-devel mesa-libGLU-devel

Compile XreaL:
	
	> scons or make

Type scons -h for more compile options.





