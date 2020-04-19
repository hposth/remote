from microbit import *

while True:
    x = str(accelerometer.get_x());
    y = str(accelerometer.get_y());
    l = str(int(button_a.is_pressed()));
    r = str(int(button_b.is_pressed()));

    for i in range(0, 6-len(x)):
        x = '=' + x;

    for i in range(0, 6-len(y)):
        y = '=' + y;

    print("n{0}{1}{2}{3}".format(x, y, l, r), end='');
    sleep(16) # because 60Hz is about 16.67 frames per second, so an update every 16 seconds should make for smooth handling.