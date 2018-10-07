#!/bin/bash

motor_pin=25
light_pin=24
input_pin=23

gpio -g mode $motor_pin output
gpio -g mode $light_pin output

gpio -g write $motor_pin 1
gpio -g write $light_pin 1

v4l2-ctl --set-ctrl=rotate=180 -c auto_exposure=1 -c exposure_time_absolute=500 -c white_balance_auto_preset=1 
v4l2-ctl -c exposure_metering_mode=1 -c brightness=70 -c saturation=100
v4l2-ctl -c contrast=15 -c sharpness=100 -c color_effects=1 -c scene_mode=11

/home/pi/projects/FlightOfSwallows/./PiCamera 321 240 90 0
