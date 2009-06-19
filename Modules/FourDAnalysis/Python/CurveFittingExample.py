# ----------------------------------------------------------------------
#
#  Example Script of Curve Fitting for 3D Slicer Four D Analysis Module
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

import scipy, scipy.optimize, numpy
from scipy.integrate import quad


# ----------------------------------------------------------------------
# Equation to Fit (gamma variate function)
# ----------------------------------------------------------------------
def gvf(x, Sp, alpha, beta, Ta, S0):
    global scipy, numpy
    y = Sp * numpy.abs(scipy.power((scipy.e / (alpha*beta)), alpha)) * numpy.abs(scipy.power((x-Ta), alpha)) * scipy.exp(-(x-Ta)/beta) + S0
    return(y)


# ----------------------------------------------------------------------
# Calculate error (the optimizer will try minimizing this function)
# ----------------------------------------------------------------------
def residuals_with_error(param_fit, y, x):
    global gvf
    Sp, alpha, beta, Ta, S0 = param_fit
    err = y - (gvf(x, Sp, alpha, beta, Ta, S0))
    return(err)


# ----------------------------------------------------------------------
# Main routine
# ----------------------------------------------------------------------

def main(execdict):
    global gvf, residuals_with_error, quad

    # --------------------
    # Input Array
    srcCurve = execdict['InputCurve']
    x        = srcCurve[:, 0]  # Time
    y_meas   = srcCurve[:, 1]  # Intensity value
    
    # ------------------------------
    # Set initial parameter
    # Sp, alpha, beta, Ta, S0
    #param_init = [400.0, 3.0, 1.0, 0.0, 20.0]
    param_init = [200.0, 3.0, 1.0, 0.0, 20.0]
    
    # ------------------------------
    # Perform optimization
    param_output = scipy.optimize.leastsq(residuals_with_error, param_init, args=(y_meas, x),full_output=True)
    param_result = param_output[0] # fitted parameters
    covar_result = param_output[1] # covariant matrix
    
    
    # ------------------------------
    # Get result parameters
    Sp, alpha, beta, Ta, S0 = param_result
    
    # ------------------------------
    # Print out parameters
    sys.stderr.write(' Sp     : %f\n' % Sp )
    sys.stderr.write(' alpha  : %f\n' % alpha )
    sys.stderr.write(' beta   : %f\n' % beta )
    sys.stderr.write(' Ta     : %f\n' % Ta )
    sys.stderr.write(' S0     : %f\n' % S0 )
    
    # ------------------------------
    # Generate fitted curve
    fittedCurve = execdict['FittedCurve']
    x1 = fittedCurve[:, 0]
    y  = gvf(x1, Sp, alpha, beta, Ta, S0)
    fittedCurve[:, 1] = y
    
    # ------------------------------
    # Calculate MTT
    range = [0.0, 100.0]
    sts = quad(lambda x: x*(gvf(x, Sp, alpha, beta, Ta, S0) - S0), range[0], range[1])
    ss  = quad(lambda x: gvf(x, Sp, alpha, beta, Ta, S0) - S0, range[0], range[1])
    MTT = sts[0] / ss[0]
    
    sys.stderr.write(' MTT     : %f\n' % MTT )
    
    # ------------------------------
    # Output parameters into parameter dictionary
    paramDict = execdict['ParameterDictionary']
    paramDict['MTT'] = MTT



# ----------------------------------------------------------------------
# Execute main()
# ----------------------------------------------------------------------

main(locals())

