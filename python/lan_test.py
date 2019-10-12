#!/usr/bin/env python

import subprocess
import time
import os

DELAY_BETWEEN_PINGS = 5    # delay in seconds
DELAY_BETWEEN_TESTS = 10  # delay in seconds
START_DELAY = 10          # delay in seconds
# if you want to add more sites change line below like this. ["google.co.uk" , "bbc.co.uk"]
#SITES = ["192.168.100.1"]
SITES = ["google.com"]

# issue Linux ping command to determine internet connection status
def ping(site):
  
  cmd = "/bin/ping -c 1 " + site
  try:
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
  except : #subprocess.CalledProcessError, e:
    return 0
  else:
    return 1

# ping the sites in the site list the specified number of times
# and return number of successful pings
def ping_sites(site_list, wait_time, times):
  successful_pings = 0
  attempted_pings = times * len(site_list)
  for t in range(0, times):
    for s in site_list:
      successful_pings += ping(s)
      time.sleep(wait_time)

  return successful_pings



# main loop: ping sites, wait, repeat
print("LAN Network test Starts")
time.sleep(START_DELAY)

while True:
  success = ping_sites(SITES, DELAY_BETWEEN_PINGS, 5)
 
  print(success)
  if success == 0:
    os.system("sudo reboot")
    
    
  time.sleep(DELAY_BETWEEN_TESTS)