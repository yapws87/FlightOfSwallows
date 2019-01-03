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
#'C:/Users/yapws87/Desktop/FlightOfSwallows/2018-10-18_status.txt'
#txt_inFile ='C:/Users/yapws87/Desktop/FlightOfSwallows/2018-10-12_in.txt'
#txt_outFile ='C:/Users/yapws87/Desktop/FlightOfSwallows/2018-10-12_out.txt'
#histo_img_path ='C:/Users/yapws87/Desktop/FlightOfSwallows/histo_2018-10-12.jpg'

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
b_log.drawHisto(birdRes_in.histo,birdRes_out.histo,time_interval / 5,histo_img_path,date_str,1, 'Frequency Of Birds', 'Bird Count')
#drawHisto(birdRes_in.histo,birdRes_out.histo,time_interval / 5,speedName,date_str,2, 'Speed Of Birds', 'Speed [km/h]')
#drawHisto(birdRes_in.histo,birdRes_out.histo,time_interval / 5,trailName,date_str,3, 'Trail Of Birds', 'Trail Count')

print ('Total_In : ', birdRes_in.total, ' | ','Total_Out: ', birdRes_out.total, '|\n')

bird_acc = 0
if birdRes_in.total > birdRes_out.total:
	bird_acc = birdRes_out.total / float(birdRes_in.total) * 100
else :
	bird_acc = birdRes_in.total / float(birdRes_out.total) * 100

print ('Acc : ', int(bird_acc), '%|\n')

main_bird_out = b_log.getHistoStat(birdRes_out.histo,5.0,20.0)
main_bird_in = b_log.getHistoStat(birdRes_in.histo,5.0,20.0)


print ('Peak_In : ', int(main_bird_in), ' | ', 'Peak_Out: ', int(main_bird_out),'|\n')

#extract_status_data(file_status,5)

b_log.saveData(txt_dailytxt_path,date_stamp, main_bird_out,main_bird_in)

b_log.plotBirdTrendLine(txt_dailytxt_path,line_img_path)

file_inBird.close()
file_outBird.close()
#file_status.close()