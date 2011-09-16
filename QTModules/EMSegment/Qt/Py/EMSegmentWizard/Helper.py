# slicer imports
from __main__ import vtk, slicer

# python includes
import sys
import time

class Helper( object ):
  '''
  classdocs
  '''

  @staticmethod
  def Info( message ):
    '''
    
    '''

    print "[EMSegmentPy " + time.strftime( "%m/%d/%Y %H:%M:%S" ) + "]: " + str( message )
    sys.stdout.flush()

  @staticmethod
  def Warning( message ):
    '''
    
    '''

    print "[EMSegmentPy " + time.strftime( "%m/%d/%Y %H:%M:%S" ) + "]: WARNING: " + str( message )
    sys.stdout.flush()

  @staticmethod
  def Error( message ):
    '''
    
    '''

    print "[EMSegmentPy " + time.strftime( "%m/%d/%Y %H:%M:%S" ) + "]: ERROR: " + str( message )
    sys.stdout.flush()


  @staticmethod
  def Debug( message ):
    '''
    
    '''

    showDebugOutput = 0
    from time import strftime
    if showDebugOutput:
        print "[EMSegmentPy " + time.strftime( "%m/%d/%Y %H:%M:%S" ) + "] DEBUG: " + str( message )
        sys.stdout.flush()

  @staticmethod
  def CreateSpace( n ):
    '''
    '''
    spacer = ""
    for s in range( n ):
      spacer += " "

    return spacer


  @staticmethod
  def GetNthStepId( n ):
    '''
    '''
    steps = [None, # 0
             'SelectTask', # 1
             'DefineInputChannels', # 2
             'DefineAnatomicalTree', # 3
             'DefineAtlas', # 4
             'EditRegistrationParameters', # 5
             'DefinePreprocessing', # 6
             'SpecifyIntensityDistribution', # 7
             'EditNodeBasedParameters', # 8
             'DefineMiscParameters', # 9
             'Segment', # 10
             'Statistics'] # 11

    if n < 0 or n > len( steps ):
      n = 0

    return steps[n]

  @staticmethod
  def GetRegistrationTypes():
    '''
    '''
    return ['None', 'Fast', 'Accurate']

  @staticmethod
  def GetInterpolationTypes():
    '''
    '''
    return ['Linear', 'Nearest Neighbor', 'Cubic' ]

  @staticmethod
  def GetPackages():
    '''
    '''
    return ['CMTK', 'BRAINS', 'PLASTIMATCH', 'DEMONS', 'DRAMMS', 'ANTS']

  @staticmethod
  def GetSpecificationTypes():
    '''
    '''
    return ['Manual', 'Manual Sampling']#, 'Auto Sampling']

  @staticmethod
  def GetStoppingConditionTypes():
    '''
    '''
    return ['Iterations', 'LabelMap', 'Weights']

  @staticmethod
  def onClickInSliceView( sliceViewInteractorStyle, sliceLogic ):
    '''
    Get the last event of a certain sliceView and return it as RAS coordinates.
    '''
    coordinates = []

    i = sliceViewInteractorStyle.GetInteractor()
    if i:

      clickedPos = i.GetLastEventPosition()

      # we need a 4 element point to be able to multiply further down
      coordinates.append( clickedPos[0] )
      coordinates.append( clickedPos[1] )
      coordinates.append( 0 )
      coordinates.append( 1 )

      xyToRas = sliceLogic.GetSliceNode().GetXYToRAS()

      rasPos = xyToRas.MultiplyPoint( coordinates )

      return [rasPos[0], rasPos[1], rasPos[2]]


