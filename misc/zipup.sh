# zipup.sh:
# Script to generate the distributions.
#
# By Christer Sandberg (ported from Adime made by Sven Sandberg)
#
# Requires zip, tar, gzip, rm, cp, and sed. Also requires an environment
# variabel ALLEGRO pointing to your Allegro directory.
#
##

# note: these are patched by fixver.sh script.
zipversion=210
tgzversion=2.1.0

if [ "$1" != "-q" ]; then

echo "Clean up..."
# Replace tabs with blanks
./fix.sh unix --ttos
# Make unix line endings
./fix.sh unix --dtou

echo "Converting DOS bat-file from Unix to DOS/Win32"
cp fix.bat _tmpfile
perl -p -i -e "s/([^\r]|^)\n/\1\r\n/" _tmpfile
touch -r fix.bat _tmpfile
mv _tmpfile fix.bat

# Dont generate dependencies if a parameter was given.
# This is because I often realize that I missed something essential _after_
# calling zipup.sh, so it saves lots of time if the dependencies don't need
# to be rebuilt
if [ "$1" = "--no-depend" ]; then
   echo "Skip generating dependencies"
else
   echo "Generate dll stuff..."
   ./misc/fixdll.sh

   echo "Generate dependencies..."
   #But don't alter the line endings (--quick ensures that)
   ./fix.sh mingw32 --quick
   make depend

   ./fix.sh msvc --quick
   make depend

   ./fix.sh unix --quick
   make depend

   ./fix.sh cygwin --quick
   make depend

   ./fix.sh djgpp --quick
   make depend
fi

echo "Generate other files for the distribution..."
make docs "LFN=y"
make dat

fi

echo "Copy default makefile..."
cp misc/emptymak makefile

files="makefile fix* misc/*.sh cgui.png \
       misc/makefile* misc/emptymak \
       misc/dllsyms.lst misc/*.c misc/makedoc/*.c misc/makedoc/*.h \
       docs/*._tx \
       readme.txt *.html\
       resource/*.dat resource/cguilabl \
       src/*.c src/*.h src/*.t src/*.ht \
       include/*.h include/cgui/*.h \
       obj/*/*/*.dep obj/*/plugins.h \
       lib/*/tmpfile.txt lib/*/*.def \
       tools/*.c tools/*.h tools/readme.txt \
       examples/*.c examples/*.inc examples/examples.dat \
       examples/keyboard.dat examples/running.dat examples/*.cfg examples/readme.txt"

afiles=`echo $files       | sed -e's/\([^ ][^ ]*\)/ cgui\/\1/g'`


#############################################################################
echo "Remove old archives..."
rm -f cgui$zipversion.zip
rm -f cgui-$tgzversion.tar.gz

# Line endings are still unix in all files, including bat-files. To make the tar archive useful also for
# Windows users, we change the line endings to DOS/Windows already before packing the tar. This should be
# no problem for unix users since the bat file has no meaning in unix.
cp "fix.bat" _tmpfile
perl -p -i -e "s/([^\r]|^)\n/\1\r\n/" _tmpfile
touch -r "fix.bat" _tmpfile
mv _tmpfile "fix.bat"

echo "Create tar.gz archives..."
cd ..
tar -cv $afiles | gzip -9  > cgui/cgui-$tgzversion.tar.gz
cd cgui

echo "Create zip archives..."
# Change line endings from unix to DOS/Windows before packing the zip archives
./fix.sh djgpp --utod
cd ..
zip -9 cgui/cgui$zipversion.zip $afiles
cd cgui

echo "Archives finished!"

# Restore correct unix line endings
./fix.sh unix --dtou

# fix.sh omits to change line endings in the bat-file, so we need to change it separately because we want the local
# copy to have the unix style to have the right endings when committing to cvs next time.
mv "fix.bat" _tmpfile
tr -d '\015' < _tmpfile > "fix.bat"
touch -r _tmpfile "fix.bat"
rm _tmpfile

echo "All done!"
