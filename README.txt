
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

	3) INSTALLATION
	
	4) GETTING THE SOURCE CODE AND MEDIA

	5) COMPILING ON WIN32 WITH VISUAL C++ 2008 EXPRESS EDITION

	6) COMPILING ON GNU/LINUX
	
	7) CHANGES
	
	8) FEATURES
	
	9) CONSOLE VARIABLES
	
	10) KNOWN ISSUES
	
	11) BUG REPORTS
	
	12) FUTURE PLAN
	
	13) CONTRIBUTIONS
	
	14) DONATIONS



___________________________________

1) SYSTEM REQUIREMENTS
__________________________



Minimum system requirements:

	CPU: 2 GHz Intel compatible
	System Memory: 512MB
	Graphics card: Any graphics card that supports Direct3D 10 and OpenGL >= 3.2

Recommended system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 1024MB+
	Graphics card: Geforce 8800 GT, ATI HD 4850 or higher. 




_______________________________

2) LICENSE
______________________

See COPYING.txt for all the legal stuff.



_______________________________

3) INSTALLATION
______________________

This release does not contain Enemy Territory's game data, the game data is still
covered by the original EULA and must be obeyed as usual.

Wolfenstein: Enemy Territory is a free release, and can be downloaded from
http://www.splashdamage.com/content/wolfenstein-enemy-territory-barracks

Install the latest version of Wolfenstein: Enemy Territory for your platform to get the game data
and copy it to ET-XreaL/etmain or patch your Wolfenstein: Enemy Territory by extracting ET-XreaL_snapshot_date.7z over it.



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

	> apt-get install scons libcurl-openssl-dev libsdl1.2-dev libxxf86dga-dev libxxf86vm-dev libglu1-mesa-dev
	
On Fedora

	> yum install scons SDL-devel libXxf86dga-devel libXxf86vm-devel mesa-libGLU-devel

Compile XreaL:
	
	> scons or make

Type scons -h for more compile options.


___________________________________________________

7) Changes made since the Wolfenstein: Enemy Territory GPL release
__________________________________________

	* ported XreaL renderer to src/engine/rendererGL3/
	* rewrote .mdm/.mdx code to be GPU accelerated
	* rewrote .md3/.mdc code to be GPU accelerated
	* added support for Linux 64-bit
	* replaced Linux sound backend with SDL
	* added .avi recorder from ioquake3 including sound support
	* added AABB collision optimisations
	* added new ETXMap BSP compiler based on NetRadiant's q3map2
	* added new ETXRadiant level editor based on DarkRadiant

	
	
___________________________________________________

8) General XreaL id Tech 3 Features
__________________________________________
	
	* clever usage of vertex buffer objects (VBO) to speed up rendering of everything
    * avoids geometry processing at render time using the CPU (worst bottleneck with the Q3A engine)
    * renders up to 500 000 – 1 000 000 polygons at 80 - 200 fps on current hardware (DX10 generation)
	* optional GPU occlusion culling (improved Coherent Hierarchy Culling) useful for rendering large city scenes
    * Doom 3 .MD5mesh/.MD5anim skeletal model and animation support
    * Unreal Actor X .PSK/.PSA skeletal model and animation support
    * true 64 bit HDR lighting with adaptive tone mapping
    * advanced projective and omni-directional soft shadow mapping methods like VSM and ESM
    * real-time sun lights with parallel-split shadow maps
    * optional deferred shading
    * relief mapping that can be enabled by materials
    * optional uniform lighting and shadowing model like in Doom 3 including globe mapping
    * supports almost all Quake 3, Enemy Territory and Doom 3 material shader keywords
    * TGA, PNG, JPG and DDS format support for textures
    * usage of frame buffer objects (FBO) to perform offscreen rendering effects
    * improved TrueType font support that does not require external tools

	

___________________________________________________

9) CONSOLE VARIABLES
__________________________________________
	
cg_shadows						Sets the shadows quality
								0 = Off
								1 = Blob shadow
								2 = planar (broken and unsupported)
								3 = stencil shadow volumes in XreaL (unsupported and was removed)
								4 = Variance Shadow Mapping (16-bit quality)
								5 = Variance Shadow Mapping (32-bit quality)
								6 = Exponential Shadow Mapping (32-bit quality)
								

r_dynamicLight					Enable dynamic lights
r_dynamicLightCastShadows		Enable dynamic lights to cast shadows with all interacting surfaces (expensive)

r_vboVertexSkinning				Enables skeletal animation rendering on the GPU
								Pros:
									- Can be much faster with Nvidia cards
								Cons:
									- Can be slower than the CPU path on ATI cards

r_normalMapping					Enables bump mapping
								0 = Disables it and allows faster ET style render mode
								1 = Enables it with simple Blinn-Phong specular lighting
								
r_parallaxMapping				Enables Relief Mapping if materials support it
									
				
r_cameraPostFX					Enable camera postprocessing effects like filmgrain
r_cameraFilmGrain				Enable camera film grain simulation
r_cameraFilmGrainScale			Set the grain scale
r_cameraVignette				Enable vignetting postprocessing effect

