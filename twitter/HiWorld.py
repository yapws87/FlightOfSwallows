from twython import Twython
from time import localtime, strftime

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

time_str = strftime("%a, %d %b %Y %H:%M:%S", localtime())
 
message = "Hello twitter!! I am online... beep beep beep\n" + time_str
twitter.update_status(status=message)
print('Tweeted')