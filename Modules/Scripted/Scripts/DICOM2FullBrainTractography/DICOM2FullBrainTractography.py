from __main__ import vtk, qt, ctk, slicer
import DICOM2FullBrainTractographyLib
#
# DICOM2FullBrainTractography
#

class DICOM2FullBrainTractography:
  def __init__(self, parent):
    parent.title = "DWI to Full Brain Tractography"
    parent.category = "Diffusion"
    parent.contributor = ""
    parent.helpText = """
    Example of scripted loadable extension.
    """
    parent.acknowledgementText = """
    This module was developed by Demian Wassermann, LMI/PNL/SPL Brigham and Women's Hospital & Harvard Medical School
    """
    self.parent = parent

#
# qDICOM2FullBrainTractographyWidget
#

class DICOM2FullBrainTractographyWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()
    
  def setup(self):
    # Instantiate and connect widgets ...
    
    # Collapsible button
    #dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    #dummyCollapsibleButton.text = "A collapsible button"

    full_tractography_workflow = DICOM2FullBrainTractographyLib.full_tractography_workflow(
        DICOM2FullBrainTractographyLib.WorkflowConfiguration(),
        self.layout
    )

  def enter(self):
      pass

  def exit(self):
      pass
    

