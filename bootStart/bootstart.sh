sudo python /home/pi/projects/FlightOfSwallows/Twitter/HiWorld.py
sudo modprobe bcm2835-v4l2
v4l2-ctl --set-ctrl horizontal_flip=1 vertical_flip=1
sudo bash /home/pi/projects/FlightOfSwallows/.run.sh

