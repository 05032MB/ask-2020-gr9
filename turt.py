from turtle import *
from serial import *
from time import sleep

turt = Turtle()

ser = Serial("/dev/ttyUSB1", 9600)

showturtle()

def turt_debug_bearing():
    print("Turt bearing: ", turt.heading())

def onward():
    bear = (turt.heading() - 90) // 45;
    if (bear < 0):
        bear = 8 - abs(bear)
        
    print("Bear: ", bear)
    bear = bear + 48
    
    turt.forward(45)
    ser.write(chr(int(bear)).encode('utf-8'))

def rot_left():
    turt.left(45)
    turt_debug_bearing()

def rot_right():
    turt.right(45)
    turt_debug_bearing()

def toggle_write():
    ser.write('8'.encode('utf-8'))
    if(turt.isDown()):
        turt.up()
    else:
        turt.down()

#def back():
#    turt.back(45)

onkey(bye, "q")

sleep(5)
print("Robot gotowy")

onkey(onward, "Up")
onkey(rot_left, "Left")
onkey(rot_right, "Right")
onkey(toggle_write, 'u')
#onkey(back, "Down")

listen()
mainloop()
