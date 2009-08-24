from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000

class ExtractSubvolumeGUI(ScriptedModuleGUI):

    def __init__(self):
        ScriptedModuleGUI.__init__(self)
        self.vtkScriptedModuleGUI.SetCategory("Converters")
        self.FiducialsNodeSelector = slicer.vtkSlicerNodeSelectorWidget()
        self.VolumeSelector = slicer.vtkSlicerNodeSelectorWidget()
        self.OutVolumeSelector = slicer.vtkSlicerNodeSelectorWidget()
        self.ApplyButton = slicer.vtkKWPushButton()
    
    def Destructor(self):
        pass
    
    def RemoveMRMLNodeObservers(self):
        pass
    
    def RemoveLogicObservers(self):
        pass

    def AddGUIObservers(self):
        self.ApplyButtonTag = self.AddObserverByNumber(self.ApplyButton,vtkKWPushButton_InvokedEvent)

    def RemoveGUIObservers(self):
        self.RemoveObserver(self.ApplyButtonTag)

    def ProcessGUIEvents(self,caller,event):
        if caller == self.ApplyButton and event == vtkKWPushButton_InvokedEvent:
            self.Apply()

    def Status(self,text):
        slicer.ApplicationGUI.GetMainSlicerWindow().SetStatusText(text)

    def ErrorDialog(self,text):
        slicer.Application.InformationMessage(text)
        dialog = slicer.vtkKWMessageDialog()
        parent = slicer.ApplicationGUI.GetMainSlicerWindow()
        dialog.SetParent(parent)
        dialog.SetMasterWindow(parent)
        dialog.SetStyleToMessage()
        dialog.SetText(text)
        dialog.Create()
        dialog.Invoke()

    def Apply(self):

        inputVolume = self.VolumeSelector.GetSelected()
        if not inputVolume:
          self.ErrorDialog("No input volume found")
          return
      
        outputVolume = self.OutVolumeSelector.GetSelected()
        if not outputVolume:
          self.ErrorDialog("No output volume found")
          return

        inputSeeds = self.FiducialsNodeSelector .GetSelected()
        if not inputSeeds:
          self.ErrorDialog("No fiducials found")
          return

        if inputSeeds.GetNumberOfFiducials() < 2:
          self.ErrorDialog("Need at least 2 fiducials in the list")
          return

        self.Status("Applying Subvolume...")

        # get the seed points (RAS)
        p0 = inputSeeds.GetNthFiducialXYZ(0)
        p1 = inputSeeds.GetNthFiducialXYZ(1)
        lower = map(min,p0,p1)
        

        # convert seed points to input volume IJK
        rasToIJK = slicer.vtkMatrix4x4()
        inputVolume.GetRASToIJKMatrix(rasToIJK)
        p0IJK = rasToIJK.MultiplyPoint(p0[0],p0[1],p0[2],1.0)
        p1IJK = rasToIJK.MultiplyPoint(p1[0],p1[1],p1[2],1.0)

        # find the bounding box of the seeds
        lowerIJK = map(int, map(min, p0IJK, p1IJK ) )
        upperIJK = map(int, map(max, p0IJK, p1IJK ) )

        # get an array of the input volume, extract the sub
        a = inputVolume.GetImageData().ToArray()
        sub = a[lowerIJK[2]:upperIJK[2], lowerIJK[1]:upperIJK[1], lowerIJK[0]:upperIJK[0]]
        
        # set up output node
        outImage = slicer.vtkImageData()
        outImage.SetDimensions(sub.shape[2],sub.shape[1],sub.shape[0])
        outImage.AllocateScalars()
        outImage.ToArray()[:] = sub[:]
        outputVolume.SetAndObserveImageData(outImage)
        rasToIJK.Invert()
        ijkToRAS = rasToIJK
        outputVolume.SetIJKToRASMatrix(rasToIJK)
        origin = ijkToRAS.MultiplyPoint(lowerIJK[0],lowerIJK[1],lowerIJK[2],1.0)
        outputVolume.SetOrigin(origin[0], origin[1], origin[2])
        outputVolume.ModifiedSinceReadOn()

        displayNode = inputVolume.GetDisplayNode()
        if displayNode != None:
          newDisplayNode = displayNode.NewInstance()
          newDisplayNode.Copy(displayNode)
          slicer.MRMLScene.AddNodeNoNotify(newDisplayNode);
          outputVolume.SetAndObserveDisplayNodeID(newDisplayNode.GetID())

        appLogic = slicer.ApplicationLogic
        selectionNode = appLogic.GetSelectionNode()
        if inputVolume.GetLabelMap():
          outputVolume.SetLabelMap(1)
          selectionNode.SetReferenceActiveLabelVolumeID(outputVolume.GetID())
        else:
          selectionNode.SetReferenceActiveVolumeID(outputVolume.GetID())
        appLogic.PropagateVolumeSelection()
       
        self.Status("Done applying Subvolume.")

    def BuildGUI(self):
        self.GetUIPanel().AddPage("Extract Subvolume","Extract Subvolume","")
        pageWidget = self.GetUIPanel().GetPageWidget("Extract Subvolume")
        helpText = "Create a new volume that contains the extent of the input volume that falls within the space defined by two fiducials.  Note that the fiducials may not exactly match the voxel boundaries of the input volume so not all cases are handled.  The first two fiducials of the input list are used.  See <a>http://www.slicer.org/slicerWiki/index.php/Modules:ExtractSubvolume-Documentation-3.4</a> for more information."
        aboutText = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.  Module implemented by Steve Pieper."
        self.BuildHelpAndAboutFrame(pageWidget,helpText,aboutText)
    
        moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        moduleFrame.SetParent(self.GetUIPanel().GetPageWidget("Extract Subvolume"))
        moduleFrame.Create()
        moduleFrame.SetLabelText("Extract Subvolume")
        moduleFrame.ExpandFrame()
        widgetName = moduleFrame.GetWidgetName()
        pageWidgetName = self.GetUIPanel().GetPageWidget("Extract Subvolume").GetWidgetName()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))
    
        self.VolumeSelector.SetNodeClass("vtkMRMLScalarVolumeNode","","","")
        self.VolumeSelector.SetParent(moduleFrame.GetFrame())
        self.VolumeSelector.Create()
        self.VolumeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self.VolumeSelector.UpdateMenu()
        self.VolumeSelector.SetBorderWidth(2)
        self.VolumeSelector.SetLabelText("Input Volume: ")
        self.VolumeSelector.SetBalloonHelpString("select an input volume from the current mrml scene.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.VolumeSelector.GetWidgetName())
    
        self.FiducialsNodeSelector.SetNodeClass("vtkMRMLFiducialListNode","","","")
        self.FiducialsNodeSelector.SetParent(moduleFrame.GetFrame())
        self.FiducialsNodeSelector.Create()
        self.FiducialsNodeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self.FiducialsNodeSelector.UpdateMenu()
        self.FiducialsNodeSelector.SetBorderWidth(2)
        self.FiducialsNodeSelector.SetLabelText("Input Fiducials: ")
        self.FiducialsNodeSelector.SetBalloonHelpString("select a fiducial list (the first two fiducial points will define the subvolume.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.FiducialsNodeSelector.GetWidgetName())
    
        self.OutVolumeSelector.SetNodeClass("vtkMRMLScalarVolumeNode","","","Extract Subvolume VolumeOut")
        self.OutVolumeSelector.SetNewNodeEnabled(1)
        self.OutVolumeSelector.SetParent(moduleFrame.GetFrame())
        self.OutVolumeSelector.Create()
        self.OutVolumeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self.OutVolumeSelector.UpdateMenu()
        self.OutVolumeSelector.SetBorderWidth(2)
        self.OutVolumeSelector.SetLabelText("Output Volume: ")
        self.OutVolumeSelector.SetBalloonHelpString("select an output volume from the current mrml scene.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.OutVolumeSelector.GetWidgetName())
    
        self.ApplyButton.SetParent(moduleFrame.GetFrame())
        self.ApplyButton.Create()
        self.ApplyButton.SetText("Apply")
        self.ApplyButton.SetWidth(8)
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 10" % self.ApplyButton.GetWidgetName())

    def TearDownGUI(self):
        self.VolumeSelector.SetParent(None)
        self.VolumeSelector = None
        self.FiducialsNodeSelector.SetParent(None)
        self.FiducialsNodeSelector = None
        self.OutVolumeSelector.SetParent(None)
        self.OutVolumeSelector = None
        self.ApplyButton.SetParent(None)
        self.ApplyButton = None
   
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("Extract Subvolume")
            self.GetUIPanel().RemovePage("Extract Subvolume")

