from twython import Twython
from twit_auth import (
      consumer_key,
      consumer_secret,
      access_token,
      access_token_secret
  )
 
twitter = Twython(
      consumer_key,
      consumer_secret,
      access_token,
      access_token_secret
  )
 
message = "Testing a video file again..."


video = open('/home/pi/Downloads/SampleVideo_360x240_1mb.mp4','rb')
response = twitter.upload_video(media=video,media_type='video/mp4')
twitter.update_status(status=message, media_ids=[response['media_id']])
