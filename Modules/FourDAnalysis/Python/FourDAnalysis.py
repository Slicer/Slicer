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

import re
import sys
import imp
import scipy.optimize 
import numpy
import copy
#from mpfit import mpfit

import time

# ----------------------------------------------------------------------
# Base class for curve fitting algorithm classes
# ----------------------------------------------------------------------



class CurveAnalysisBase(object):

    # Parameters to optimze
    ParameterNameList     = []
    InitialParameter      = []
    Constraints           = None
    
    Parameter             = []

    # Input curve
    InputCurveNameList    = []

    # constants
    ConstantNameList    = []
    Constant            = []

    # Target curve
    TargetCurve           = []

    CovarianceMatrix      = []

    FunctionVectorInput   = 0

    MethodName            = ''
    MethodDescription     = ''

    #def __init__(self):
        ## ParameterNameList and Initial Param should be set here
        
    # ------------------------------
    # Get method name

    def GetMethodName(self):
        return self.MethodName

    # ------------------------------
    # Get method description

    def GetMethodDescription(self):
        return self.MethodDescription


    def Function(self, x, param):
        return 0


    def CalcOutputParamDict(self, param):
        return {}

    ## ------------------------------
    #  Generic residual error function.
    #  This function calls Function() defined in the child class.
    #
    #  Two different optimization packages have been tested.
    #
    # ------------------------------
    # Generic residual error fucntion for scipy.optimize package
    
    def ResidualError(self, param, y, x):
        lst = range(len(x))
        f = self.REBUF
        for i in lst:
            f[i] = self.Function(x[i], param)
    
        err = (y - f)
        #err2 = sum(err*err)
        return err
    
    # ------------------------------
    # Residual error for functions that accept vector for scipy.optimize package
    
    def ResidualErrorVec(self, param, y, x):
        err = y - self.Function(x, param)
    
        #err2 = sum(err*err)
        return err

    ## ------------------------------
    ## Residual error for functions that accept vector
    #
    #def ResidualError(self, p, fjac=None, x=None, y=None, err=None):
    #    err = y - self.Function(x, p)
    #    status = 0
    #    return ([status, err])


    # ------------------------------
    # Convert signal intensity curve to concentration curve
    def SignalToConcent(self, signal):
        return signal

    # ------------------------------
    # Convert concentration curve to signal intensity curve
    # (used to generate a fit curve)
    def ConcentToSignal(self, concent):
        return concent

    # ------------------------------
    # Parameters to optimze

    def GetParameterNameList(self):
        return self.ParameterNameList

    def SetInitialParameter(self, param):
        self.InitialParameter = param

    def GetInitialParameters(self):
        return self.InitialParameter

    def GetParameter(self):
        return self.Parameter

    # ------------------------------
    # Constraints
    def SetConstraints(self, constraints):
        self.Constraints = constraints

    # ------------------------------
    # Constants

    def GetConstantNameList(self):
        return self.ConstantNameList

    def GetConstants(self):
        return self.Constant

    def SetConstant(self, name, param):
        return 0

    # ------------------------------
    # Output parameters

    def GetOutputParamNameList(self):
        dict = self.CalcOutputParamDict(self.InitialParameter)
        list = []
        for key, value in dict.iteritems():
            list.append(key)
        return list

    def GetOutputParam(self):
        return self.CalcOutputParamDict(self.Parameter)

    # ------------------------------
    # Input curve

    def GetInputCurveNameList(self):
        return self.InputCurveNameList

    def SetInputCurve(self, name, curve):
        return 0
    
    # ------------------------------
    # Target curve

    def SetTargetCurve(self, targetCurve):
        self.TargetCurve = targetCurve

    # ------------------------------
    # Fit curve

    def GetFitCurve(self, x):
        if self.FunctionVectorInput == 0:
            lst = range(len(x))
            f = x.copy()
            for i in lst:
                f[i] = self.Function(x[i], self.Parameter)
            return self.ConcentToSignal(f)
        else:
            return self.ConcentToSignal(self.Function(x, self.Parameter))


    # ------------------------------
    # Initialize parameters (called just before optimization)

    def Initialize(self):
        return 0

    # ------------------------------
    # Execute optimization

    def Execute(self):
        
        self.Initialize()

        x      = self.TargetCurve[:, 0]
        y_meas = self.SignalToConcent(self.TargetCurve[:, 1])

        param0 = self.InitialParameter

        #######
        ## following code uses mpfit

        #p0 = numpy.array(self.InitialParameter,dtype='float64')
        #parinfo = []
        #diag = []
        #parbase={'value':0., 'fixed':0, 'limited':[0,0], 'limits':[0.,0.]}
        #for i in range(0, len(param0)):
        #    parinfo.append(copy.deepcopy(parbase))
        #    parinfo[i]['value'] = p0[i]
        #    if len(self.Constraints) > 0:
        #        parinfo[i]['limited'] = [1, 1]
        #        parinfo[i]['limits'] = [self.Constraints[i][0], self.Constraints[i][1]]
        #        #diag = (self.Constraint[i][1] - self.Constraint[i][0])*10
        #
        #fa = {'x':x, 'y':y_meas}
        ##output = mpfit(self.ResidualError, p0, parinfo=parinfo,functkw=fa,nprint=0,ftol=1.e-8,xtol=1.e-5,diag=diag)
        #output = mpfit(self.ResidualError,p0, parinfo=parinfo,functkw=fa, nprint=1,ftol=1.e-5,xtol=1.e-5,diag=diag)
        #self.Parameter = output.params

        #######
        ## following code uses scipy.optimize package
        #
        #bound = self.Constraint
        if self.FunctionVectorInput == 0:
            self.REBUF  = scipy.zeros(len(x))   # to reduce number of memory allocations
            param_output = scipy.optimize.leastsq(self.ResidualError, param0, args=(y_meas, x),full_output=False,ftol=1e-04,xtol=1.49012e-04)
            #param_output = scipy.optimize.fmin_cobyla(self.ResidualError, param0, cons=bound, args=(y_meas, x), maxfun=10000)
            #param_output = scipy.optimize.fmin_tnc(self.ResidualError, param0, args=(y_meas, x), bounds=bound2, epsilon=1e-08, scale=[0.1, 0.1, 1], offset=None, messages=15, maxCGit=-1, maxfun=None, eta=-1, stepmx=0, accuracy=0, fmin=0, ftol=-1, xtol=-1, pgtol=-1, rescale=-1)
        
        else:
            param_output = scipy.optimize.leastsq(self.ResidualErrorVec, param0, args=(y_meas, x),full_output=False,ftol=1e-04,xtol=1.49012e-04)
            #param_output = scipy.optimize.fmin_cobyla(self.ResidualErrorVec, param0, cons=bound, args=(y_meas, x), maxfun=1000)
            #param_output = scipy.optimize.fmin_tnc(self.ResidualErrorVec, param0, args=(y_meas, x), approx_grad=True, bounds=bound, epsilon=1e-04, scale=[1, 1, 10], offset=None, messages=0, maxCGit=-1, maxfun=None, eta=-1, stepmx=0, accuracy=0, fmin=0, ftol=-1, xtol=-1, pgtol=-1, rescale=-1)
            #param_output = scipy.optimize.fmin_l_bfgs_b(self.ResidualErrorVec, param0, args=(y_meas, x), approx_grad=True, bounds=bound, m=10, factr=10000000.0, pgtol=1.0000000000000001e-05, epsilon=1e-08, iprint=0, maxfun=15000)
            
        self.Parameter        = param_output[0] # fitted parameters
        self.CovarianceMatrix = param_output[1] # covariant matrix
        #self.Parameter       = param_output # fitted parameters

        return 1        ## should return 0 if optimization fails


