from __main__ import qt, ctk

from EMSegmentStep import *
from Helper import *
import os

class EMSegmentSelectTaskStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '1. Define Task' )
    self.setDescription( 'Select a (new) task.' )

    self.__tasksList = dict()
    self.__preprocessingTasksList = dict()
    self.__newTaskString = 'Create new task..'
    self.__parent = super( EMSegmentSelectTaskStep, self )

  def createUserInterface( self ):
    '''
    '''

    self.buttonBoxHints = self.ButtonBoxHidden

    self.__layout = self.__parent.createUserInterface()

    # let's load all tasks
    self.loadTasks()
    self.loadPreprocessingTasks()

    eminfoLabel = qt.QLabel( 'This module provides EM segmentation based on an atlas.\nBy defining an anatomical tree, different structures can be segmented.\n\n' )
    eminfoLabel.setFont( self.__parent.getBoldFont() )
    self.__layout.addRow( eminfoLabel )

    selectTaskLabel = qt.QLabel( 'Select Task' )
    selectTaskLabel.setFont( self.__parent.getBoldFont() )
    self.__layout.addRow( selectTaskLabel )

    self.__taskComboBox = qt.QComboBox()
    self.__taskComboBox.toolTip = "Choose a task."

    # fill the comboBox with the taskNames
    self.__taskComboBox.addItems( self.getTaskNames() )
    self.__taskComboBox.setCurrentIndex( self.__taskComboBox.findText( 'MRI Human Brain' ) )
    self.__taskComboBox.connect( 'currentIndexChanged(int)', self.onTaskSelected )
    self.__layout.addRow( Helper.CreateSpace( 20 ), self.__taskComboBox )

    infoLabel = qt.QLabel( 'For more information about the tasks click here:' )
    self.__layout.addRow( Helper.CreateSpace( 20 ), infoLabel )

    urlLabel = qt.QLabel( '<a href=http://www.slicer.org/slicerWiki/index.php/EMSegmenter-Tasks>http://www.slicer.org/slicerWiki/index.php/EMSegmenter-Tasks</a>' )
    urlLabel.setOpenExternalLinks( True )
    self.__layout.addRow( Helper.CreateSpace( 20 ), urlLabel )

    # add empty row
    self.__layout.addRow( "", qt.QWidget() )
    # add empty row
    self.__layout.addRow( "", qt.QWidget() )
    # add empty row
    self.__layout.addRow( "", qt.QWidget() )
    # add empty row
    self.__layout.addRow( "", qt.QWidget() )

    chooseModeLabel = qt.QLabel( 'Choose Mode' )
    chooseModeLabel.setFont( self.__parent.getBoldFont() )
    self.__layout.addRow( chooseModeLabel )

    self.__buttonBox = qt.QDialogButtonBox()
    simpleButton = self.__buttonBox.addButton( self.__buttonBox.Discard )
    simpleButton.setIcon( qt.QIcon() )
    simpleButton.text = "Simple"
    simpleButton.toolTip = "Click to use the simple mode."
    advancedButton = self.__buttonBox.addButton( self.__buttonBox.Apply )
    advancedButton.setIcon( qt.QIcon() )
    advancedButton.text = "Advanced"
    advancedButton.toolTip = "Click to use the advanced mode."
    self.__layout.addWidget( self.__buttonBox )

    # connect the simple and advanced buttons
    simpleButton.connect( 'clicked()', self.goSimple )
    advancedButton.connect( 'clicked()', self.goAdvanced )

  def onTaskSelected( self ):
    '''
    '''
    index = self.__taskComboBox.currentIndex
    taskName = self.__taskComboBox.currentText

    # re-enable the simple and advanced buttons
    self.__buttonBox.enabled = True

    if taskName == self.__newTaskString:
      # create new task was selected

      # disable the simple and advanced buttons
      self.__buttonBox.enabled = False

      # create new dialog
      self.__d = qt.QDialog()
      dLayout = qt.QFormLayout( self.__d )

      self.__nameEdit = qt.QLineEdit()
      dLayout.addRow( 'New Task Name:', self.__nameEdit )

      self.__preprocessingComboBox = qt.QComboBox()
      list = self.__preprocessingTasksList.keys()
      list.sort( lambda x, y: cmp( x.lower(), y.lower() ) )
      self.__preprocessingComboBox.addItems( list )
      # also, add None
      self.__preprocessingComboBox.addItem( 'None' )
      dLayout.addRow( 'Pre-processing:', self.__preprocessingComboBox )

      buttonBox = qt.QDialogButtonBox()
      #cancelButton = buttonBox.addButton(buttonBox.Discard)
      #cancelButton.text = 'Cancel'
      okButton = buttonBox.addButton( buttonBox.Apply )
      okButton.setIcon( qt.QIcon() )
      okButton.text = 'Apply'
      okButton.connect( 'clicked()', self.createNewTask )
      dLayout.addWidget( buttonBox )

      self.__d.setModal( True )
      self.__d.show()

  def createNewTask( self ):
    '''
    '''
    name = self.__nameEdit.text
    preprocessing = self.__preprocessingComboBox.currentText
    if preprocessing == 'None':
      taskFile = None
    else:
      taskFile = self.__preprocessingTasksList[preprocessing]

    if not name:
      messageBox = qt.QMessageBox.warning( self, 'Error', 'The name can not be empty!' )
      return

    if not taskFile:
      # use default taskfile
      taskFileShort = slicer.vtkMRMLEMSGlobalParametersNode.GetDefaultTaskTclFileName()
      taskFile = taskFileShort
    else:
      # get just the filename
      taskFileShort = os.path.split( taskFile )[1]

    self.mrmlManager().CreateAndObserveNewParameterSet()
    templateNodes = slicer.mrmlScene.GetNodesByClass( 'vtkMRMLEMSTemplateNode' )

    self.mrmlManager().SetNthParameterName( templateNodes.GetNumberOfItems() - 1, name )
    self.mrmlManager().SetTclTaskFilename( taskFileShort )

    # update combobox
    self.__taskComboBox.clear()
    self.loadTasks()
    self.__taskComboBox.addItems( self.getTaskNames() )

    # select new task
    self.__taskComboBox.setCurrentIndex( self.__taskComboBox.findText( name ) )

    self.__d.hide()

    self.goAdvanced()

  def loadTasks( self ):
    '''
    Load all available Tasks and save them to self.__tasksList as key,value pairs of taskName and fileName
    '''
    if not self.logic():
      Helper.Error( "No logic class!" )
      return False

    # we query the logic for a comma-separated string with the following format of each item:
    # tasksName:tasksFile
    tasksList = self.logic().GetTasks().split( ',' )

    self.__tasksList.clear()

    templateNodes = slicer.mrmlScene.GetNodesByClass( 'vtkMRMLEMSTemplateNode' )

    # look in the scene
    for i in range( templateNodes.GetNumberOfItems() ):

      t = templateNodes.GetItemAsObject( i )
      # there are already nodes in the scene, let's propagate them to our list
      taskName = t.GetName()
      taskFile = slicer.vtkMRMLEMSGlobalParametersNode.GetDefaultTaskTclFileName()

      self.__tasksList[taskName] = taskFile

    # look in files
    for t in tasksList:
      task = t.split( ':' )
      if len( task ) == 2:
        taskName = task[0]
        taskFile = task[1]

      # add this entry to our tasksList, if it does not exist yet
      if not self.__tasksList.has_key( taskName ):
        self.__tasksList[taskName] = taskFile

    return True

  def loadPreprocessingTasks( self ):
    '''
    Load all available Tasks and save them to self.__tasksList as key,value pairs of taskName and fileName
    '''
    if not self.logic():
      Helper.Error( "No logic class!" )
      return False

    # we query the logic for a comma-separated string with the following format of each item:
    # tasksName:tasksFile
    tasksList = self.logic().GetPreprocessingTasks().split( ',' )

    self.__preprocessingTasksList.clear()

    for t in tasksList:
      if t:
        task = t.split( ':' )
        if len( task ) == 2:
          taskName = task[0]
          taskFile = task[1]

      # add this entry to our tasksList
      self.__preprocessingTasksList[taskName] = taskFile

    return True

  def loadTask( self ):
    '''
    '''
    index = self.__taskComboBox.currentIndex

    taskName = self.__taskComboBox.currentText
    taskFile = self.__tasksList[taskName]

    if not taskName or not taskFile:
      # error!
      Helper.Error( "Either taskName or taskFile was empty!" )
      return False

    # now get any loaded EMSTemplateNode which could fit our name
    templateNodesPreLoad = slicer.mrmlScene.GetNodesByClassByName( 'vtkMRMLEMSTemplateNode', taskName )
    if templateNodesPreLoad.GetNumberOfItems() > 0:
      # this is strange behavior but we can continue in this case!
      Helper.Warning( "We already have the template node in the scene and do not load it again!" )

    else:

      # there was no relevant template node in the scene, so let's import the mrml file
      # this is the normal behavior!      
      Helper.Info( "Attempting to load task '" + taskName + "' from file '" + taskFile + "'" )

      # only load if no relevant node exists
      self.mrmlManager().ImportMRMLFile( taskFile )


    # now get the loaded EMSTemplateNode
    templateNodes = slicer.mrmlScene.GetNodesByClassByName( 'vtkMRMLEMSTemplateNode', taskName )

    if not templateNodes:
      # error!
      Helper.Error( "Could not find any template node after trying to load them!" )
      return False

    # we load the last template node which fits the taskname
    templateNode = templateNodes.GetItemAsObject( templateNodes.GetNumberOfItems() - 1 )

    loadResult = self.mrmlManager().SetLoadedParameterSetIndex( templateNode )

    if int( loadResult ) != 0:
      Helper.Info( "EMS node is corrupted - the manager could not be updated with new task: " + taskName )
      #return False
    else:
      Helper.Info( "Loading completed." )

    self.logic().DefineTclTaskFileFromMRML()

    return True


  def getTaskNames( self ):
    '''
    Get the taskNames of our tasksList (alphabetically sorted)
    '''
    list = self.__tasksList.keys()
    list.sort( lambda x, y: cmp( x.lower(), y.lower() ) )

    list.append( self.__newTaskString )

    return list

  def goSimple( self ):
    '''
    '''

    workflow = self.workflow()
    if not workflow:
      Helper.Error( "No valid workflow found!" )
      return False

    # we go forward in the simpleMode branch
    workflow.goForward( 'SimpleMode' )


  def goAdvanced( self ):
    '''
    '''

    workflow = self.workflow()
    if not workflow:
      Helper.Error( "No valid workflow found!" )
      return False

    # we go forward in the advancedMode branch
    workflow.goForward( 'AdvancedMode' )

  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    # we will try to load the selected task
    if self.loadTask():
      self.__parent.validationSucceeded( desiredBranchId )
    else:
      self.__parent.validationFailed( desiredBranchId, 'Load Task Error', 'There was an error loading the selected task!' )

