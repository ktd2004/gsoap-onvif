###Project Introduction


The project is a onvif client project,
through the project,you can learn
how to get the IPC(ip camera)'s RTSP URL, video configuration, WSDL address, and the Device capabilities(such as PTZ),
The project have the security check moudle

About the API function ,you can learn from below website

http://www.onvif.org/onvif/ver20/util/operationIndex.html

----

###ONVIF official website
http://www.onvif.org/

###The Gsoap Toolkit
http://www.cs.fsu.edu/~engelen/soap.html

My project is Base on gsoap 2.8-17
###download
http://sourceforge.net/projects/gsoap2/files/

for example,you get the RTSP URL, you can paly  the video use the player,such as VLC player.

###VLC offical website
http://www.videolan.org/

and you can save the video use the ffmpeg tool,

for example 
ffmpeg -i ffmpeg -i rtsp://admin:admin@172.18.4.100:554 -b 300 -s 320x240 -vcodec copy  -ab 32 -ar 24000 -acodec aac -strict experimental -f mp4 test.mp4

you can lear more
###ffmpeg official website
https://www.ffmpeg.org/





