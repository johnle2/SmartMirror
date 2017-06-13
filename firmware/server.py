#pip import pyown
#pip import praw

# -*- coding: utf-8 -*-
from time import gmtime, strftime, sleep, localtime
from subprocess import call
import os.path
import pyowm
import praw
import signal,sys

WEATHER_API_KEY = "d3041fa4dec2792d29c137b03e0f5bd0"
WEATHER_LOC = "Champaign, IL"

REDDIT_CLIENT = "Bthp4CLVJkNwOw"
REDDIT_SECRET = "vv24h5SBYahhn-L-_hApDFQvCKk"
REDDIT_USER = "python:SmartMirrorServer:v1.0 (by /u/JamMore)"
SUBREDDIT = "news"

SCREEN_SESSION = "mirror"
USB_PATH = "/dev/tty.usbserial-A50285BI"
BT_PATH = "/dev/tty.HC-06-DevB"
UART_PATH = ''
BAUD_RATE = 115200

def connectUART(path):
    call("screen -S " + SCREEN_SESSION + " -d -m " + path + " " + str(BAUD_RATE), shell=True)
    print 'sending to shell: '"screen -S " + SCREEN_SESSION + " -d -m " + path + " " + str(BAUD_RATE)
def printUART(message):
    call("screen -S " + SCREEN_SESSION + ' -p 0 -X stuff "' + message + '"', shell=True)
def disconnectUART():
    call("screen -S " + SCREEN_SESSION + " -X quit", shell=True)

def getDateTime():
    return strftime("%Y-%m-%d %H:%M", localtime()) 

def getWeather():
    owm = pyowm.OWM(WEATHER_API_KEY)
    observation = owm.weather_at_place(WEATHER_LOC)
    w = observation.get_weather()
    return [w.get_status(), w.get_temperature('fahrenheit')]

def getHeadlines():
    r = praw.Reddit(client_id=REDDIT_CLIENT, user_agent=REDDIT_USER, client_secret=REDDIT_SECRET)
    submissions = r.subreddit(SUBREDDIT).hot(limit=5)
    return [str(x.title.encode('ascii', 'ignore')) for x in submissions] 

def getInfo(): 
    info = {}
    info['DateTime'] = getDateTime()
    info['Weather'] = getWeather()
    info['Headlines'] = getHeadlines()
    return info

def signal_handler(signal, frame):
    print "exiting cleanly"
    disconnectUART()
    sys.exit(0)

def server(): 

    if os.path.isfile(USB_PATH):
        UART_PATH = USB_PATH
    else:
        UART_PATH = BT_PATH

    UART_PATH = BT_PATH
    print getInfo()
    signal.signal(signal.SIGINT, signal_handler)
    connectUART(UART_PATH)
    delay = 3
    sleep(delay)
    while 1:
        printUART(getDateTime().split(' ')[0] +"\r")
        print getDateTime().split(' ')[0]
        sleep(delay)
        printUART(getDateTime().split(' ')[1] +"\r")
        print getDateTime().split(' ')[1]
        sleep(delay)

        printUART("cold\r")
        sleep(delay)
        printUART("gray\r")
        sleep(delay)
        printUART("death!\r")
        sleep(delay)
        printUART("famine!\r")
        sleep(delay)
        printUART("poverty!\r")
        sleep(8)
    disconnectUART()
    signal.pause()

#server()
0
