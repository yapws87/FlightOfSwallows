#!/bin/bash

motor_pin=25
light_pin=24
input_pin=23

gpio -g mode $motor_pin output
gpio -g mode $light_pin output

gpio -g write $motor_pin 1
gpio -g write $light_pin 1

v4l2-ctl --set-ctrl=rotate=180

~/projects/FlightOfSwallows/./PiCamera 321 240 90 1
