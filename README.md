
# blink1_monitor

A blink(1) daemon that shows the computer's utilization through color.

Red channel = CPU usage

Green channel = Network i/o

Blue channel = Disk i/o

More infomation on the blink(1) thingy is here: http://thingm.com/products/blink-1.html

# Disclaimers

Only supported on OSX. Poorly documented.

# Installation

0) Make sure you have Make & Gcc (such as by installing Xcode or Command Line Tools For Xcode)

1) Run make

2) Manually run the tool by running `build/blink1_monitor`. Make sure that works.

To install as a service that launches on startup:

3) Edit the file `paulhodge.blink1_monitor.plist`. You'll need to change this line:

    <string>/Users/andy/blink1_monitor/build/blink1_monitor</string>

To an absolute path pointing to the build/blink1_monitor file on your disk.

4) Restart the computer, or just run these commands:

    launchctl load paulhodge.blink1_monitor.plist
    launchctl start paulhodge.blink1_monitor

# Credits

Hacked together by Andrew Fischer.

Based on sources from ThingM found here: https://github.com/todbot/blink1/tree/master/commandline

Also uses source code from libtop, part of Darwin released by Apple Computer.

See LICENSE.txt for license terms.
