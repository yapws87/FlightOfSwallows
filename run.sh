#!/bin/bash

motor_pin=25
light_pin=24
input_pin=23

gpio -g mode $motor_pin output
gpio -g mode $light_pin output

gpio -g write $motor_pin 1
gpio -g write $light_pin 0

v4l2-ctl --set-ctrl=rotate=180 -c auto_exposure=0 -c exposure_time_absolute=100 -c white_balance_auto_preset=1 
v4l2-ctl -c iso_sensitivity_auto=1 -c iso_sensitivity=0 
v4l2-ctl -c exposure_metering_mode=2 -c brightness=50 -c saturation=100
v4l2-ctl -c contrast=25 -c sharpness=100 -c color_effects=1 -c scene_mode=11

/home/pi/projects/FlightOfSwallows/./PiCamera 321 240 90 0
