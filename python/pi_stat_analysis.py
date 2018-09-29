		
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
		if  15 < int(data[5]) < 70:
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
	
	return BirdResult(arr_histo,total_bird)
	#print('Total Bird : ', total_bird )
	#return arr_histo