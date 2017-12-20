import cv2
import picamera
import time
from picamera.array import PiRGBArray
import numpy as np
import serial
import math

def bestValue(bot,noOfBots):
    cx=0
    cy=0
    valid=False
    x=np.empty(noOfBots,dtype=int)
    y=np.empty(noOfBots,dtype=int)
    for i in range(0,noOfBots-1):
        M=cv2.moments(bot[i])
        if(M["m00"]!=0):
            x[i]=int(M["m10"]/M["m00"])
            y[i]=int(M["m01"]/M["m00"])
        else:
            continue
        
    #calculate median
    x.sort()
    y.sort()

    #median=(n+1)/2 th item
    #(n+1)=noOfBots
    #divmod gives quotient and remainder
    q,rem=divmod(noOfBots,2)

    if(q==0):
        valid=False
        return cx,cy,valid
    
    if(rem%2==0):
        cx=x[q]
        cy=y[q]
        valid=True
    else:
        if((q-1)<0):
            return cx,cy,valid
        else:
            cx=(x[q-1]+x[q])/2
            cy=(y[q-1]+y[q])/2
            valid=True

    if(cx>112 or cy>112):
        valid=False
        
    return cx,cy,valid

#start camera
camera=picamera.PiCamera()
camWidth=112
camHeight=112
camera.resolution=(camWidth,camHeight) 
camera.framerate=32 #90 max
rawCapture=PiRGBArray(camera,size=(camWidth,camHeight))
time.sleep(0.1)

#get Arduino
usb=serial.Serial('/dev/ttyACM0',9600)
usb.timeout=0 #dont wait for data
serialStop=False #store the serial state
stopCode='S'
startCode='G'

noise=20

#variable to store the actual shape
bot=np.empty(135,dtype=object) #empty array of given size
noOfBots=0 #index for bot array
counter=0 #variable to control the object detect loop before serial check
maxcounter=0 #variable to check to send data to arduino
co=None #empty object to store contours temporarily

#threshold value for red in hsv
lower_red=np.array([-10,50,50])
higher_red=np.array([10,255,255])

preNum=0;

#main loop
while(True):

    #check switch
    if(True):

        #check Serial bus for commands
        c=usb.read()
        if(c=='S' or c=='G'):
            print(c)
        if((c==stopCode or serialStop) and c!=startCode):
            serialStop=True
            counter=0
            maxcounter=0
            bot=np.empty(135,dtype=object)
            noOfBots=0
            
        else:
            serialStop=False
            
            if(maxcounter>=20):
                print "max reached"

                #get best value among all for the bot
                cx,cy,valid=bestValue(bot,noOfBots)
                if(valid):
                    print cx
                    #print cy
                    #dont transmit cy because neither robots will fly
                    #can be used in other filtering

                    if(cx<20):
                        usb.write(str(0))
                    elif(cx>90):
                        usb.write(str(9))
                    else:
                        #i have 1-8 and 21-89 to encode
                        #so (90-20)/8=8.75 i.e. 8.75 px=1 encoded data
                        coded=(int)(((cx-20)//8.75)+1)
                        usb.write(str(coded))
  
                else:
                    
                    print "Invalid"
                    #new code added
                    usb.write("N")
                    
                maxcounter=0
                counter=0
                bot=np.empty(135,dtype=object)
                noOfBots=0

            maxcounter=maxcounter+27
                    
            #imageprocess
            for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
                image=frame.array #get image in form of array
                imageGray=cv2.cvtColor(image,cv2.COLOR_BGR2GRAY) #convert to grayscale

                #change minlevel and maxlevel as needed
                edge=cv2.Canny(image,100,200,True) #get the edges
                cpedge=edge
                
                #find all the contours(shapes)
                contours, hierarchy = cv2.findContours(cpedge,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)

                #sort the contours in ascending order of area i.e find the 10 contours
                #of highest area
                contours=sorted(contours,key=cv2.contourArea,reverse=True)[:10]
                

                preNum=noOfBots
                #loop over the contours to find the required one
                for i,c in enumerate(contours):
                    
                    #approximate the contour to 4 points (for a rectangle)
                    peri=cv2.arcLength(c,True)
                    #approx=cv2.approxPolyDP(c,0.1*peri,True)
                    approx=cv2.convexHull(c)
                    

                    #if the contour has four points,it is the required rectangle
                    #note that the first rectangle found will be saved
                    #since the contours are in ascending order and no other rectangle
                    #will be in the arena, the biggest should be the bot

                    rect=cv2.minAreaRect(c)
                    width=rect[1][0]
                    height=rect[1][1]
                    periCalc=2*(width+height)
                    area=cv2.contourArea(c)
                    co=None
                    #abs(periCalc-peri)<noise and
                    if(len(approx)>=4 and area>20):
                        #mask the contour onto another image
                        mask=np.zeros_like(image)
                        cv2.fillConvexPoly(mask,approx,(255,255,255))
                        newImg=cv2.bitwise_and(image,mask)

                        #change to hsv and convert to binary with red as white
                        hsv=cv2.cvtColor(newImg,cv2.COLOR_BGR2HSV)
                        redImg = cv2.inRange(hsv,lower_red,higher_red);

                        cv2.imshow("asd",redImg)

                        isRed,hierarchy = cv2.findContours(redImg,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
                        isRed=sorted(isRed,key=cv2.contourArea,reverse=True)

                        if(len(isRed)>0):
                            #register as bot
                            bot[noOfBots]=approx
                            noOfBots=noOfBots+1
                            co=approx
                            break
    

                #nothing found then check color only
                if(noOfBots==preNum):
                    hsv=cv2.cvtColor(image,cv2.COLOR_BGR2HSV)
                    redImg = cv2.inRange(hsv,lower_red,higher_red);

                    cv2.imshow("asd",redImg)

                    isRed,hierarchy = cv2.findContours(redImg,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
                    isRed=sorted(isRed,key=cv2.contourArea,reverse=True)

                    if(len(isRed)>0):
                        #register as bot
                        approx=cv2.convexHull(isRed[0])
                        bot[noOfBots]=approx
                        noOfBots=noOfBots+1
                        co=approx

                #draw over the contour
                cv2.drawContours(image,[co],-1,(0,255,0),3)

                key=cv2.waitKey(1)

                cv2.imshow("Edges",edge)
                cv2.imshow("Image",image)

        
                rawCapture.truncate(0)

                #count the no of objects detected
                counter=counter+1

                if(counter>=maxcounter):
                    cv2.destroyAllWindows()
                    break
            
    

