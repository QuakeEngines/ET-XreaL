if exist ..\..\..\..\..\etxradiant\lib rd ..\..\..\..\..\etxradiant\lib /S /Q
if exist ..\..\..\..\..\etxradiant\etc rd ..\..\..\..\..\etxradiant\etc /S /Q

del ..\..\..\..\..\etxradiant\*.dll

copy ..\..\w32deps\openal\bin\OpenAL32.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\openal\bin\wrap_oal.dll ..\..\..\..\..\etxradiant /Y

copy ..\..\w32deps\glew\bin\glew32.dll ..\..\..\..\..\etxradiant /Y

copy ..\..\w32deps\libpng\bin\libpng13.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\libiconv\bin\libiconv2.dll ..\..\..\..\..\etxradiant /Y

md ..\..\..\..\..\etxradiant\etc
xcopy ..\..\w32deps\gtk2\bin\etc\* ..\..\..\..\..\etxradiant\etc\. /S /Y

md ..\..\..\..\..\etxradiant\lib
xcopy ..\..\w32deps\gtk2\bin\lib\* ..\..\..\..\..\etxradiant\lib\. /S /Y

@rem copy ..\..\w32deps\gtk2\bin\iconv.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\intl.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\jpeg62.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libatk-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgailutil-18.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libcairo-2.dll ..\..\..\..\..\etxradiant /Y
@rem copy ..\..\w32deps\gtk2\bin\libfontconfig-1.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgdk_pixbuf-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgdk-win32-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgio-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libglib-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgmodule-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgobject-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgthread-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libgtk-win32-2.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libpango-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libpangocairo-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libpangoft2-1.0-0.dll ..\..\..\..\..\etxradiant  /Y
copy ..\..\w32deps\gtk2\bin\libpangowin32-1.0-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libpng12-0.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\libtiff3.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\w32deps\gtk2\bin\zlib1.dll ..\..\..\..\..\etxradiant /Y

copy ..\..\w32deps\python\bin\python26.dll ..\..\..\..\..\etxradiant /Y

@rem Copy the compiled GtkSourceView x86 DLLs to install
copy ..\..\build\libs\Win32\libgtksourceview.dll ..\..\..\..\..\etxradiant /Y

@rem Copy the compiled GTKGlext x86 DLLs to install
copy ..\..\build\libs\Win32\libgdkglext.dll ..\..\..\..\..\etxradiant /Y
copy ..\..\build\libs\Win32\libgtkglext.dll ..\..\..\..\..\etxradiant /Y

@rem Copy the compiled libxml2 x86 DLL to install
copy ..\..\build\libs\Win32\libxml2.dll ..\..\..\..\..\etxradiant /Y
