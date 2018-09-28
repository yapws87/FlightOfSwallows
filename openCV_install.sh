
# Installing all prerequisites
echo "Installing all prerequisites.."
yes | sudo apt-get install build-essential 
yes | sudo apt-get install cmake 
yes | sudo apt-get install pkg-config 
yes | sudo apt-get install libjpeg-dev 
yes | sudo apt-get install libtiff5-dev 
yes | sudo apt-get install libjasper-dev 
yes | sudo apt-get install libpng12-dev 
yes | sudo apt-get install libavcodec-dev 
yes | sudo apt-get install libavformat-dev 
yes | sudo apt-get install libswscale-dev 
yes | sudo apt-get install libv4l-dev 
yes | sudo apt-get install libxvidcore-dev 
yes | sudo apt-get install libx264-dev 
yes | sudo apt-get install libgtk2.0-dev 
yes | sudo apt-get install libatlas-base-dev 
yes | sudo apt-get install gfortran 
yes | sudo apt-get install python2.7-dev 
yes | sudo apt-get install python3-dev 

# Download and install OpenCV
echo "Installing OpenCV.."
cd ~
file_dir=opencv-3.3.1
if [ -d "$file_dir" ]
then
	echo "$file_dir found, skip to next step"
else
	echo "$file_dir not found. Downloading..."
	wget -O opencv.zip https://codeload.github.com/opencv/opencv/zip/3.3.1
	unzip opencv.zip
fi

# echo "Installing OpenCV Contrib.."
# file_dir=opencv_contrib-3.4.3
# if [ -d "$file_dir" ]
# then
# 	echo "$file_dir found, skip to next step"
# else
# 	echo "$file_dir not found. Downloading..."
# 	wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.4.3.zip
# 	unzip opencv_contrib.zip
# fi


echo "Installing Pip "
file_dir=get-pip.py
if [ -d "$file_dir" ]
then
	echo "$file_dir found, skip to next step"
else
	echo "$file_dir not found. Downloading..."
	wget https://bootstrap.pypa.io/get-pip.py
fi
sudo python get-pip.py

# Setting up virtual environemnt
yes | sudo pip install virtualenv virtualenvwrapper
yes | sudo rm -rf ~/.cache/pip
 
 
#virtualenv and virtualenvwrapper
export WORKON_HOME=$HOME/.virtualenvs
source /usr/local/bin/virtualenvwrapper.sh
 

echo -e "\n# virtualenv and virtualenvwrapper" >> ~/.profile
echo "export WORKON_HOME=$HOME/.virtualenvs" >> ~/.profile
echo "source /usr/local/bin/virtualenvwrapper.sh" >> ~/.profile

source ~/.profile

mkvirtualenv cv -p python3

source ~/.profile
workon cv

pip install numpy

#Install guide: Raspberry Pi 3 + Raspbian Jessie + OpenCV 3Shell

# Increase the memory alocation in pi
sed -i -e 's/CONF_SWAPSIZE=100/CONF_SWAPSIZE=1024/g' /etc/dphys-swapfile

# Restart the service
sudo /etc/init.d/dphys-swapfile stop
sudo /etc/init.d/dphys-swapfile start

# start compiling
cd ~/opencv-3.3.1/
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    # -D INSTALL_PYTHON_EXAMPLES=ON \
	-D ENABLE_PRECOMPILED_HEADERS=OFF ..
    # -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib-3.4.3/modules \
    # -D BUILD_EXAMPLES=ON ..

make -j4
sudo make install
sudo ldconfig

#Install guide: Raspberry Pi 3 + Raspbian Jessie + OpenCV 3Shell
#ls -l /usr/local/lib/python3.5/site-packages/

#cd /usr/local/lib/python3.5/site-packages/
#sudo mv cv2.cpython-35m-arm-linux-gnueabihf.so cv2.so

#cd ~/.virtualenvs/cv/lib/python3.5/site-packages/
#ln -s /usr/local/lib/python3.5/site-packages/cv2.so cv2.so