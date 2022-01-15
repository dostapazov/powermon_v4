#!/bin/bash

echo "Install powermon from deb package"
#apt-get install libqt5*
echo "install niktes-powermon package "
$(apt deb powermon-19-07-28-2.1-amd64.deb)
echo "install missing package"
$(apt-get --yes --force-yes install -f)
update-menus
echo "done install"
