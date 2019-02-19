import sys
#import datetime
from datetime import datetime, date, time
from datetime import timedelta
import numpy as np
import matplotlib
matplotlib.use('Agg')
from matplotlib.dates import date2num
from matplotlib import pyplot as plt
import matplotlib.mlab as mlab
import math

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
		#histo_time = str(timedelta(seconds=time_count))
		histo.append([time_count / 3600.0,0,0,0])
		time_count = time_count + bin_seconds
	
	
	time_anchor = 0
	bin_acc = 0
	speed_acc = 0
	trail_acc = 0
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
		trail_acc = trail_acc + int(data[6])
		total_bird = total_bird + 1
		
		#print(1,data_time - time_anchor,bin_seconds * 1.2)
		while (data_time - time_anchor) >=  bin_seconds * 0.9:
			#bar_time = str(timedelta(seconds=time_anchor))
			time_anchor = time_anchor + bin_seconds
			#print(2,data_time - time_anchor,bin_seconds,bin_acc)
			if (data_time - time_anchor) < bin_seconds * 0.9:
				
				hist_time = time_anchor / 3600.0
				hist_bird_count = bin_acc
				hist_speed = speed_acc / (bin_acc + 0.001)
				hist_trail = trail_acc / (bin_acc + 0.001)
				
				# minimum bird to consider as not noise
				if hist_bird_count < 10 :
					hist_speed = 0
					hist_trail = 0
								
				histo[idx] = [ hist_time, hist_bird_count , hist_speed, hist_trail ]
				speed_acc = 0
				bin_acc = 0
				trail_acc = 0

				#print(idx,histo[idx])

			else:
				histo[idx] = [time_anchor  / 3600.0, 0,0,0]
			
			
			idx = idx + 1
			
		
		if idx >= total_bins:
			break
			
	
	arr_histo = np.array(histo).reshape(len(histo),4)
	#print(np.transpose(arr_histo[:,1]))
	#print('Total Bird : ', total_bird )
	return BirdResult(arr_histo,total_bird)
	
	#return arr_histo

def extract_status_data(file_status, time_interval_mins):

	# Initialize
	histo = []
	time_count = 0
	total_bins = (60 * 24) / time_interval_mins # Calculates the total bin for a single day
	count_time = 0

	for line in file_status:
		line.replace('\t','\s')
		#print(line)
		data = line.split()
		
		# Extract time infomation
		data_time_token = data[3].split(':')
		data_time = datetime(2000,1,1,int(data_time_token[0]),int(data_time_token[1]))
		data_time = (data_time - datetime(2000,1,1)).total_seconds()

		# Extract status infomation
		saturation	 = data[5].split('=')[1]
		overflow	 = data[6].split('=')[1]
		proc_time 	= data[7].split('=')[1]
		bird_in 	= data[8].split('=')[1]
		bird_out 	= data[9].split('=')[1]
		pi_temperature = data[10].split('=')[1].split("'")[0] # get only the number
		pi_core		 = data[11].split('=')[1]

		
		#print(data_time,saturation,overflow, pi_temperature,pi_core)

		
			

def calculate_norm_area(mu, std, max_val, interval):
		
	if mu == 0:
		return 0	
	pdf = mlab.normpdf(mu, mu, std)
	multiplier = max_val / pdf
	bin_ = interval / 60.0

	index = mu
	area = max_val
	area_part = max_val 
	while area_part > 10:
		index += bin_
		#print('a:{} i:{} b:{}'.format(area_part,index,bin_))
		area_part = mlab.normpdf(index, mu, std) * multiplier
		area += 2 * area_part
		
	return area

