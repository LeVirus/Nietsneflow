#!/bin/bash

sudo cp Nietsneflow3d/Lib/lib* /usr/local/lib/
sudo cp bin/Arnihs /usr/local/bin/
cp -r Nietsneflow3d/Ressources/ $HOME/.local/share/Arnihs
cp launch.sh $HOME/.local/share/Arnihs
ls -l $HOME/.local/share/Arnihs/launch.sh
chmod +x $HOME/.local/share/Arnihs/launch.sh
ls -l $HOME/.local/share/Arnihs/launch.sh
cp Arnihs.desktop $HOME/.local/share/applications
