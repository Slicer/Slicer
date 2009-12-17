
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

class CurveFittingToftsModelBiexponentialInput(CurveAnalysisBase):
    
    # ------------------------------
    # Constructor -- Set initial parameters
    def __init__(self):
        #self.ParameterNameList     = ['Ktrans', 've', 'Dose', 'delay']
        #self.InitialParameter      = [0.1, 0.1, 4.0, 0.0]
        #self.Constraints = [(0.0001, 0.7), (0.0001, 0.7), (1, 10), (1, 100)]

        #self.ParameterNameList     = ['Ktrans', 've', 'delay']
        #self.InitialParameter      = [0.1, 0.1, 0.0]
        #self.Constraints = [(0.0001, 0.7), (0.0001, 0.7), (1, 100)]

        self.ParameterNameList     = ['Ktrans', 've']
        self.InitialParameter      = [0.1, 0.1]
        self.Constraints = [(0.0001, 0.7), (0.0001, 0.7)]
        
        # The following default constant values ecept vp are from the paper: Tofts PS,
        # "Modeling Tracer Kinetics in Dynamic Dg-DTPA MR Imaging, JMRI, 1997; 7(1):91-101
        # Delay parameter is in seconds.
        self.ConstantNameList   = ['a1', 'a2', 'm1', 'm2', 'vp', 'delay', 'Dose']
        self.Constant           = [3.99, 4.78, 0.144, 0.0111, 0.03, 0.0, 0.0]

        #self.InputCurveNameList = ['AIF']
        self.FunctionVectorInput = 1

        self.MethodName          = 'Tofts Model, Biexponential Decay Plasma Concentration '
        self.MethodDescription   = ''
        
    # ------------------------------
    # Convert signal intensity curve to signal enhancement curve
    # Assuming parmagnetic contrast media (e.g. Gd-DTPA)
    def SignalToConcent(self, signal):
        if self.mrange > 0.0:
            self.signal0 = numpy.mean(signal[0:self.mrange])
        else:
            self.signal0 = signal[0]
            
        cont = signal / self.signal0 - 1.0
        #cont = signal - self.signal0
        #cont = signal
        return cont

    
    # ------------------------------
    # Convert signal enhancement curve to signal intensity curve
    def ConcentToSignal(self, concent):

        signal = (concent + 1.0) * self.signal0
        #signal = concent + self.signal0
        #sginal = concent
        return signal


    # ------------------------------
    # Set Constants
    def SetConstant(self, name, param):
        if name == 'a1':
            self.a1 = param
        if name == 'a2':
            self.a2 = param
        if name == 'm1':
            self.m1 = param
        if name == 'm2':
            self.m2 = param
        if name == 'vp':
            self.vp = param
        if name == 'delay':
            self.delay = param
            ## Note: TargetCurve has already been set 
            tarray = self.TargetCurve[:, 0]
            mrange = 0
            for t in tarray:
                if t < self.delay:
                    mrange = mrange + 1
                else:
                    break
            self.mrange = mrange
        if name == 'Dose':
            self.Dose = param

    # ------------------------------
    # Definition of the function
    def Function(self, x, param):
        #Ktrans, ve, D, delay = param
        #Ktrans, ve, delay = param
        Ktrans, ve  = param
        #Ktrans, kep = param
        
        a1 = self.a1
        a2 = self.a2
        m1 = self.m1
        m2 = self.m2
        vp = self.vp
        delay = self.delay
        kep = Ktrans / ve
        D  = self.Dose
        
        # Calculate shifted time and convert to minutes
        # if x < delay:  x_min = 0
        # else x_min = (x - delay) / 60.0
        x_min = (scipy.greater_equal(x, delay) * (x - delay)) / 60.0

        term1  = a1*( scipy.exp(-((kep)*x_min)) - scipy.exp(-(m1*x_min))) / (m1-(kep))
        term2  = a2*( scipy.exp(-((kep)*x_min)) - scipy.exp(-(m2*x_min))) / (m2-(kep))
        y = 0.0

        if vp > 0.0:
            term3  = a1*scipy.exp(-(m1*x_min))
            term4  = a2*scipy.exp(-(m2*x_min))
            y  = (D*Ktrans)*(term1+term2)+(vp*D)*(term3+term4)
        else:
            y  = (D*Ktrans)*(term1+term2)

        
        return y * scipy.greater_equal(x, delay)


    # ------------------------------
    # Calculate the output parameters (called by GetOutputParam())
    def CalcOutputParamDict(self, param):
        Ktrans, ve  = param
        #Ktrans, ve, D, delay = param
        #Ktrans, ve, delay = param

        dict = {}
        dict['Ktrans'] = Ktrans
        dict['ve']     = ve
        dict['kep']    = Ktrans / ve
        #dict['delay']  = delay
        
        return dict





