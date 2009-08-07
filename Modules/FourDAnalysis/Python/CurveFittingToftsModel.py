# ----------------------------------------------------------------------
#
#  Python Package for Curve Fitting in 3D Slicer 4D Analysis Module
#
#   Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH)
#   All Rights Reserved.
#
#   See Doc/copyright/copyright.txt
#   or http://www.slicer.org/copyright/copyright.txt for details.
#
#   Author: Junichi Tokuda (tokuda@bwh.harvard.edu)
#
#   For more detail, please refer:
#      http://wiki.na-mic.org/Wiki/index.php/Slicer3:FourDAnalysis
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# NOTE:
#   This python script requires SciPy package, which doesn't come with
#   Slicer3 package in default. Build 3D Slicer with USE_SCIPY option
#   (can be configured in slicer_variables.tcl) before run this script
#   from 3D Slicer.
# ----------------------------------------------------------------------

from FourDAnalysis import CurveAnalysisBase

import scipy, numpy
import sys

from scipy.integrate import quad
from scipy.integrate import fixed_quad
from scipy.integrate import quadrature
from scipy.interpolate import interp1d, splrep, splev
from numpy import r_

# ----------------------------------------------------------------------
# Tofts Model
# ----------------------------------------------------------------------

class CurveFittingToftsModel(CurveAnalysisBase):

    # ------------------------------
    # Constructor -- Set initial parameters
    def __init__(self):
        self.ParameterNameList  = ['Ktrans', 'vp', 've']
        self.InitialParameter   = [0.1, 0.1, 0.01] 
        #self.InitialOptimParam  = [0.015, 0.12, 0.12] 
        #self.InitialOptimParam  = [0.05, 0.2, 0.2] 
        self.InputCurveNameList = ['AIF']

        self.MethodName          = 'Tofts Model'
        self.MethodDescription   = '...'

        # dummy 
        self.AifTime = r_[0:5]
        self.AifData = r_[0:5]
        self.tck = splrep(self.AifTime, self.AifData, s=0)

    # ------------------------------
    # Convert signal intensity curve to concentration curve
    # Assuming parmagnetic contrast media (e.g. Gd-DTPA)
    def SignalToConcent(self, signal):
        cont = signal / signal[0] - 1.0
        return cont
    
    # ------------------------------
    # Convert concentration curve to signal intensity curve
    def ConcentToSignal(self, concent):
        signal = (concent + 1.0) * self.TargetCurve[0, 1]
        return signal
       
    # ------------------------------
    # Generate arteral input function from given data
    def SetInputCurve(self, name, curve):
        if name == 'AIF':
            self.AifTime = curve[:,0]
            self.AifData = self.SignalToConcent(curve[:,1])
            self.Tck = splrep(self.AifTime, self.AifData, s=0)
        
    # ------------------------------
    # Arteral input function (AIF)
    def Aif(self, x):
        y = splev(x, self.Tck, der=0)
        #y = self.AifTable[int(x/self.dt)]
        return y

    # ------------------------------
    # Definition of the function
    def Function(self, x, param):
        Ktrans, vp, ve = param
        lst = range(len(x))
        y = scipy.zeros(len(x))
        for i in lst:
            xx = x[i]
            s = quadrature(lambda t: self.Aif(t) * scipy.exp(-Ktrans*(xx-t)/ve), 0.0, xx, tol=1.0e-03, vec_func=False)
            #s = quadrature(lambda t: splev(t,self.Tck,der=0) * scipy.exp(-Ktrans*(xx-t)/ve), 0.0, xx, vec_func=False)
            y[i] = vp * self.Aif(xx) + Ktrans  * s[0]
        return y

    # ------------------------------
    # Calculate the output parameters (called by GetOutputParam())
    def CalcOutputParamDict(self, param):
        Ktrans, vp, ve = param

        dict = {}
        dict['Ktrans'] = Ktrans
        dict['vp']     = vp
        dict['ve']     = ve
        
        return dict





