#! /opt/local/bin/python2.5

import scipy.io
import matplotlib.pyplot as plt

da0 = scipy.io.read_array('curve_001.csv', separator=',')
da1 = scipy.io.read_array('curve_003.csv', separator=',')
plt.plot(da1)
