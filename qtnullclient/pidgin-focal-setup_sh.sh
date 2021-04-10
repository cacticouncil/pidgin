#!/usr/bin/env bash
sudo apt update
sudo apt upgrade
sudo apt install intltool libgtkspell-dev libxml++2.6-dev libgstreamer-plugins-base1.0-dev libfarstream-0.2-dev libidn11-dev libmeanwhile-dev libavahi-glib-dev libavahi-client-dev libnm-dev libperl-dev libgnutls28-dev tk-dev pidgin-dev
git clone --branch qt-nullclient https://github.com/cacticouncil/pidgin.git ~/pidgin
export _TMP_NS="\$(tail -1 /etc/resolv.conf | cut -d' ' -f2)"
export _TMP_HOST=$(if [ $WSL_INTEROP ]; then echo $_TMP_NS; else echo "localhost"; fi)
echo -e "export WSL_HOST=$_TMP_HOST" >> ~/.profile
echo -e "export DISPLAY=\$WSL_HOST:0" >> ~/.profile
cd ~/pidgin
./configure
