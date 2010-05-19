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
        self.ParameterNameList  = ['Sp', 'alpha', 'beta', 'Ta']
        self.InitialParameter   = [200.0, 3.0,    1.0,    0.0] 

        self.ConstantNameList   = ['TpcMax']
        self.Constant           = [0.0]

        self.FunctionVectorInput = 1

        self.MethodName          = 'Gamma Variate Function fitting'
        self.MethodDescription   = '...'

        self.TargetCurve         = numpy.array([[0, 0]])

        self.TpcMax              = 0.0

    # ------------------------------
    # Convert signal intensity curve to concentration curve
    # Assuming parmagnetic contrast media (e.g. Gd-DTPA)
    def SignalToConcent(self, signal):
        cont = signal
        return cont

    # ------------------------------
    # Convert concentration curve to signal intensity curve
    def ConcentToSignal(self, concent):
        signal = concent
        return signal
       
    # ------------------------------
    # Set Constants
    def SetConstant(self, name, param):
        if name == 'TpcMax':
            # precontrast duration
            self.TpcMax = param
        return

    def CalcS0(self, Ta):
        tarray = self.TargetCurve[:, 0]
        signal = self.TargetCurve[:, 1]
        S0 = 0.0

        Ta1 = 0.0
        if self.TpcMax > 0.0:
            Ta1 = self.TpcMax
        else:
            Ta1 = Ta
        
        ### following code assumes that the time interval is constant
        #mrange = numpy.floor(Ta / (tarray[1]-tarray[0]))

        ### following code can handle variable interval
        mrange = 0
        for t in tarray:
            if t < Ta1:
                mrange = mrange + 1
            else:
                break
        if mrange > 0.0:
            S0 = numpy.mean(signal[0:mrange])
        else:
            S0 = signal[0]
        #print mrange
        #print signal
        return S0

    # ------------------------------
    # Definition of the function
    def Function(self, x, param):
        Sp, alpha, beta, Ta  = param
        S0 = self.CalcS0(Ta)
        x2 = (scipy.greater_equal(x, Ta) * (x - Ta))
        #y  = Sp * numpy.abs(scipy.power((scipy.e / (alpha*beta)), alpha)) * numpy.abs(scipy.power(x2, alpha)) * scipy.exp(-x2/beta) + S0
        y  = Sp * numpy.abs(scipy.power((scipy.e / (alpha*beta)), alpha) * scipy.power(x2, alpha) * scipy.exp(-x2/beta)) + S0
        return y

    # ------------------------------
    # Calculate the output parameters (called by GetOutputParam())
    def DiffFunction(self, x, param):
        Sp, alpha, beta, Ta  = param
        if x < Ta:
            return 0
        x2 = x - Ta
        y = Sp * scipy.power((scipy.e / (alpha*beta)), alpha) * (alpha * scipy.power(x2, alpha - 1) * scipy.exp(-x2/beta) - scipy.power(x2, alpha) * scipy.exp(-x2/beta) / beta)
        return y
        
    
    # ------------------------------
    # Calculate the output parameters (called by GetOutputParam())
    def CalcOutputParamDict(self, param):
        Sp, alpha, beta, Ta = param
        S0 = self.CalcS0(Ta)

        sts = quad(lambda x: x*(self.Function(x, param) - S0), 0.0, 1000.0)
        ss  = quad(lambda x: self.Function(x, param) - S0, 0.0, 1000.0)
        if ss[0] <> 0.0:
            MTT = sts[0] / ss[0]
        else:
            MTT = 0.0

        dict = {}
        dict['MTT']   = MTT
        dict['Sp']    = Sp
        dict['alpha'] = alpha
        dict['beta']  = beta
        dict['Ta']    = Ta
        dict['S0']    = S0
        dict['TTP']   = alpha*beta + Ta
        dict['S240']    = self.Function(240.0+Ta, param)
        dict['S480']    = self.Function(480.0+Ta, param)

        # Calculate initial slope defined by the sloat at t, where
        #    f''(t) = 0
        
        # point of contact
        pcx = beta * (alpha - scipy.sqrt(alpha)) + Ta
        if pcx < Ta:
            pcx = Ta
        pcy = self.Function(pcx, param)
        dict['MaxSlope']  = self.DiffFunction(pcx, param)
        dict['TMaxSlope'] = pcx

        # Calculate standard deviation of residual error
        if S0 == 0.0:
            dict['SDRE'] = 0.0
        else:
            x = self.TargetCurve[:, 0]
            y = self.SignalToConcent(self.TargetCurve[:, 1])
            err = 0.0
            if self.FunctionVectorInput == 0:
                err = self.ResidualError(param, y, x) / S0
            else:
                err = self.ResidualErrorVec(param, y, x) / S0
            dict['SDRE'] = numpy.std(err)

        return dict


