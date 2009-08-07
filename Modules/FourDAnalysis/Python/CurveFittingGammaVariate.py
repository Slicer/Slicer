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
from scipy.integrate import quad
import sys

# ----------------------------------------------------------------------
# Gamma Variate Function fitting class
# ----------------------------------------------------------------------

class CurveFittingGammaVariate(CurveAnalysisBase):

    # ------------------------------
    # Constructor -- Set initial parameters
    def __init__(self):
        self.ParameterNameList  = ['Sp', 'alpha', 'beta', 'Ta', 'S0']
        self.InitialParameter   = [200.0, 3.0,    1.0,    0.0,  20.0] 

        self.MethodName          = 'Gamma Variate Function fitting'
        self.MethodDescription   = '...'

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
    # Definition of the function
    def Function(self, x, param):
        Sp, alpha, beta, Ta, S0 = param
        y = Sp * numpy.abs(scipy.power((scipy.e / (alpha*beta)), alpha)) * numpy.abs(scipy.power((x-Ta), alpha)) * scipy.exp(-(x-Ta)/beta) + S0
        return y
    
    # ------------------------------
    # Calculate the output parameters (called by GetOutputParam())
    def CalcOutputParam(self, param):
        Sp, alpha, beta, Ta, S0 = param

        sts = quad(lambda x: x*(self.Function(x, param) - S0), 0.0, 100.0)
        ss  = quad(lambda x: self.Function(x, param) - S0, 0.0, 100.0)
        if ss <> 0.0:
            MTT = sts[0] / ss[0]
        else:
            MTT = 0.0

        dict = {}
        dict['MTT']   = MTT
        dict['Sp']    = Sp

        #dict['alpha'] = alpha
        #dict['beta']  = beta
        #dict['Ta']    = Ta
        #dict['S0']    = S0

        return dict


