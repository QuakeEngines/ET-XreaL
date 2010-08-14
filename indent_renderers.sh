#!/bin/sh

for i in `find src/renderer/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/renderer/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/renderer/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/renderer/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find -iname "*~"`; do rm $i; done
for i in `find -iname "*~"`; do rm $i; done
