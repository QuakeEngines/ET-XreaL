![Screenshot, Goldrush Work in Progress](https://github.com/Maxxiii/ET-XreaL-clone/blob/master/screenshot.jpg)
# ET-XreaL clone
This is copy of ET-XreaL project. It's not in active development (since 2011), and I am not its author. I simply cloned it to Github for convenience. Original readme file, containing full description and installation instructions is "**[readme.txt](https://github.com/Maxxiii/ET-XreaL-clone/blob/master/README.txt)**".  
P.S. Currently developing successors of ET:XreaL - [Daemon Engine](https://github.com/DaemonEngine/Daemon), [OpenWolf Engine](https://github.com/TheDushan/OpenWolf-Engine)   

## Description
**XreaL** is a heavily improved Quake 3 Arena engine. It's bundled with new tools and a demo game that was never finished.  
**ET:XreaL** is a subproject bringing all XreaL enhancements to Return to Castle Wolfenstein: Enemy Territory.  

## Original links
ET:XreaL official pages: [Sourceforge](https://sourceforge.net/projects/xreal/), [Moddb](https://www.moddb.com/mods/etxreal) / Old threads on Splashdamage forums: [1](https://forums.splashdamage.com/t/et-xreal/), [2](https://forums.splashdamage.com/t/etxreal-0-3-0/)  
XreaL official website not currently working, but copy of it exists on [Web Archive](http://web.archive.org/web/20110131153542/http://xreal-project.net:80/).  
[ET:XreaL source code](https://sourceforge.net/p/xreal/ET-XreaL) (cloned here), [XreaL Shader manual](https://tremap.xtr3m.net/__Games/Xreal/Manual_Shader_1/ShaderManual.htm) ([PDF](https://www.moddb.com/mods/etxreal/downloads/shader-manual)), [ETXRadiant](https://sourceforge.net/p/xreal/ET-XreaL_etxradiant/), [Omni-bot](https://sourceforge.net/p/xreal/ET-XreaL_omni-bot/ci/master/tree/), [Texture Guidelines](http://web.archive.org/web/20100115044416/http://redmine.xreal-project.net/projects/xreal/wiki/Texture_Guidelines)

Full engine package, including game files (1.5 Gb) you can download from [Sourceforge](https://sourceforge.net/projects/xreal/files/latest/download) or from [Moddb](https://www.moddb.com/mods/etxreal/downloads/etxreal-030)  
Wolfenstein: Enemy Territory is free game, you can download it [here](https://www.splashdamage.com/games/wolfenstein-enemy-territory/)

## Features
ET:XreaL  
* Modern OpenGL 3.2 renderer with all deprecated OpenGL calls removed  
* Clever usage of vertex buffer objects (VBO) to speed up rendering of everything  
* Avoids geometry processing each frame using the CPU (worst bottleneck with the Q3A engine)  
* Renders up to 500 000 - 1 000 000 polygons at 80 - 200 fps on current hardware (DX10 generation)  
* Optional GPU occlusion culling (improved Coherent Hierarchy Culling) useful for rendering large city scenes  
* Doom 3 .MD5mesh/.MD5anim skeletal model and animation support  
* Unreal Actor X .PSK/.PSA skeletal model and animation support  
* True 64 bit HDR lighting with adaptive tone mapping  
* Advanced projective and omni-directional soft shadow mapping methods like EVSM  
* Real-time sun lights with parallel-split shadow maps  
* Optional deferred shading  
* Relief mapping that can be enabled by materials  
* Optional uniform lighting and shadowing model like in Doom 3 including globe mapping  
* Supports almost all Quake 3, Enemy Territory and Doom 3 material shader keywords  
* TGA, PNG, JPG and DDS format support for textures  
* Usage of frame buffer objects (FBO) to perform offscreen rendering effects  
* Improved TrueType font support that does not require external tools  
* Linux 64-bit support  
* Linux sound backend using SDL  
* .avi recorder from ioquake3 including sound support  
* Optimized collision detection routines  
* Support for Omni-bot  
* GUID system to generate internal etkeys which are sent as cl_guid  
  
ET:XMap BSP Compiler
* Based on q3map2 by Randy 'ydnar' Reddig including additional fixes by the NetRadiant edition  
* Supports Doom 3 and Quake 4 .map formats  
* Built-in mini BSP viewer using -draw  

## Continue reading
Everything else - look in original readme file (**[readme.txt](https://github.com/Maxxiii/ET-XreaL-clone/blob/master/README.txt)**).
