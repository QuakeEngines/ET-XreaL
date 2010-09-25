if exist ..\..\..\..\..\etxradiant\lib rd ..\..\..\..\..\etxradiant\lib /S /Q
if exist ..\..\..\..\..\etxradiant\etc rd ..\..\..\..\..\etxradiant\etc /S /Q

del ..\..\..\..\..\etxradiant\*.dll

copy ..\..\w64deps\openal\bin\OpenAL32.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\openal\bin\wrap_oal.dll ..\..\..\..\..\etxradiant /Y

@rem Copy everything from w32deps etc folder
md ..\..\..\..\..\etxradiant\etc
xcopy ..\..\w32deps\gtk2\bin\etc\* ..\..\..\..\..\etxradiant\etc\. /S /Y

@rem The LIB folder containing the engines should come from the w64deps folder
md ..\..\..\..\..\etxradiant\lib
xcopy ..\..\w64deps\gtk2\bin\lib\* ..\..\..\..\..\etxradiant\lib\. /S /Y

copy ..\..\w64deps\gtk2\bin\libatk-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libcairo-2.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libjpeg-62.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libpng12-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libtiff.dll ..\..\..\..\..\etxradiant /Y 
copy ..\..\w64deps\gtk2\bin\libgio-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libgdk_pixbuf-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libgdk-win32-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libglib-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libgmodule-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libgobject-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libgtk-win32-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libpango-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\zlib1.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libpangocairo-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libpangowin32-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libpangoft2-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w64deps\gtk2\bin\libintl-8.dll ..\..\..\..\..\etxradiant /Y

copy ..\..\w64deps\glew\lib\glew32.dll ..\..\..\..\..\etxradiant /Y

copy ..\..\w64deps\python\bin\python26.dll ..\..\..\..\..\etxradiant /Y

@rem Copy the compiled GtkSourceView x64 DLLs to install
copy ..\..\build\libs\x64\libgtksourceview.dll ..\..\..\..\..\etxradiant /Y

@rem Copy the compiled GTKGlext x64 DLLs to install
copy ..\..\build\libs\x64\libgdkglext.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\build\libs\x64\libgtkglext.dll ..\..\..\..\..\etxradiant /Y

@rem Copy the compiled libxml2 DLL to install
copy ..\..\build\libs\x64\libxml2.dll ..\..\..\..\..\etxradiant /Y