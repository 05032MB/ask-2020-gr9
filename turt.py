from turtle import *
from serial import *
from time import sleep
import os
import sys

def get_usb_device():
    usb_devices = list(filter(lambda d : 'usb' in d.lower(), os.listdir('/dev/')))
    if len(usb_devices) == 1:
        print(f'Found {usb_devices[0]}')
        return usb_devices[0]
    print("Which device is the ROBOT?")
    for i, d in enumerate(usb_devices):
        print(f'{i}: {d}')
    idx = int(input())
    return usb_devices[idx]

turt = Turtle()
usb = get_usb_device()

ser = Serial(f"/dev/{usb}", 9600)

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
    if(turt.isdown()):
        turt.up()
    else:
        turt.down()

def send_char(char : str):
    print('Diode flipped')
    ser.write(char.encode('utf-8'))

#def back():
#    turt.back(45)

def manual():
    onkey(bye, "q")
    onkey(onward, "Up")
    onkey(rot_left, "Left")
    onkey(rot_right, "Right")
    onkey(toggle_write, 'u')
    onkey(turt.down, 'D')
    onkey(turt.up, 'U')
    onkey(lambda : send_char('9'), 'x')
    #onkey(back, "Down")
    listen()
    mainloop()

def from_args(command_list):
    commands = {
        'u' : turt.up,
        'd' : turt.down,
        'f' : onward,
        'l' : rot_left,
        'r' : rot_right,
        't' : toggle_write
    }
    for command in command_list:
        try:
            commands[command]()
        except Exception as e:
            print(e)
            break
    else:
        print("Command list executed")

if __name__ == '__main__':
    sleep(5)
    print("Robot gotowy")
    if len(sys.argv) == 1:
        manual()
    elif len(sys.argv) > 1:
        from_args(sys.argv[1])