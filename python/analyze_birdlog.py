import sys
#import datetime
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
		bin_acc  = bin_acc + 1
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

# Draws histogram
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
	
def getHistoStat(histo_data,time_start, time_end):
	
	# Get data info
	bin_total = len(histo_data)
	bin_val = 24 / float(bin_total)
	
	acc_data = 0;
	
	for i in range(1,bin_total):
		loop_time = i * bin_val
		
		if loop_time >=  time_start and loop_time <= time_end:
			acc_data = acc_data + histo_data[i,1]
			
	return acc_data

# saves the daily bird count
def saveData(txt_dailytxt_path,date_stamp, bird_out, bird_in):
	daily_file = open(txt_dailytxt_path, "a")
	
	date_string = [str(date_stamp.year), "-", str(date_stamp.month), "-", str(date_stamp.day) ]
	daily_file.write(''.join(date_string))
	
	bird_out = int(bird_out)
	bird_in = int(bird_in)
	data_string = [ "\t", str(bird_out), "\t",str(bird_in),"\n"]
	daily_file.write(''.join(data_string))
	
	daily_file.close()

# Change date string YYYY-MM-DD to datetime
def getDateFromString(date_entry):
	
	print(date_entry)
	year, month, day = map(int, date_entry.split('-'))
	local_date = date(int(year),int(month),int(day))
		
	return local_date
	
# Plots the daily bird line graph
def plotBirdTrendLine(data_filename, graph_path):
	# Extract data from file
	daily_file = open(data_filename, "r")
	
	b_dates = []
	b_outs = []
	b_ins = []
	lines = daily_file.readlines()
	for line in lines:
		b_date, b_out, b_in = line.split('\t')	
		b_dates.append(b_date)
		b_outs.append( int(float(b_out)))
		b_ins.append( int(float(b_in)))
	daily_file.close()
	
	# Draw graph
	x_axis = np.arange(0,len(b_dates))
	
	
	fig, ax = plt.subplots(figsize=(10,5))
	
	line1, line2, = plt.plot(x_axis, b_outs,'b',x_axis, b_ins,'r')
	
	ax.set_ylabel('Bird Count (During Peak Hours)')
	ax.set_xlabel('Date')
	
	bar_title = 'Daily Bird Count'
	ax.set_title(bar_title)
	
	ax.legend((line1, line2), ('Bird Out', 'Bird In'))
	
	
	plt.xticks(x_axis, b_dates, rotation='vertical')
	# Pad margins so that markers don't get clipped by the axes
	plt.margins(0.25)
	# Tweak spacing to prevent clipping of tick-labels
	plt.subplots_adjust(bottom=0.25)
	plt.grid()
	plt.savefig(graph_path)
	#plt.show()
	
		
	
##--------------------------- Main Process --------------------------------
	
# load files
txt_inFile =sys.argv[1]
txt_outFile =sys.argv[2]
histo_img_path =sys.argv[3]
txt_dailytxt_path = sys.argv[4]
line_img_path =sys.argv[5]
#txt_inFile ='C:/Users/yapws87/Desktop/FlightOfSwallows/2018-10-12_in.txt'
#txt_outFile ='C:/Users/yapws87/Desktop/FlightOfSwallows/2018-10-12_out.txt'
#histo_img_path ='C:/Users/yapws87/Desktop/FlightOfSwallows/histo_2018-10-12.jpg'

file_inBird = open(txt_inFile,'r')
file_outBird = open(txt_outFile,'r')

histoname = Path(histo_img_path)
txtname = Path(txt_inFile)

time_interval = 5#minutes
date_str = txtname.stem.split('_')[0]
date_stamp = getDateFromString(date_str)

# Extract Data
birdRes_in = extractData(file_inBird,time_interval)
birdRes_out = extractData(file_outBird,time_interval)


drawHisto(birdRes_in.histo,birdRes_out.histo,time_interval / 5,histo_img_path,date_str)
print ('In:', birdRes_in.total, '\t','Out:', birdRes_out.total)

main_bird_out = getHistoStat(birdRes_out.histo,6,7)
main_bird_in = getHistoStat(birdRes_in.histo,18.0,19.0)

print ('Morning Bird: ', main_bird_out, '\t','Evening Bird: ', main_bird_in)




saveData(txt_dailytxt_path,date_stamp, main_bird_out,main_bird_in)

plotBirdTrendLine(txt_dailytxt_path,line_img_path)
