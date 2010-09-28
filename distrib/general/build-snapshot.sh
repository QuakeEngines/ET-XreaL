#!/bin/bash

ROOT=`pwd`
DATE=`date +%Y%m%d`
DEVELOPER=../..
#RELEASE=../../../snapshots/ET-XreaL_snapshot_$DATE
RELEASE=ET-XreaL_snapshot_$DATE
DEFAULTGAME=etmain
COREPK3=zz-XreaL-$DATE.pk3
DLLPK3=mp_bin-$DATE.pk3

# remove previously created package
rm $RELEASE.7z
rm -rf $RELEASE

# do a clean svn export of the latest stuff
git clone --recursive git://xreal.git.sourceforge.net/gitroot/xreal/ET-XreaL $RELEASE
#cp -a XreaL_export $RELEASE

# add game logic
cp $DEVELOPER/$DEFAULTGAME/ui_mp_x86.dll $RELEASE/$DEFAULTGAME/
cp $DEVELOPER/$DEFAULTGAME/cgame_mp_x86.dll $RELEASE/$DEFAULTGAME/
cp $DEVELOPER/$DEFAULTGAME/qagame_mp_x86.dll $RELEASE/$DEFAULTGAME/

7z a $RELEASE.7z $RELEASE/$DEFAULTGAME/ui_mp_x86.dll
7z a $RELEASE.7z $RELEASE/$DEFAULTGAME/cgame_mp_x86.dll
7z a $RELEASE.7z $RELEASE/$DEFAULTGAME/qagame_mp_x86.dll
#7z a $RELEASE.7z $RELEASE/$DEFAULTGAME/src

# build core pk3 and delete everything else
cd $RELEASE/$DEFAULTGAME
rm zz-XreaL*.pk3

# create mp_bin.pk3
zip -r $DLLPK3 src
zip $DLLPK3 ui_mp_x86.dll
zip $DLLPK3 cgame_mp_x86.dll
zip $DLLPK3 qagame_mp_x86.dll
mv $DLLPK3 ..

# build pk3 with XreaL addons
#rm -rf src
zip -r $COREPK3 . -x \*.pk3 \src \*.dll
cd ..
mv $DEFAULTGAME/$COREPK3 .
rm -rf $DEFAULTGAME
mkdir $DEFAULTGAME
mv $DLLPK3 $DEFAULTGAME
mv $COREPK3 $DEFAULTGAME
cd $DEFAULTGAME
unzip $COREPK3 MEDIA.txt

cd $ROOT

# add maps
#cp $DEVELOPER/$DEFAULTGAME/map-qx_hod-20100928.pk3 $RELEASE/$DEFAULTGAME


# add Win32 binaries
cp $DEVELOPER/ETXreaL.exe $RELEASE/
cp $DEVELOPER/ETXreaL-dedicated.exe $RELEASE/
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
#cp $DEVELOPER/xreal.x86 $RELEASE/
#cp $DEVELOPER/xreal-server.x86 $RELEASE/
#cp $DEVELOPER/base/cgamei386.so $RELEASE/base/
#cp $DEVELOPER/base/qagamei386.so $RELEASE/base/
#cp $DEVELOPER/base/uii386.so $RELEASE/base/

#cp $DEVELOPER/xmap.x86 $RELEASE/
#cp $DEVELOPER/xmap2.x86 $RELEASE/

#cp $DEVELOPER/gtkradiant/gtkradiant.x86 $RELEASE/gtkradiant/
#mkdir $RELEASE/gtkradiant/modules
#cp $DEVELOPER/gtkradiant/modules/archivezip.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/brushexport.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/entityq3.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/imagejpeg.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/imagepng.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/imageq3.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/mapq3.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/modelpico.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/shadersq3.so $RELEASE/gtkradiant/modules/
#cp $DEVELOPER/gtkradiant/modules/vfsq3.so $RELEASE/gtkradiant/modules/

# clean up things
rm -rf $RELEASE/.git/
rm $RELEASE/.gitmodules
rm -rf $RELEASE/etxradiant/.git
rm -rf $RELEASE/distrib/
#rm $RELEASE/SConstruct
#rm $RELEASE/SConscript*
#rm $RELEASE/Makefile
rm $RELEASE/indent*
rm $RELEASE/.indent.pro
#rm $RELEASE/CODINGSTYLE.txt
#rm -rf $RELEASE/code/

# build final package
#zip -r $RELEASE.zip $RELEASE/
7z a $RELEASE.7z $RELEASE/
#rm -rf $RELEASE




