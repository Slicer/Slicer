
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
# _ui_LabelCreateDialog
#
#=============================================================================
class _ui_LabelCreateDialog(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    vLayout = qt.QVBoxLayout(parent)

    self.colorPromptLabel = qt.QLabel()
    vLayout.addWidget( self.colorPromptLabel )

    self.colorSelectorFrame = qt.QFrame()
    self.colorSelectorFrame.objectName = 'ColorSelectorFrame'
    self.colorSelectorFrame.setLayout( qt.QHBoxLayout() )
    vLayout.addWidget( self.colorSelectorFrame )

    self.colorSelectorLabel = qt.QLabel()
    self.colorSelectorFrame.layout().addWidget( self.colorSelectorLabel )

    self.colorSelector = slicer.qMRMLColorTableComboBox()
    self.colorSelector.nodeTypes = ["vtkMRMLColorNode"]
    self.colorSelector.hideChildNodeTypes = ("vtkMRMLDiffusionTensorDisplayPropertiesNode", "vtkMRMLProceduralColorNode", "")
    self.colorSelector.addEnabled = False
    self.colorSelector.removeEnabled = False
    self.colorSelector.noneEnabled = False
    self.colorSelector.selectNodeUponCreation = True
    self.colorSelector.showHidden = True
    self.colorSelector.showChildNodeTypes = True
    self.colorSelector.setMRMLScene( slicer.mrmlScene )
    self.colorSelector.setToolTip( "Pick the table of structures you wish to edit" )
    vLayout.addWidget( self.colorSelector )

    self.buttonBox = qt.QDialogButtonBox()
    self.buttonBox.setStandardButtons(qt.QDialogButtonBox.Ok |
                                      qt.QDialogButtonBox.Cancel)
    vLayout.addWidget(self.buttonBox)

    self.buttonBox.button(qt.QDialogButtonBox.Ok).setToolTip("Use currently selected color node.")
    self.buttonBox.button(qt.QDialogButtonBox.Cancel).setToolTip("Cancel current operation.")

#=============================================================================
#
# LabelCreateDialog
#
#=============================================================================
class LabelCreateDialog(object):

  colorNodeID = _map_property(lambda self: self.ui.colorSelector, "currentNodeID")

  #---------------------------------------------------------------------------
  def __init__(self, parent, master, mergeVolumePostfix):
    self._master = master
    self._mergeVolumePostfix = mergeVolumePostfix

    self.dialog = qt.QDialog(parent)
    self.dialog.objectName = 'EditorLabelCreateDialog'
    self.ui = _ui_LabelCreateDialog(self.dialog)

    if master is None:
        return

    self.ui.colorPromptLabel.text = """
    Create a merge label map or a segmentation for selected master volume %s.
    New volume will be %s.
    Select the color table node that will be used for segmentation labels.
    """ %(self._master.GetName(), self._master.GetName()+self._mergeVolumePostfix)

    self.ui.buttonBox.connect("accepted()", self.accept)
    self.ui.buttonBox.connect("rejected()", self.dialog, "reject()")

  #---------------------------------------------------------------------------
  def accept(self):
    self.dialog.accept()

  #---------------------------------------------------------------------------
  def exec_(self):
    return self.dialog.exec_()
