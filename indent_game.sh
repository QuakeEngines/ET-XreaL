#!/bin/sh

for i in `find src/botai/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/botai/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/cgame/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/cgame/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/game/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/game/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/ui/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/ui/ -iname "*.c"`; do dos2unix $i; done


for i in `find src/botai/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/botai/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/cgame/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/cgame/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/game/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/game/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/ui/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/ui/ -iname "*.c"`; do ./indent.exe $i; done


for i in `find -iname "*~"`; do rm $i; done
for i in `find -iname "*~"`; do rm $i; done
