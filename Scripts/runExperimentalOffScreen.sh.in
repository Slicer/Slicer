# pick a non typical DISPLAY port.
setenv DISPLAY :1980.0

# poor mans killall. In case Xvfb is still running.
ps -ef | grep Xvfb | grep -v grep | awk '{print $2}' | xargs kill

Xvfb :1980 -screen 0 1600x1200x24 &

# call your dashboard
make Experimental

# poor mans killall
ps -ef | grep Xvfb | grep -v grep | awk '{print $2}' | xargs kill

