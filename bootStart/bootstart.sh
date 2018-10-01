#!/bin/bash
sudo python /home/pi/projects/FlightOfSwallows/twitter/HiWorld.py
sudo modprobe bcm2835-v4l2
v4l2-ctl --set-ctrl=rotate=180
sudo bash /home/pi/projects/FlightOfSwallows/./run.sh

