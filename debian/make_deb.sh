#!/bin/bash
echo "make .dep packet"

function print_usage {
 echo  "Usage:"
 echo  "make_deb.sh packet-file-dir packet-name [packet-version = 1.0]"
}

if [ -z $1 ] ; then
  print_usage
  exit
fi

if [ -z $2 ] ; then
  print_usage
  exit
fi

ver=$3
if [ -z $3 ] ; then 
 ver="1.0"
fi


arch=$(dpkg --print-architecture)
src_folder=$1
packet_name=$2
echo "source folder $src_folder"
echo "packet name $packet_name"
echo "version $ver"
echo "architecture $arch"


dest_file="$(pwd)/"$packet_name"-$(date +%y-%m-%d)-$ver-$arch.deb"

#make ../Makefile
#cp -f ../bin/$appname  $srcdir
#strip "$srcdir/powermon"
#chmod -c 755 "$srcdir/powermon"

md5deep -r "$src_folder" > "$src_folder"/DEBIAN/md5sums

fakeroot dpkg-deb --build $src_folder  $dest_file

