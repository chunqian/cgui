#! /bin/sh
#
#  Shell script to adjust the version numbers and dates in cgui.h,
#  dllver.rc, readme._tx, cgui._tx and makefile.ver.
cv1=2
cv2=0
cv3=5
cv4=

echo "Changing version from $cv1.$cv2.$cv3 $cv4 to $1.$2.$3 $4" 1>&2

if [ $# -lt 3 -o $# -gt 4 ]; then
   echo "**** Command line error!" 1>&2
   echo "Usage: fixver major_num sub_num wip_num [comment]" 1>&2
   echo "Example: fixver 3 9 1 WIP" 1>&2
   exit 1
fi

# get the version and date strings in a nice format
if [ $# -eq 3 ]; then
   verstr="$1.$2.$3"
else
   verstr="$1.$2.$3 ($4)"
fi

year=$(date +%Y)
month=$(date +%m)
day=$(date +%d)
datestr="$(date +%b) $day, $year"

echo "s/cv1\=.*/cv1\=$1/" > fixver.sed
echo "s/cv2\=.*/cv2\=$2/" >> fixver.sed
echo "s/cv3\=.*/cv3\=$3/" >> fixver.sed
echo "s/cv4\=.*/cv4\=$4/" >> fixver.sed
cp misc/fixver.sh fixver.tmp
sed -f fixver.sed fixver.tmp > misc/fixver.sh

# patch include/cgui.h
echo "s/\#define CGUI_VERSION_MAJOR .*/\#define CGUI_VERSION_MAJOR      $1/" > fixver.sed
echo "s/\#define CGUI_VERSION_MINOR .*/\#define CGUI_VERSION_MINOR      $2/" >> fixver.sed
echo "s/\#define CGUI_VERSION_PATCH .*/\#define CGUI_VERSION_PATCH      $3/" >> fixver.sed
echo "s/\#define CGUI_VERSION_STRING .*/\#define CGUI_VERSION_STRING     \"$verstr\"/" >> fixver.sed
echo "s/\#define CGUI_DATE .*/\#define CGUI_DATE               $year$month$day    \/\* yyyymmdd \*\//" >> fixver.sed
echo "s/\#define CGUI_DATE_STRING .*/\#define CGUI_DATE_STRING \"$datestr\"/" >> fixver.sed
echo "Patching include/cgui.h..."
cp include/cgui.h fixver.tmp
sed -f fixver.sed fixver.tmp > include/cgui.h

# todo: allegro patches src/win/dllver.rc, do we need this?

# patch docs/readme._tx
echo "s/^\( *Version\) .*/\1 $verstr/" > fixver.sed
echo "s/By Christer Sandberg, .*\./By Christer Sandberg, $datestr\./" >> fixver.sed
echo "s/The Manual of CGUI.*/The Manual of CGUI $verstr/" >> fixver.sed
echo "Patching docs/readme._tx..."
cp docs/readme._tx fixver.tmp
sed -f fixver.sed fixver.tmp > docs/readme._tx

# patch docs/cgui._tx
echo "Patching docs/cgui._tx..."
echo "s/@manh=\"version [^\"]*\"/@manh=\"version $verstr\"/" >> fixver.sed
cp docs/cgui._tx fixver.tmp
sed -f fixver.sed fixver.tmp > docs/cgui._tx

# patch docs/internal._tx
echo "Patching docs/internal._tx..."
cp docs/internal._tx fixver.tmp
sed -f fixver.sed fixver.tmp > docs/internal._tx

# patch docs/thanks._tx
echo "Patching docs/thanks._tx..."
echo "s/@manh=\"version [^\"]*\"/@manh=\"version $verstr\"/" >> fixver.sed
cp docs/thanks._tx fixver.tmp
sed -f fixver.sed fixver.tmp > docs/thanks._tx

# patch docs/wishlist._tx
echo "Patching docs/wishlist._tx..."
echo "s/@manh=\"version [^\"]*\"/@manh=\"version $verstr\"/" >> fixver.sed
cp docs/wishlist._tx fixver.tmp
sed -f fixver.sed fixver.tmp > docs/wishlist._tx

# patch docs/changes._tx
echo "Patching docs/changes._tx..."
echo "s/@manh=\"version [^\"]*\"/@manh=\"version $verstr\"/" >> fixver.sed
cp docs/changes._tx fixver.tmp
sed -f fixver.sed fixver.tmp > docs/changes._tx

# patch misc/makefile.ver
echo "s/LIB_VERSION = .*/LIB_VERSION = $1$2/" > fixver.sed
echo "s/shared_version = .*/shared_version = $1.$2.$3/" >> fixver.sed
echo "s/shared_major_minor = .*/shared_major_minor = $1.$2/" >> fixver.sed

echo "Patching misc/makefile.ver..."
cp misc/makefile.ver fixver.tmp
sed -f fixver.sed fixver.tmp > misc/makefile.ver

# patch misc/zipup.sh
echo "s/zipversion=.*/zipversion=$1$2$3/" > fixver.sed
echo "s/tgzversion=.*/tgzversion=$1.$2.$3/" >> fixver.sed

echo "Patching misc/zipup.sh..."
cp misc/zipup.sh fixver.tmp
sed -f fixver.sed fixver.tmp > misc/zipup.sh

# clean up after ourselves
rm fixver.sed fixver.tmp

echo "Done!"
