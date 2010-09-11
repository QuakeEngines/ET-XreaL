copy ..\..\w32deps\libxml2\lib\libxml2.dll ..\..\..\..\xrealradiant

copy ..\..\w64deps\openal\bin\OpenAL32.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\openal\bin\wrap_oal.dll ..\..\..\..\xrealradiant

rem Copy everything from w32deps etc folder
md ..\..\..\..\xrealradiant\etc
xcopy ..\..\w32deps\gtk2\bin\etc\* ..\..\..\..\xrealradiant\etc\. /S /Y

rem The LIB folder containing the engines should come from the w64deps folder
md ..\..\..\..\xrealradiant\lib
xcopy ..\..\w64deps\gtk2\bin\lib\* ..\..\..\..\xrealradiant\lib\. /S /Y

copy ..\..\w64deps\gtk2\bin\libatk-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libcairo-2.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libjpeg-62.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libpng12-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libtiff.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libgio-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libgdk_pixbuf-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libgdk-win32-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libglib-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libgmodule-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libgobject-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libgtk-win32-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libpango-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\zlib1.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libpangocairo-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w64deps\gtk2\bin\libpangowin32-1.0-0.dll ..\..\..\..\xrealradiant

copy ..\..\w64deps\glew\lib\glew32.dll ..\..\..\..\xrealradiant