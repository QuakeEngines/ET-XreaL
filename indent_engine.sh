#!/bin/sh

for i in `find src/botlib/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/botlib/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/client/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/client/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/null/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/null/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/qcommon/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/qcommon/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/server/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/server/ -iname "*.c"`; do dos2unix $i; done

for i in `find src/win32/ -iname "*.h"`; do dos2unix $i; done
for i in `find src/win32/ -iname "*.c"`; do dos2unix $i; done


for i in `find src/botlib/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/botlib/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/client/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/client/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/null/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/null/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/qcommon/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/qcommon/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/server/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/server/ -iname "*.c"`; do ./indent.exe $i; done

for i in `find src/win32/ -iname "*.h"`; do ./indent.exe $i; done
for i in `find src/win32/ -iname "*.c"`; do ./indent.exe $i; done


for i in `find -iname "*~"`; do rm $i; done
for i in `find -iname "*~"`; do rm $i; done
