from microbit import *

MAX_DATA_LENGTH = 16

while True:
    pad = "=" * MAX_DATA_LENGTH;
    sub = "({0},{1},{2},{3})".format(str(accelerometer.get_x()), str(accelerometer.get_y()), int(button_a.is_pressed()), int(button_b.is_pressed()))
    dat = sub + pad[len(sub):]
    print(dat)
    sleep(5)