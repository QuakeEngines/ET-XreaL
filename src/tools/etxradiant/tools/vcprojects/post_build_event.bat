copy ..\..\w32deps\libxml2\lib\libxml2.dll ..\..\..\..\xrealradiant

copy ..\..\w32deps\openal\bin\OpenAL32.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\openal\bin\wrap_oal.dll ..\..\..\..\xrealradiant

copy ..\..\w32deps\glew\bin\glew32.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\libpng\bin\libpng13.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\libiconv\lib\libiconv2.dll ..\..\..\..\xrealradiant
md ..\..\..\..\xrealradiant\etc
xcopy ..\..\w32deps\gtk2\bin\etc\* ..\..\..\..\xrealradiant\etc\. /S /Y
md ..\..\..\..\xrealradiant\lib
xcopy ..\..\w32deps\gtk2\bin\lib\* ..\..\..\..\xrealradiant\lib\. /S /Y
copy ..\..\w32deps\gtk2\bin\iconv.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\intl.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\jpeg62.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libatk-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgailutil-18.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libcairo-2.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libfontconfig-1.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgdk_pixbuf-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgdk-win32-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgio-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libglib-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgmodule-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgobject-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgtk-win32-2.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libpango-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libpangocairo-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libpangoft2-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libpangowin32-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgdkglext-win32-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libgtkglext-win32-1.0-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtksourceview-2.0\bin\libgtksourceview.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\libpng12-0.dll ..\..\..\..\xrealradiant
copy ..\..\w32deps\gtk2\bin\zlib1.dll ..\..\..\..\xrealradiant