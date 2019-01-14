import sys
from pathlib import Path
import bird_log_func as b_log
import numpy as np
	
##--------------------------- Main Process --------------------------------
	
# load files
txt_inFile =sys.argv[1]
txt_outFile =sys.argv[2]
histo_img_path =sys.argv[3]
txt_dailytxt_path = sys.argv[4]
line_img_path =sys.argv[5]
#status_path=sys.argv[6]

file_inBird = open(txt_inFile,'r')
file_outBird = open(txt_outFile,'r')
#file_status = open(status_path,'r')

histoname = Path( )
txtname = Path(txt_inFile)

time_interval = 5#minutes
date_str = txtname.stem.split('_')[0]
date_stamp = b_log.getDateFromString(date_str)

# Extract Data
birdRes_in = b_log.extractData(file_inBird,time_interval)
birdRes_out = b_log.extractData(file_outBird,time_interval)


speedName = str(histoname.parent) + "/" + str(histoname.stem) + "_speed" + str(histoname.suffix) 
trailName = str(histoname.parent) + "/" + str(histoname.stem) + "_trail" + str(histoname.suffix)

# Calculate Peak
peak_bo, mu_bo, std_bo, max_bo= b_log.getHistoStat(birdRes_out.histo,6,7.2)
peak_bi,mu_bi, std_bi, max_bi = b_log.getHistoStat(birdRes_in.histo,18.5,20.0)

# Calculate Area
area_bo = b_log.calculate_norm_area(mu_bo, std_bo, max_bo, time_interval)
area_bi = b_log.calculate_norm_area(mu_bi, std_bi, max_bi, time_interval)

# Draw histogram
b_log.drawHisto(birdRes_in.histo,birdRes_out.histo,time_interval / 5,histo_img_path	,date_str,1, 'Frequency Of Birds', 'Bird Count',mu_bo, mu_bi, std_bo, std_bi, max_bo, max_bi)



bird_acc = 0
if birdRes_in.total > birdRes_out.total:
	bird_acc = birdRes_out.total / float(birdRes_in.total) * 100
else :
	bird_acc = birdRes_in.total / float(birdRes_out.total) * 100

print ('Total_In: {} | Total_Out: {} |'.format( birdRes_in.total, birdRes_out.total ))
print ('Acc: {} |'.format( int(bird_acc) ))
print ('Peak_In: {} | Peak_Out: {} |'.format( int(peak_bi), int(peak_bo) ))
print ('Area_In: {} | Area_Out: {} |'.format( int(area_bi), int(area_bo) ))


b_log.saveData(txt_dailytxt_path,date_stamp, peak_bo, peak_bi)
b_log.plotBirdTrendLine(txt_dailytxt_path,line_img_path)

file_inBird.close()
file_outBird.close()
#file_status.close()