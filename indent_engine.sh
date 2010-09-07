#!/bin/sh

for i in `find src/engine/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/engine/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/engine/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/engine/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find -iname "*~"`; do rm $i; done
for i in `find -iname "*~"`; do rm $i; done
