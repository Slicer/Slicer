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

class CurveFittingToftsModelStepInput(CurveAnalysisBase):

    duration = 1.0

    # ------------------------------
    # Constructor -- Set initial parameters
    def __init__(self):
        self.ParameterNameList  = ['Ktrans', 've']
        self.InitialParameter   = [0.1, 0.1] 
        self.ConstantNameList = ['Duration', 'Delay', 'Cp0']
        self.Constant         = [1.0, 0.0, 1.0]
        self.FunctionVectorInput = 1

        self.MethodName          = 'Tofts Model with Step Input Function'
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
    # Set Constants
    def SetConstant(self, name, param):
        if name == 'Duration':
            self.duration = param;
        if name == 'Delay':
            self.delay = param;
        if name == 'Cp0':
            self.cp0 = param;
        
    # ------------------------------
    # Definition of the function
    def Function(self, t, param):
        Ktrans, ve = param
        duration   = self.duration
        delay      = self.delay
        cp0        = self.cp0


        # If step imput is assumed, the respons can be described as
        #
        #   y = (Ktrans * cp0 / kep) * (1-exp(-kep*t))         (t < duration)   ... (1)
        #   y = (Ktrans * cp0 / kep) * exp(-kep*(t-duration))  (t >= duration)  ... (2)

        #sys.stderr.write('t     : %s\n' % t )


        # Calculate shifted time
        t2 = scipy.greater_equal(t, delay) * (t - delay);

        # To describe C(t) in one equasion, we introduce t_dush, which is
        # defined by t_dash = t (t < duration) and t_dash = duration (t >= duration):
        t_dash = scipy.less(t2, duration)*t2 + scipy.greater_equal(t2, duration)*duration

        # Eq. (1) and (2) can be rewritten by:
        kep = Ktrans / ve
        y = (Ktrans*cp0/kep)*(scipy.exp(kep*t_dash) - 1) * scipy.exp(-kep*t2)

        return y

    # ------------------------------
    # Calculate the output parameters (called by GetOutputParam())
    def CalcOutputParamDict(self, param):
        Ktrans, ve = param

        dict = {}
        dict['Ktrans'] = Ktrans
        dict['ve']     = ve
        dict['kep']    = Ktrans / ve
        
        return dict





