from __main__ import qt, ctk
from Helper import *

class EMSegmentDynamicFrame:

  def __init__( self ):
    '''
    '''
    self.__layout = None
    self.__checkButtonElementIDs = dict()
    self.__textLabelElementIDs = dict()
    self.__mrmlManager = None
    self.__logic = None

  def setLayout( self, layout ):
    '''
    '''
    self.__layout = layout

  def layout( self ):
    '''
    '''

    return self.__layout


  def getElementByID( self, id ):
    '''
    '''
    if not self.__elementIDs.has_key( id ):
      Helper.Error( "Could not find element!" )
    else:
      return self.__elementIDs[id]

  def saveCheckButtonElement( self, id, element ):
    '''
    '''
    if ( not id >= 0 )  or not element:
      Helper.Error( "Wrong call: id and element have to be set" )
      return False

    if self.__checkButtonElementIDs.has_key( id ):
      Helper.Warning( "We are overwriting an element.." )

    self.__checkButtonElementIDs[id] = element

  def saveTextLabelElement( self, id, element ):
    '''
    '''
    if ( not id >= 0 )  or not element:
      Helper.Error( "Wrong call: id and element have to be set" )
      return False

    if self.__textLabelElementIDs.has_key( id ):
      Helper.Warning( "We are overwriting an element.." )

    self.__textLabelElementIDs[id] = element

  def clearElements( self ):
    '''
    '''
    if self.layout():

      # clear off the layout from checkbuttons
      for n in self.__checkButtonElementIDs.keys():
        print "removing.."
        w = self.__checkButtonElementIDs[n]
        if self.layout():
          self.layout().removeWidget( w )
        w.deleteLater()
        w.setParent( None )
        w = None
        self.__checkButtonElementIDs[n] = None

      # clear off the layout from textlabels
      for n in self.__textLabelElementIDs.keys():
        print "removing.."
        w = self.__textLabelElementIDs[n]
        if self.layout():
          self.layout().removeWidget( w )
        w.deleteLater()
        w.setParent( None )
        w = None
        self.__textLabelElementIDs[n] = None

    # reset the dicts
    self.__checkButtonElementIDs = dict()
    self.__textLabelElementIDs = dict()


  def DefineCheckButton( self, label, initState, id ):
    '''
    '''

    newCheckButton = qt.QCheckBox( label )
    newCheckButton.setChecked( bool( initState ) )

    if self.layout():
      self.layout().addWidget( newCheckButton )
      self.saveCheckButtonElement( id, newCheckButton )
      return True
    else:
      return False

  def DefineTextLabel( self, label, id ):
    '''
    '''
    labelReplaced = label.replace( "\n", "<br>" )

    newLabel = qt.QLabel( labelReplaced )

    if self.layout():
      self.layout().addWidget( newLabel )
      self.saveTextLabelElement( id, newLabel )
      return True
    else:
      return False

  def setMRMLManager( self, mrmlManager ):
    self.__mrmlManager = mrmlManager

  def setLogic( self, logic ):
    self.__logic = logic

  def mrmlManager( self ):
    return self.__mrmlManager

  def logic( self ):
    return self.__logic


  def SaveSettingsToMRML( self ):
    '''
    '''
    oldDefText = self.mrmlManager().GetGlobalParametersNode().GetTaskPreProcessingSetting()

    if not oldDefText:
      return

    # let's split it
    defList = oldDefText.split( ":" )

    # filter empties
    defList = filter( None, defList )

    # loop through the controls and build the defText

    # checkboxes
    for id in self.__checkButtonElementIDs.keys():

      currentCheckbox = self.__checkButtonElementIDs[id]

      newDefText = "C"

      if currentCheckbox:

        if currentCheckbox.checked:
          newDefText += "1"
        else:
          newDefText += "0"

        # replace value in defList
        defList[id] = newDefText

        #Helper.Debug( "Replaced item " + str( id ) + " with " + str( newDefText ) )

    # TODO
    # volumes

    # TODO
    # textedits

    #
    #
    # Final step:

    # build the defText string
    newDefText = ":"
    newDefText += ":".join( defList )

    # and propagate it to MRML
    self.mrmlManager().GetGlobalParametersNode().SetTaskPreProcessingSetting( newDefText )


  def LoadSettingsFromMRML( self ):
    '''
    '''
    defText = self.mrmlManager().GetGlobalParametersNode().GetTaskPreProcessingSetting()

    if not defText:
      return

    # the defText string is formated like
    # C1:C1
    # which means two checkboxes, both enabled

    # let's split it
    defList = defText.split( ":" )

    # filter empties
    defList = filter( None, defList )

    if len( defList ) > 0:

      cIndex = -1

      for d in defList:

        # loop through all entries in the definition list

        if len( d ) < 2:
          # definition texts need two characters
          continue

        # checkboxes
        if d[0] == "C":

          # cIndex is the index of checkboxes, increase it
          cIndex += 1

          # checkbox
          if self.__checkButtonElementIDs.has_key( cIndex ):

            cState = int( d[1] )

            currentCheckBox = self.__checkButtonElementIDs[cIndex]

            if cState == 0:
              currentCheckBox.setChecked( False )
            else:
              currentCheckBox.setChecked( True )

        # TODO volumes

        # TODO textedits




