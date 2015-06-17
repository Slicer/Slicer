
import qt
import slicer

#-----------------------------------------------------------------------------
def _map_property(objfunc, name):
  """Creates a Python :class:`property` associated with an object
  ``attributename``. ``objfunc`` is a function that takes a ``self`` as
  parameter and returns the object to consider.
  """
  return property(lambda self: getattr(objfunc(self), name),
                  lambda self, value: setattr(objfunc(self), name, value))

#=============================================================================
#
# _ui_LabelSelectDialog
#
#=============================================================================
class _ui_LabelSelectDialog(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    vLayout = qt.QVBoxLayout(parent)

    self.labelPromptLabel = qt.QLabel()
    vLayout.addWidget( self.labelPromptLabel )

    self.labelSelectorFrame = qt.QFrame()
    self.labelSelectorFrame.setLayout( qt.QHBoxLayout() )
    vLayout.addWidget( self.labelSelectorFrame )

    self.labelSelectorLabel = qt.QLabel()
    self.labelPromptLabel.setText( "Select existing label map volume to edit." )
    self.labelSelectorFrame.layout().addWidget( self.labelSelectorLabel )

    self.labelSelector = slicer.qMRMLNodeComboBox()
    self.labelSelector.nodeTypes = ( "vtkMRMLLabelMapVolumeNode", "" )
    self.labelSelector.selectNodeUponCreation = False
    self.labelSelector.addEnabled = False
    self.labelSelector.noneEnabled = False
    self.labelSelector.removeEnabled = False
    self.labelSelector.showHidden = False
    self.labelSelector.showChildNodeTypes = False
    self.labelSelector.setMRMLScene( slicer.mrmlScene )
    self.labelSelector.setToolTip( "Pick the label map to edit" )
    self.labelSelectorFrame.layout().addWidget( self.labelSelector )

    self.labelButtonFrame = qt.QFrame()
    self.labelButtonFrame.setLayout( qt.QHBoxLayout() )
    vLayout.addWidget( self.labelButtonFrame )

    self.labelDialogApply = qt.QPushButton("Apply", self.labelButtonFrame)
    self.labelDialogApply.setToolTip( "Use currently selected label node." )
    self.labelButtonFrame.layout().addWidget(self.labelDialogApply)

    self.labelDialogCancel = qt.QPushButton("Cancel", self.labelButtonFrame)
    self.labelDialogCancel.setToolTip( "Cancel current operation." )
    self.labelButtonFrame.layout().addWidget(self.labelDialogCancel)

    self.labelButtonFrame.layout().addStretch(1)

    self.labelDialogCreate = qt.QPushButton("Create New...", self.labelButtonFrame)
    self.labelDialogCreate.setToolTip( "Cancel current operation." )
    self.labelButtonFrame.layout().addWidget(self.labelDialogCreate)


#=============================================================================
#
# LabelSelectDialog
#
#=============================================================================
class LabelSelectDialog(object):

  labelNodeID = _map_property(lambda self: self.ui.labelSelector, "currentNodeID")

  #---------------------------------------------------------------------------
  def __init__(self, parent):
    self.dialog = qt.QDialog(parent)
    self.dialog.objectName = 'EditorLabelSelectDialog'
    self.ui = _ui_LabelSelectDialog(self.dialog)

    self.ui.labelDialogApply.connect("clicked()", self.accept)
    self.ui.labelDialogCancel.connect("clicked()", self.dialog, "reject()")

    self.createNew = False
    self.ui.labelDialogCreate.connect("clicked()", self.onLabelDialogCreate)

  #---------------------------------------------------------------------------
  def accept(self):
    self.dialog.accept()

  #---------------------------------------------------------------------------
  def exec_(self):
    return self.dialog.exec_()

  #---------------------------------------------------------------------------
  def onLabelDialogCreate(self):
    self.createNew = True
    self.dialog.reject()
