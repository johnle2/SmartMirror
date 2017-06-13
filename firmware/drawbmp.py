#Python wrapper for computer server, which transmit the 
#UI and necessary data to the Cortex M0 via Bluetooth
#which will be displayed on TFT screen

import PIL
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

import server as serv
import time 
import os
#CONSTS
SCREEN = (480, 800)

#COLOR PALETTE
BACKGROUND_COLOR    = (0,0,0)  
PRIMARY_TEXT        = (255,255,255)  
SECONDARY_TEXT      = (0,0,0)  
TERTIARY_TEXT       = (0,0,0)  

#Fonts
CLOCK_FONT_PATH = "fonts/1.otf"
FONT_PATH = "fonts/1.ttf"
font1 = ImageFont.truetype(CLOCK_FONT_PATH,120)
font2 = ImageFont.truetype(FONT_PATH,40)
font3 = ImageFont.truetype("fonts/2.otf",22)
font4 = ImageFont.truetype(CLOCK_FONT_PATH,80)

NEWS = "Today's Headlines:"
#FUNCTION TO FORMAT HEADLINES. (Text goes off the screen, multiple lines, etc.)
def formatNews(news):
    LINE_NUM = 34 
    para = [] 
    for head in news:
        t = ""
        if len(head) > LINE_NUM: 
            t = head[:LINE_NUM] + "\n   " + head[LINE_NUM:]
        if len(head) > LINE_NUM*2: 
            t = t[:LINE_NUM*2] + "\n   " + t[LINE_NUM*2:]
        if len(t) > 0: 
            para.append(t)
        else:
            para.append(head)
    return "-- " + "\n\n-- ".join(para)



def drawScreen(data):
	img = Image.new("RGBA", SCREEN, BACKGROUND_COLOR)
	draw = ImageDraw.Draw(img)

	TIME = data["DateTime"].split()[1]
	DATE = data["DateTime"].split()[0]
	TEMP = str(data["Weather"][1]['temp'])[:2] + " F"
	WEAT = str(data["Weather"][0])
	ALL_NEWS = formatNews(data["Headlines"][:3])
    
	im = Image.open("weather/" + WEAT + ".png").convert('RGBA')    
    #DRAWING START
	draw.text((20, 30),     TIME,   PRIMARY_TEXT,   font=font1)
	draw.text((65, 170),    DATE,   PRIMARY_TEXT,   font=font2)
	img.paste(im, (40,300), im)
	draw.text((200, 300),    TEMP,   PRIMARY_TEXT,   font=font4)
	draw.text((20, 500),    NEWS,   PRIMARY_TEXT,   font=font2)
	draw.text((20, 550),    ALL_NEWS,   PRIMARY_TEXT,   font=font3)
    #DRAWING END

	draw = ImageDraw.Draw(img)
	img = img.convert(mode="P", colors=255)
	img = img.rotate(90, expand=True)
	img.save("a_test.bmp")

def drawTime(data):
	img = Image.new("RGBA", (300,160), BACKGROUND_COLOR)
	draw = ImageDraw.Draw(img)
	TIME = data["DateTime"].split()[1]
	draw.text((20, 30),     TIME,   PRIMARY_TEXT,   font=font1)
	draw = ImageDraw.Draw(img)
	img = img.convert(mode="P", colors=255)
	img = img.rotate(90, expand=True)
	img.save("a_test.bmp")

def update_mirror():
	data = serv.getInfo()
	drawScreen(data);
	os.system("sudo ./main a_test.bmp 0 0")
	a = 0
	print "start"
	while 1:
		time.sleep(1)
		data = serv.getInfo()
		a +=1
		if a % 240 == 0:		#update once every 4 hours
			drawScreen(data)
			os.system("sudo ./main a_test.bmp 0 0")
		elif a % 60 == 0:
			print "update"
			drawTime(data)
			os.system("sudo ./main a_test.bmp 0 0")

update_mirror()