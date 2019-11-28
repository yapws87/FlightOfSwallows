# Flight Of Swallows

## Introduction
Measures bird count flying in and out of trap door.

## Process
### Noise Removal
Image from the pi camera can sometimes be noisy due to the unstable power supply of the bird house. This can distort the bird counting algorithm and produce inaccurate results.

Below is a snapshot of a typical noisy image. Notice the horizontal lines that appear through out the image, since it is almost systematically produced, it can be effectively removed(reduced) using in the FT(Fourier Transform) space.


<img src="images/noisy_2.PNG" alt="FFT_noise" width="300"/>

The image after FT. Since noise arise from horizontal lines in the spacial space, the same noise will appear as a vertical line in the frequency space. Removing the main vertical line in the frequency space can effectively remove the horizontal noise in the spacial domain. 

<img src="images/FFT_noise.PNG" alt="FFT_noise" width="250"/>

Vertical line removed in the FT space.

<img src="images/FFT_noise_less.PNG" alt="FFT_noise" width="250"/>


Inverse transform of the above image will produce image below. Even though the thick line remains, the high frequency horizontal line disappears.

<img src="images/noiseless_2.PNG" alt="FFT_noise" width="300"/>


## Useful Raspberry Pi Tips

### Enabling PiCamera to OpenCV
In order for openCv to access the Picamera, the following command is required :

```
sudo modprobe bcm2835-v4l2
```

This command will enable camera for openCV automatically.

---


### Run a command on Raspberry Pi boot
There are several methods to run a command after a pi boot. One of them is to add the command to rc.local file using

```
sudo nano /etc/rc.local
```

However it will be risky modifying command here as any mistake will render the Pi to a improper boot.

The safer and confirmed way of doing this is to invoke crontab,

```
 crontab -e
```

and then add command as follow,

```
  @reboot sudo modprobe bcm2835-v4l2
```

log can be created to record the error that might occur doing process

```
@reboot sudo modprobe bcm2835-v4l2 > /home/pi/Error.log 2>&1
```

---

### Make a file Executable
To make a script file or a .exe file executable the function chmod need to be used:

```
chmod 755 xxxx.exe
```

---

### Compiling a .CPP file in Raspberry
Only 2 files is necessary for compiling a .cpp file. The first is the .cpp file itself and the latter a txt file containing the following lines:

```
cmake_minimum_required(VERSION 3.1)
project( {proj_name} )
find_package( OpenCV REQUIRED)
find_package( Threads)

add_executable( {proj_name} {cpp_name}.cpp )
target_link_libraries( {proj_name} ${OpenCV_LIBS} )
target_link_libraries( {proj_name} ${CMAKE_THREAD_LIBS_INIT} )
```

Then navigate the console to project folder and run the following command

```
cd
cmake .
make
```

Then you can execute the code by running the .exe file created

---

### Setting a Static IP for Raspberry Pi 3
For the latest version Raspbian(2017), the /etc/network/interface file no longer supports static ip changes. Instead you will need to modify the dhcpcd file using the following command

```
sudo nano /etc/dhcpcd.conf
```
and insert the following lines

```
interface enxb827eb73a96b
static ip_address=192.168.0.222/24
static routers=192.168.0.1
static domain_name_servers=192.168.0.1
```

Be sure to insert the full name of the interface (i.e. enxb827eb73a96b instead of eth0) as it is found not to work with other names. You can find out the name of the connection if the following command

```
ifconfig
```

and it should result something like this

<img src="images/ip_pic.PNG" alt="FFT_noise" width="500"/>

the first entry from line shows the name of the connection and inet shows the current ip address.

---

### Delete Program
Raspbian usually comes with many pre-installed programs that eats up a lot of unnecessary space. One of them is wolfram-engine.

Program can be deleted from rapsberry pi by using the following

```
sudo apt-get purge wolfram-engine
```

---

### Measuring Core Performance
Often times we need to monitor the status of the pi to ensure that its functioning at its optimal condition.

#### Measuring Temperature
This command will check the current temperature of the arm chip and return values in degree celcius

```
vcgencmd measure_temp
```

#### Measuring Core Frequency
This command will check the frequency of the arm chip and return values in hertz

```
vcgencmd measure_clock arm
```

---

### Setting GPIO Control in Raspberry Pi
An easy way to control the GPIO setting through the terminal is by using WiringPi.
Here is an example of using WiringPi

```
> gpio -g mode 18 output
> gpio -g write 18 1
> gpio -g write 18 0
```

The code above will make the LED connected to PIN 18 (broadcom pin) blink.

Reading from the pin can be done using :

```
> gpio -g mode 17 up
> gpio -g read 17
```

The code above will return either 0 or 1 depending of the input of pin 17.

All of the code above can be done in C if wiringpi.h is included in the project.
FOr more information,visit,
https://learn.sparkfun.com/tutorials/raspberry-gpio

---
