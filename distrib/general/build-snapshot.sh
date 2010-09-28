#!/bin/bash

PWD=`pwd`
DATE=`date +%Y%m%d`
DEVELOPER=../..
RELEASE=XreaL_PreAlpha_$DATE

# remove previously created package
rm $RELEASE.7z
rm -rf $RELEASE

# do a clean svn export of the latest stuff
svn export https://xreal.svn.sourceforge.net/svnroot/xreal/trunk/xreal $RELEASE
#cp -a XreaL_export $RELEASE

# build core pk3 and delete everything else
cd $RELEASE/base
COREPK3=core-$DATE.pk3
rm core*.pk3
zip -r $COREPK3 . -x \*.pk3
cd ..
mv base/$COREPK3 .
rm -rf base/
mkdir base/
mv $COREPK3 base/
cd base/
unzip $COREPK3 MEDIA.txt

cd ../..

# add maps
cp $DEVELOPER/base/map-redm02-20090416.pk3 $RELEASE/base/
cp $DEVELOPER/base/map-redm08-20090222.pk3 $RELEASE/base/
cp $DEVELOPER/base/map-gwdm2-20090302.pk3 $RELEASE/base/
#cp $DEVELOPER/base/map-yan_test-20080506.pk3 $RELEASE/base/
#cp $DEVELOPER/base/map-kat_q4dm3-20080223.pk3 $RELEASE/base/
#cp $DEVELOPER/base/map-uglyrga-20080506.pk3 $RELEASE/base/


# add characters
#cp $DEVELOPER/base/player-visor-20060619.pk3 $RELEASE/base/
cp $DEVELOPER/base/player-harley-20080305.pk3 $RELEASE/base/
#cp $DEVELOPER/base/player-nitro-20070117.pk3 $RELEASE/base/
#cp $DEVELOPER/base/player-sarge-20080131.pk3 $RELEASE/base/
#cp $DEVELOPER/base/player-idgirl-20080222.pk3 $RELEASE/base/


# add Win32 binaries
#cp $DEVELOPER/xreal.exe $RELEASE/
#cp $DEVELOPER/base/cgamex86.dll $RELEASE/base/
#cp $DEVELOPER/base/qagamex86.dll $RELEASE/base/
#cp $DEVELOPER/base/uix86.dll $RELEASE/base/

#cp $DEVELOPER/xmap.exe $RELEASE/
#cp $DEVELOPER/xmap2.exe $RELEASE/

#cp $DEVELOPER/gtkradiant/gtkradiant.exe $RELEASE/gtkradiant/
#mkdir $RELEASE/gtkradiant/modules
#cp $DEVELOPER/gtkradiant/modules/archivezip.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/brushexport.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/entityq3.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/imagejpeg.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/imagepng.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/imageq3.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/mapq3.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/modelpico.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/shadersq3.dll $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/vfsq3.dll $RELEASE/gtkradiant/modules/

# add Linux i386 binaries
cp $DEVELOPER/xreal.x86 $RELEASE/
cp $DEVELOPER/xreal-server.x86 $RELEASE/
cp $DEVELOPER/base/cgamei386.so $RELEASE/base/
cp $DEVELOPER/base/qagamei386.so $RELEASE/base/
cp $DEVELOPER/base/uii386.so $RELEASE/base/

cp $DEVELOPER/xmap.x86 $RELEASE/
cp $DEVELOPER/xmap2.x86 $RELEASE/

cp $DEVELOPER/gtkradiant/gtkradiant.x86 $RELEASE/gtkradiant/
#mkdir $RELEASE/gtkradiant/modules
cp $DEVELOPER/gtkradiant/modules/archivezip.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/brushexport.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/entityq3.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/imagejpeg.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/imagepng.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/imageq3.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/mapq3.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/modelpico.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/shadersq3.so $RELEASE/gtkradiant/modules/
cp $DEVELOPER/gtkradiant/modules/vfsq3.so $RELEASE/gtkradiant/modules/

# clean up things
#rm $RELEASE/SConstruct
#rm $RELEASE/SConscript*
#rm $RELEASE/Makefile
rm $RELEASE/indent.exe
rm $RELEASE/.indent.pro
rm $RELEASE/CODINGSTYLE.txt
#rm -rf $RELEASE/code/

# build final package
#zip -r $RELEASE.zip $RELEASE/
7z a $RELEASE.7z $RELEASE/




