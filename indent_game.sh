#!/bin/sh

for i in `find etmain/src/ -iname "*.h"`; do dos2unix $i; done
for i in `find etmain/src/ -iname "*.c"`; do dos2unix $i; done

for i in `find etmain/src/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find etmain/src/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find -iname "*~"`; do rm $i; done
for i in `find -iname "*~"`; do rm $i; done
