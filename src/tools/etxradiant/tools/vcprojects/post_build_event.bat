copy ..\..\w32deps\libxml2\lib\libxml2.dll ..\..\..\..\..\etxradiant

copy ..\..\w32deps\openal\bin\OpenAL32.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\openal\bin\wrap_oal.dll ..\..\..\..\..\etxradiant

copy ..\..\w32deps\glew\bin\glew32.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\libpng\bin\libpng13.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\libiconv\lib\libiconv2.dll ..\..\..\..\..\etxradiant
md ..\..\..\..\..\etxradiant\etc
xcopy ..\..\w32deps\gtk2\bin\etc\* ..\..\..\..\..\etxradiant\etc\. /S /Y
md ..\..\..\..\..\etxradiant\lib
xcopy ..\..\w32deps\gtk2\bin\lib\* ..\..\..\..\..\etxradiant\lib\. /S /Y
copy ..\..\w32deps\gtk2\bin\iconv.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\intl.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\jpeg62.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libatk-1.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgailutil-18.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libcairo-2.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libfontconfig-1.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgdk_pixbuf-2.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgdk-win32-2.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgio-2.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libglib-2.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgmodule-2.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgobject-2.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgtk-win32-2.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libpango-1.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libpangocairo-1.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libpangoft2-1.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libpangowin32-1.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgdkglext-win32-1.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libgtkglext-win32-1.0-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtksourceview-2.0\bin\libgtksourceview.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\libpng12-0.dll ..\..\..\..\..\etxradiant
copy ..\..\w32deps\gtk2\bin\zlib1.dll ..\..\..\..\..\etxradiant