r_bloom							Enable bloom postprocessing effect
r_bloomPasses					Number of times the blur filter is applied
r_bloomBlur						Amount to scale the X anx Y axis sample offsets


r_hdrRendering					Enable High Dynamic Range lighting (experimental)

// ----------------------------------------------------------------------------
HDR variables that are cheat protected but might be interesting for some people

r_hdrToneMapingOperator			Tone mapping method:  
								1 = Reinhard (Yxy)
								4 = Exponential

r_hdrKey						Middle gray value used in HDR tone mapping
								0 computes it dynamically
								0.72 default

r_hdrMinLuminance				Minimum luminance value threshold
r_hdrMaxLuminance				Maximum luminance value threshold

r_hdrDebug						Shows min, max and average luminance detected by the scene input

// ----------------------------------------------------------------------------

r_deferredShading				(experimental)
								0 = Renders dynamic lights using Forward Shading like in Doom 3 (default)
								1 = Renders dynamic lights using Deferred Shading like in S.T.A.L.K.E.R.
								Pros:
									- It can render hundreds of dynamic lights that don't cast shadows really fast
									- It stabilizes the fps with heavy weapon fire
								Cons:
									- It lowers the fps if there are no lights because the first depth pass is more expensive
									- It's not compatible with all default ET shader files yet
									

// ----------------------------------------------------------------------------
Variables interesting for developers and mappers

r_showTris						Shows all fast GPU path triangles blue and slow CPU path triangles red
								Blue triangles indicate that those batches don't have to be moved through the PCIe bridge.
								All required geometry and shaders are already available in the GPU memory.
								
								Red triangles indicate that some geometry has to be processed by the CPU before it can be rendered.
								This usually happens with deformVertexes shader commands.
								
r_showBatches					Draws all batches (geometry, material and lightmap combination) as individual colors.
								General rule: more batches => less performance
								Avoid many many tiny lightmaps like in the TCE mod and rather use lightmaps with 2048^2 for better batching.
								
r_showLightMaps					Draw all lightmaps (requires glsl_restart)
								
r_showDeluxeMaps				Draw all directional lightmaps (requires glsl_restart)



// ----------------------------------------------------------------------------



___________________________________________________

10) KNOWN ISSUES
__________________________________________

	* broken map loading screen
	* a few skys are broken (r_fastsky 1 can help with this)
	* light bleeding problems with cg_shadows 2 - 3 which are typical for variance shadow mapping


___________________________________________________

11) BUG REPORTS
__________________________________________

ET-XreaL is not perfect, it’s not bug free as every other software.
For fixing as much problems as possible we need as much bug reports as possible.
We can’t fix anything if we don’t know about the problems.

The best way for telling us about a bug is by submitting a bug report at our SourceForge bug tracker page:

	http://sourceforge.net/tracker/?group_id=27204&atid=389772

The most important fact about this tracker is that we can’t simply forget to fix the bugs which are posted there. 
It’s also a great way to keep track of fixed stuff.

If you want to report an issue with the game, you should make sure that your report includes all information useful to characterize and reproduce the bug.

    * search on Google
    * include the computer’s hardware and software description ( CPU, RAM, 3D Card, distribution, kernel etc. )
    * if appropriate, send a console log, a screenshot, an strace ..
    * if you are sending a console log, make sure to enable developer output:

              ETXreaL.exe +set developer 1 +set logfile 2

NOTE: We can’t help you with OS-specific issues like configuring OpenGL correctly, configuring ALSA or configuring the network.
	

	
___________________________________________________

12) FUTURE PLAN
__________________________________________

	* optimize ET decal system with VBOs
	* add Blender tools to make it easier to replace the existing models
	* improve ETXMap ET .map -> Doom 3 .map format conversion routine to handle detail brushes as func_static entities
	* improve ETXMap compiler for better support of extracting models from .bsp files for Blender and further editing
	* finish Doom 3 style ETXRadiant entity definitions file etmain/def/entities.def to have full support for all ET entity types
	* write SCons files for ETXMap and ETXRadiant
	* add Bullet physics engine (maybe)
	* use projection matrix that makes the near plane identical to the portal clipping plane.
		(Eric Leyngel describes this method in his "Projection Matrix Tricks" paper.)
		comment from https://bugzilla.icculus.org/show_bug.cgi?id=4358



___________________________________________________

13) CONTRIBUTIONS
__________________________________________

If you want to contribute media assets like textures, models or sounds to the project then:

	1) Don't derivate them from the original Enemy Territory assets. If you want to add textures or models then you have to create them from scratch.
	
	2) Release your assets under the "Creative Commons Attribution-ShareAlike 3.0 Unported" license.
		See http://creativecommons.org/licenses/by-sa/3.0/ for more details.


___________________________________________________

14) DONATIONS
__________________________________________

If you think that this project is cool and helps you with your projects or you just have fun then make a small donation, please.

Click on one of the PayPal buttons to donate money to XreaL:
	
	http://sourceforge.net/donate/index.php?group_id=27204