# ----------------------------------------------------------------------
# Class to execute curve fitting
# ----------------------------------------------------------------------

class CurveAnalysisExecuter(object):

    ModuleName = ''
    DebugMode = 0
    Constraints = {}

    # ------------------------------
    # Constructor
    def __init__(self, modulePath):
        # ------------------------------
        # Extract directory path, file name and module name
        
        direxp = re.compile('([^\/]+)$')
        extexp = re.compile('(\.py)$')
        
        directory = direxp.sub('', modulePath)
        fileNameMatch = direxp.search(modulePath)
        fileName = fileNameMatch.groups()[0]
        self.ModuleName = extexp.sub('', fileName)

        #sys.stderr.write('Directory     : %s\n' % directory )
        #sys.stderr.write('File name     : %s\n' % fileName )

        # ------------------------------
        # Add the search path if it hasn't been registered.
        try:
            id = sys.path.search(directory)
        except:
            sys.path.append(directory)

        # ------------------------------
        # Load / reload the module.

        fp, pathname, description = imp.find_module(self.ModuleName)
        try:
            self.Module = imp.load_module(self.ModuleName, fp, pathname, description)
        finally:
            if fp:
                fp.close()

        self.Constraints = {}


    # ------------------------------
    # Get Method Name
    def GetMethodName(self):
        exec('fitting = self.Module.' + self.ModuleName + '()')
        return fitting.GetMethodName()

    # ------------------------------
    # Get Method Description
    def GetMethodDescription(self):
        exec('fitting = self.Module.' + self.ModuleName + '()')
        return fitting.GetMethodDescription()

    # ------------------------------
    # Get Input Curve Name List
    def GetInputCurveNames(self):
        exec('fitting = self.Module.' + self.ModuleName + '()')
        list =  fitting.GetInputCurveNameList()
        return list

    # ------------------------------
    # Get Initial Optimization Parameter Dictionary
    def GetInitialParameters(self):
        exec('fitting = self.Module.' + self.ModuleName + '()')
        names  = fitting.GetParameterNameList()
        values = fitting.GetInitialParameters()
        n = len(names)
        params = {}
        for i in range(n):
            params[names[i]] = values[i]
        return params

    # ------------------------------
    # Get Constants
    def GetConstants(self):
        exec('fitting = self.Module.' + self.ModuleName + '()')
        names  = fitting.GetConstantNameList()
        values = fitting.GetConstants()
        n = len(names)
        params = {}
        for i in range(n):
            params[names[i]] = values[i]
        return params

    # ------------------------------
    # Get Constant Name List
    def GetConstantNames(self):
        exec('fitting = self.Module.' + self.ModuleName + '()')
        list =  fitting.GetConstantNameList()
        return list

    # ------------------------------
    # Get Output Parameter Name List
    def GetOutputParameterNames(self):
        exec('fitting = self.Module.' + self.ModuleName + '()')
        list =  fitting.GetOutputParamNameList()
        return list

    # ------------------------------
    # Set Constraint
    # 'constraintDict' should be a dictionary which associates tuple of
    # minimum and maximum values with parameter name:
    #      constraintDict['param name'] = (minimum, muximum)
    #
    def SetConstraints(self, constraintDict):
        ## Todo: check the type of the argument
        self.Constraints = constraintDict

    # ------------------------------
    # Call curve fitting class
    def Execute(self, inputCurveDict, initialParameterDict, inputParameterDict, targetCurve, outputCurve):

        exec('fitting = self.Module.' + self.ModuleName + '()')

        # ------------------------------
        # Set Curves
        for name, curve in inputCurveDict.iteritems():
            fitting.SetInputCurve(name, curve)

        fitting.SetTargetCurve(targetCurve)

        # ------------------------------
        # Set initial optimization parameters
        nameList = fitting.GetParameterNameList()
        n = len(nameList)
        paramList = numpy.zeros(n)
        for i in range(n):
            paramList[i] = initialParameterDict[nameList[i]]

        fitting.SetInitialParameter(paramList)

        # ------------------------------
        # Set constants
        for name, param, in inputParameterDict.iteritems():
            fitting.SetConstant(name, param)

        # ------------------------------
        # Set constraints, if specified
        # Constraints dictionary is converted to a list here.
        constraints = []
        if len(self.Constraints) > 0:
            for pname in nameList:
                constraints.append(self.Constraints[pname])

        fitting.SetConstraints(constraints)

        # ------------------------------
        # Run optimization
        fitting.Execute()
        outputCurve[:,1] = fitting.GetFitCurve(outputCurve[:, 0])
        result = fitting.GetOutputParam()

        return result


    # ------------------------------
    # Call curve fitting class / for mulitprocess
    def ExecuteWithQueue(self, q):

        dict = q.get()
        inputCurveDict       = dict['inputCurveDict']
        initialParameterDict = dict['initialParameterDict']
        inputParameterDict   = dict['inputParameterDict']
        targetCurve          = dict['targetCurve']
        outputCurve          = dict['outputCurve']

        exec('fitting = self.Module.' + self.ModuleName + '()')

        # ------------------------------
        # Set Curves
        for name, curve in inputCurveDict.iteritems():
            fitting.SetInputCurve(name, curve)

        fitting.SetTargetCurve(targetCurve)

        # ------------------------------
        # Set initial optimization parameters
        nameList = fitting.GetParameterNameList()
        n = len(nameList)
        paramList = numpy.zeros(n)
        for i in range(n):
            paramList[i] = initialParameterDict[nameList[i]]

        fitting.SetInitialParameter(paramList)

        # ------------------------------
        # Set constants
        for name, param, in inputParameterDict.iteritems():
            fitting.SetConstant(name, param)

        # ------------------------------
        # Run optimization

        fitting.Execute()
        outputCurve[:,1] = fitting.GetFitCurve(outputCurve[:, 0])
        
        rdict = {}
        rdict['result']      = fitting.GetOutputParam()
        rdict['outputCurve'] = outputCurve 

        q.put(rdict)