# Draws histogram
def drawHisto(histo_in,histo_out,hist_width,histo_img_path,date_string,graph_type, histo_title, histo_y_axis_name,mu1,mu2,sig1,sig2,max1,max2 ):

	fig, ax = plt.subplots(figsize=(10,5))
	
	#ind = np.arange(N)    # the x locations for the groups
	width = 0.08333  * hist_width    # the width of the bars: can also be len(x) sequence


	p2 = ax.bar(histo_out[:,0], histo_out[:,graph_type], width, color='b',alpha = 0.5)
	p1 = ax.bar(histo_in[:,0], histo_in[:,graph_type], width, color='r', alpha = 0.5)
	

	x = np.arange(0,24,0.01)

	if mu1 > 0 :
		pdf1 = mlab.normpdf(mu1, mu1, sig1)
		multiplier1 = max1 / pdf1
		ax.plot( x ,mlab.normpdf(x , mu1, sig1) * multiplier1,'--')
	
	if mu2 > 0 :
		pdf2 = mlab.normpdf(mu2, mu2, sig2)
		multiplier2 = max2 / pdf2
		ax.plot( x ,mlab.normpdf(x , mu2, sig2) * multiplier2,'--')

	ax.set_ylabel(histo_y_axis_name)
	ax.set_xlabel('Time [Hour]')
	
	#now = date_string

	#bar_title = 'Frequency of Birds : [' + str(now.year) + '.' + str(now.month) + '.' + str(now.day) + ']'
	bar_title = histo_title + ' : [' + date_string + ']'
	ax.set_title(bar_title)
	ax.set_xticks(np.arange(0,24,1))
	ax.legend((p1[0], p2[0]), ('Bird In', 'Bird Out'))
	
	plt.savefig(histo_img_path)
	#plt.show()
	
def getHistoStat(histo_data,time_start, time_end):
	
	# Get data info
	bin_total = len(histo_data)
	bin_val = 24 / float(bin_total)
	
	#print(bin_total)
	#print(bin_val)
	#print(histo_data)
	acc_data = 0
	mean = 0
	data_collect = []
	weights = 0

	max_index = 0
	max_value = 0
	max_time = 0
	for i in range(1,bin_total):
		loop_time = i * bin_val
		

		if loop_time >=  time_start and loop_time <= time_end:
			acc_data = acc_data + histo_data[i,1]
			real_time = loop_time + bin_val / 2
			
			# calculate mean
			mean += real_time * histo_data[i,1]
			weights += histo_data[i,1]

			# collect data
			data = histo_data[i,1]
			data_collect.append((real_time, data))

			# identify the peak 
			if data > max_value:
				max_value = data
				max_index = i
				max_time = real_time
	# no data in the specified region
	if weights <= 0:
		return 0 , 0, 0 , 0
	
	mean = mean / weights


	# Search for gaussian range from the peak
	weights = max_value
	bot_signal = False
	i = max_index
	data_collect_2 = []
	data_collect_2.append((max_time,max_value))
	pre_data = max_value
	while not bot_signal:
		i = i - 1
		real_time = i * bin_val + bin_val / 2
		data = histo_data[i,1]
		diff = abs(pre_data - data)

		# Break if the signal is higher
		if diff > pre_data * 0.6 or not data :
			bot_signal = True
		else:
			data_collect_2.append((real_time,data))
			weights = weights + data
			pre_data = data

	bot_signal = False
	pre_data = max_value
	i = max_index
	while not bot_signal:
		i = i + 1
		real_time = i * bin_val + bin_val / 2
		data = histo_data[i,1]
		diff = abs(pre_data - data)

		# Break if the signal is higher
		if diff > pre_data * 0.5 or not data:
			bot_signal = True		
		else:
			data_collect_2.append((real_time,data))
			weights = weights + data
			pre_data = data
	
	#print(data_collect_2)
	std = 0
	max_val = 0
	#for dat in data_collect:
	for dat in data_collect_2:
		#std += (dat[0] - mean) * (dat[0] - mean) * dat[1]
		std += (dat[0] - max_time) * (dat[0] - max_time) * dat[1]
		if dat[1] > max_val:
			max_val = dat[1]
	
	total_data = len(data_collect_2)
	denom = (total_data - 1) / float(total_data) * (weights - 1)
	std = math.sqrt(std / denom) 
	
	#print(histo_data[i,1])

	return acc_data, mean, std, max_val

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
	
	#print(date_entry)
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
	
		