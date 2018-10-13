import sys
from datetime import datetime, date, time
from datetime import timedelta
import numpy as np
import matplotlib
matplotlib.use('Agg')
from matplotlib.dates import date2num
from matplotlib import pyplot as plt
from pathlib import Path


class BirdResult:

	def __init__(self,histo,total):
		self.histo = histo
		self.total = total
		

def printData(file_bird):
	for line in file_bird:
		print(line)
		
def extractData(file_bird,bin_minutes):
	
	# Initialize
	histo = []
	time_count = 0
	bin_seconds = bin_minutes * 60
	total_bins = (60 * 24) / bin_minutes
	for i in range(0,int(total_bins)):
		histo_time = str(timedelta(seconds=time_count))
		histo.append([time_count / 3600.0,0,0])
		time_count = time_count + bin_seconds
	
	
	time_anchor = 0
	bin_acc = 0
	speed_acc = 0
	idx = 0
	total_bird = 0
	for line in file_bird:
		line.replace('\t','\s')
		data = line.split()
		
		data_time_token = data[3].split(':')
		data_time = datetime(2000,1,1,int(data_time_token[0]),int(data_time_token[1]))
		data_time = (data_time - datetime(2000,1,1)).total_seconds()
		
		#print(data)
		#if  15 < int(data[5]) < 70:
		bin_acc = bin_acc + 1
		speed_acc = speed_acc + int(data[5])
		total_bird = total_bird + 1
		
		
		while (data_time - time_anchor) >= bin_seconds:
			bar_time = str(timedelta(seconds=time_anchor))
			time_anchor = time_anchor + bin_seconds
			if (data_time - time_anchor) <  bin_seconds:
				histo[idx] = [time_anchor / 3600.0,bin_acc , speed_acc / (bin_acc + 0.001)]
				speed_acc = 0
				bin_acc = 0
			else:
				histo[idx] = [time_anchor / 3600.0, 0,0]
				
			
			idx = idx + 1
		
		if idx >= total_bins:
			break
			
	
	arr_histo = np.array(histo).reshape(len(histo),3)
	
	#print('Total Bird : ', total_bird )
	return BirdResult(arr_histo,total_bird)
	
	#return arr_histo

def drawHisto(histo_in,histo_out,hist_width,histo_img_path,date_string):

	fig, ax = plt.subplots(figsize=(10,5))
	
	#ind = np.arange(N)    # the x locations for the groups
	width = 0.085  * hist_width    # the width of the bars: can also be len(x) sequence
	#width = 0.085 * 12
	
	ind = np.arange(len(histo_in))
	p2 = ax.bar(histo_out[:,0], histo_out[:,1], width, color='b',alpha = 0.5)
	p1 = ax.bar(histo_in[:,0], histo_in[:,1], width, color='r', alpha = 0.5)
	
	ax.set_ylabel('Bird Count')
	ax.set_xlabel('Time (Hour)')
	
	#now = date_string

	#bar_title = 'Frequency of Birds : [' + str(now.year) + '.' + str(now.month) + '.' + str(now.day) + ']'
	bar_title = 'Frequency of Birds : [' + date_string + ']'
	ax.set_title(bar_title)
	ax.set_xticks(np.arange(0,24,1))
	ax.legend((p1[0], p2[0]), ('Bird In', 'Bird Out'))
	
	plt.savefig(histo_img_path)
	#plt.show()
	
	
	
# load files
txt_inFile =sys.argv[1]
txt_outFile =sys.argv[2]
histo_img_path =sys.argv[3]
#txt_inFile ='C:/Users/yapws87/Desktop/FlightOfSwallows/2018-10-09_in.txt'
#txt_outFile ='C:/Users/yapws87/Desktop/FlightOfSwallows/2018-10-09_out.txt'
#histo_img_path ='C:/Users/yapws87/Desktop/FlightOfSwallows/histo_2018-10-09.jpg'
#

file_inBird = open(txt_inFile,'r')
file_outBird = open(txt_outFile,'r')

time_interval = 5#minutes


# Extract Data

birdRes_in = extractData(file_inBird,time_interval)
birdRes_out = extractData(file_outBird,time_interval)

histoname = Path(histo_img_path)
print(histoname.stem)
drawHisto(birdRes_in.histo,birdRes_out.histo,time_interval / 5,histo_img_path,histoname.stem)
print ('In:', birdRes_in.total, '\t','Out:', birdRes_out.total)