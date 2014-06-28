hlock - simple hourly screen locker for PC users
============================
Good healthy habbits
its a modification of xlock
simply blank screen for 5mins(default) on every hour 


Requirements
------------
In order to build hlock you need the Xlib header files.


Installation
------------
Edit config.mk to match your local setup (hlock is installed into
the /usr/local namespace by default).

**Default take 3mins break on the hour**
BREAK_DELAY(Default) = 3(mins)
BREAK_INTERVAL(Default) = 60(mins)

Afterwards enter the following command to build and install hlock
(if necessary as root):

    make clean install


Running hlock
-------------
Simply invoke the 'hlock' command and take a walk, 
there's no getting out of this one.
