from twython import Twython
import sys
import socket

REMOTE_SERVER = "www.google.com"
def is_connected(hostname):
  try:
    # see if we can resolve the host name -- tells us if there is
    # a DNS listening
    host = socket.gethostbyname(hostname)
    # connect to the host -- tells us if the host is actually
    # reachable
    s = socket.create_connection((host, 80), 2)
    return True
  except:
     pass
  return False
#timeit is_connected(REMOTE_SERVER)

if not is_connected(REMOTE_SERVER):
    sys.exit(1)



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


message = sys.argv[1]
photo = open(sys.argv[2],'rb')
response = twitter.upload_media(media=photo)
twitter.update_status(status=message, media_ids=[response['media_id']])