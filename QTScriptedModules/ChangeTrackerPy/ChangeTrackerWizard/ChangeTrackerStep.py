from __main__ import qt, ctk

class ChangeTrackerStep( ctk.ctkWorkflowWidgetStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )

#    self.__mrmlManager = None
#    self.__logic = None
#    self.__workflow = None

  def setWorkflow( self, workflow ):
    '''
    '''
    self.__workflow = workflow

  def setParameterNode(self, parameterNode):
    '''
    Keep the pointer to the parameter node for each step
    '''
    self.__parameterNode = parameterNode

  '''
  def setMRMLManager( self, mrmlManager ):
    self.__mrmlManager = mrmlManager

  def setLogic( self, logic ):
    self.__logic = logic

  def mrmlManager( self ):
    return self.__mrmlManager

  def logic( self ):
    return self.__logic
  '''

  def workflow( self ):
    return self.__workflow

  def parameterNode(self):
    return self.__parameterNode

  def getBoldFont( self ):
    '''
    '''
    boldFont = qt.QFont( "Sans Serif", 12, qt.QFont.Bold )
    return boldFont

  def createUserInterface( self ):
    self.__layout = qt.QFormLayout( self )
    self.__layout.setVerticalSpacing( 5 )

    # add empty row
    self.__layout.addRow( "", qt.QWidget() )
    # add empty row
    self.__layout.addRow( "", qt.QWidget() )

    return self.__layout

  def onEntry( self, comingFrom, transitionType ):
    comingFromId = "None"
    if comingFrom: comingFromId = comingFrom.id()
    print "-> onEntry - current [%s] - comingFrom [%s]" % ( self.id(), comingFromId )
    super( ChangeTrackerStep, self ).onEntry( comingFrom, transitionType )

  def onExit( self, goingTo, transitionType ):
    goingToId = "None"
    if goingTo: goingToId = goingTo.id()
    print "-> onExit - current [%s] - goingTo [%s]" % ( self.id(), goingToId )
    super( ChangeTrackerStep, self ).onExit( goingTo, transitionType )

  def validate( self, desiredBranchId ):
    print "-> validate %s" % self.id()

  def validationSucceeded( self, desiredBranchId ):
    '''
    '''
    super( ChangeTrackerStep, self ).validate( True, desiredBranchId )

  def validationFailed( self, desiredBranchId, messageTitle='Error', messageText='There was an unknown error. See the console output for more details!' ):
    '''
    '''
    messageBox = qt.QMessageBox.warning( self, messageTitle, messageText )
    super( ChangeTrackerStep, self ).validate( False, desiredBranchId )

  '''
  def updateWidgetFromParameters(self, parameterNode):
  '''
