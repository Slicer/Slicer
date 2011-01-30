from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

vtkKWScale_ScaleValueChangedEvent = 10001
vtkKWScale_ScaleValueStartChangingEvent = 10002
vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000

class PythonGADScriptedModuleGUI(ScriptedModuleGUI):

    def __init__(self):
        ScriptedModuleGUI.__init__(self)
        self.GadNodeSelector = slicer.vtkSlicerNodeSelectorWidget()
        self.ConductanceScale = slicer.vtkKWScaleWithEntry()
        self.TimeStepScale = slicer.vtkKWScaleWithEntry()
        self.NumberOfIterationsScale = slicer.vtkKWScaleWithEntry()
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
        self.ConductanceScaleChangingTag = self.AddObserverByNumber(self.ConductanceScale,vtkKWScale_ScaleValueStartChangingEvent)
        self.ConductanceScaleChangedTag = self.AddObserverByNumber(self.ConductanceScale,vtkKWScale_ScaleValueChangedEvent)
      
        self.TimeStepScaleChangingTag = self.AddObserverByNumber(self.TimeStepScale,vtkKWScale_ScaleValueStartChangingEvent)
        self.TimeStepScaleChangedTag = self.AddObserverByNumber(self.TimeStepScale,vtkKWScale_ScaleValueChangedEvent)
      
        self.NumberOfIterationsScaleChangingTag = self.AddObserverByNumber(self.NumberOfIterationsScale,vtkKWScale_ScaleValueStartChangingEvent)
        self.NumberOfIterationsScaleChangedTag = self.AddObserverByNumber(self.NumberOfIterationsScale,vtkKWScale_ScaleValueChangedEvent)
        
        self.VolumeSelectorSelectedTag = self.AddObserverByNumber(self.VolumeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
        
        self.OutVolumeSelectorSelectedTag = self.AddObserverByNumber(self.OutVolumeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
        
        self.GadNodeSelectorSelectedTag = self.AddObserverByNumber(self.GadNodeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
        
        self.ApplyButtonTag = self.AddObserverByNumber(self.ApplyButton,vtkKWPushButton_InvokedEvent)

        #interactor = slicer.ApplicationGUI.GetRenderWindowInteractor()
        #self.LeftButtonReleaseTag = interactor.AddObserver("LeftButtonReleaseEvent",self.TestCallback)

        #sliceInteractor = slicer.ApplicationGUI.GetMainSliceGUI("Red").GetSliceViewer().GetRenderWidget().GetRenderWindowInteractor()
        #self.SliceLeftButtonReleaseTag = sliceInteractor.AddObserver("LeftButtonReleaseEvent",self.TestCallback2)
       
    def RemoveGUIObservers(self):
        self.RemoveObserver(self.ConductanceScaleChangingTag)
        self.RemoveObserver(self.ConductanceScaleChangedTag)
      
        self.RemoveObserver(self.TimeStepScaleChangingTag)
        self.RemoveObserver(self.TimeStepScaleChangedTag)
      
        self.RemoveObserver(self.NumberOfIterationsScaleChangingTag)
        self.RemoveObserver(self.NumberOfIterationsScaleChangedTag)
        
        self.RemoveObserver(self.VolumeSelectorSelectedTag)
        
        self.RemoveObserver(self.OutVolumeSelectorSelectedTag)
        
        self.RemoveObserver(self.GadNodeSelectorSelectedTag)
        
        self.RemoveObserver(self.ApplyButtonTag)

        #self.RemoveObserver(self.LeftButtonReleaseTag)
        #self.RemoveObserver(self.SliceLeftButtonReleaseTag)

    def ProcessGUIEvents(self,caller,event):
        if caller == self.ConductanceScale and event == vtkKWScale_ScaleValueChangedEvent:
            self.UpdateMRML()
        elif caller == self.TimeStepScale and event == vtkKWScale_ScaleValueChangedEvent: 
            self.UpdateMRML()
        elif caller == self.NumberOfIterationsScale and event == vtkKWScale_ScaleValueChangedEvent:
            self.UpdateMRML()
        elif caller == self.VolumeSelector and event == vtkSlicerNodeSelectorWidget_NodeSelectedEvent and self.VolumeSelector.GetSelected():
            self.UpdateMRML()
        elif caller == self.OutVolumeSelector and event == vtkSlicerNodeSelectorWidget_NodeSelectedEvent and self.OutVolumeSelector.GetSelected(): 
            self.UpdateMRML()
        elif caller == self.GadNodeSelector and event == vtkSlicerNodeSelectorWidget_NodeSelectedEvent and self.GadNodeSelector.GetSelected():
            node = self.GadNodeSelector.GetSelected()
            self.GetLogic().SetAndObserveScriptedModuleNode(node)
            self.SetAndObserveScriptedModuleNode(node)
            self.UpdateGUI()
        elif caller == self.ApplyButton and event == vtkKWPushButton_InvokedEvent:
            self.UpdateMRML()
            self.Apply()

    def TestCallback(self):
        dialog = slicer.vtkKWMessageDialog()
        dialog.SetParent(slicer.ApplicationGUI.GetMainSlicerWindow())
        dialog.SetMasterWindow(slicer.ApplicationGUI.GetMainSlicerWindow())
        dialog.SetStyleToMessage()
        dialog.SetText("TestCallback")
        dialog.Create()
        dialog.Invoke()

    def TestCallback2(self):
        dialog = slicer.vtkKWMessageDialog()
        dialog.SetParent(slicer.ApplicationGUI.GetMainSlicerWindow())
        dialog.SetMasterWindow(slicer.ApplicationGUI.GetMainSlicerWindow())
        dialog.SetStyleToMessage()
        dialog.SetText("TestCallback2")
        dialog.Create()
        dialog.Invoke()

    def Apply(self):

        if not self.GetScriptedModuleNode():
          slicer.Application.ErrorMessage("No input ScriptedModuleNode found")
          return

        scriptedModuleNode = self.GetScriptedModuleNode()
    
        inVolume = scriptedModuleNode.GetParameter('InputVolumeRef')
        if not inVolume:
          slicer.Application.ErrorMessage("No input volume found")
          return
      
        outVolume = scriptedModuleNode.GetParameter('OutputVolumeRef')
        if not outVolume:
          slicer.Application.ErrorMessage("No output volume found")
          return
    
        outVolume.CopyOrientation(inVolume)
        outVolume.SetAndObserveTransformNodeID(inVolume.GetTransformNodeID())
    
        gradientAnisotropicDiffusionImageFilter = slicer.vtkITKGradientAnisotropicDiffusionImageFilter()
        gradientAnisotropicDiffusionImageFilter.SetInput(inVolume.GetImageData())
        gradientAnisotropicDiffusionImageFilter.SetConductanceParameter(scriptedModuleNode.GetParameter('Conductance'))
        gradientAnisotropicDiffusionImageFilter.SetNumberOfIterations(scriptedModuleNode.GetParameter('NumberOfIterations'))
        gradientAnisotropicDiffusionImageFilter.SetTimeStep(scriptedModuleNode.GetParameter('TimeStep'))
        gradientAnisotropicDiffusionImageFilter.Update()
    
        image = slicer.vtkImageData()
        image.DeepCopy(gradientAnisotropicDiffusionImageFilter.GetOutput())
        outVolume.SetAndObserveImageData(image)
        outVolume.ModifiedSinceReadOn()
       
        slicer.Application.InformationMessage("Done applying GradientAnisotropicDiffusion.")

    def UpdateMRML(self):
        node = self.GetScriptedModuleNode()
        if not node:
            self.GadNodeSelector.SetSelectedNew("vtkMRMLScriptedModuleNode")
            self.GadNodeSelector.ProcessNewNodeCommand("vtkMRMLScriptedModuleNode", "GADParameters")
            node = self.GadNodeSelector.GetSelected()
            self.GetLogic().SetAndObserveScriptedModuleNode(node)
            self.SetScriptedModuleNode(node)
      
        self.GetLogic().GetMRMLScene().SaveStateForUndo(node)
     
        node.SetParameter('Conductance',self.ConductanceScale.GetValue())
        node.SetParameter('TimeStep',self.TimeStepScale.GetValue())
        node.SetParameter('NumberOfIterations',self.NumberOfIterationsScale.GetValue())  
        
        if self.VolumeSelector.GetSelected():
            node.SetParameter('InputVolumeRef',self.VolumeSelector.GetSelected())
              
        if self.OutVolumeSelector.GetSelected():
            node.SetParameter('OutputVolumeRef',self.OutVolumeSelector.GetSelected())

    def UpdateGUI(self):
        node = self.GetScriptedModuleNode()
        if node:
            conductance = node.GetParameter('Conductance')
            if conductance:
                self.ConductanceScale.SetValue(conductance)
            numberOfIterations = node.GetParameter('NumberOfIterations')
            if numberOfIterations:
                self.NumberOfIterationsScale.SetValue(numberOfIterations)
            timeStep = node.GetParameter('TimeStep')
            if timeStep:
                self.TimeStepScale.SetValue(timeStep)

    def ProcessMRMLEvents(self,caller,event):
        if caller == self.GetScriptedModuleNode():
            self.UpdateGUI()

    def BuildGUI(self):
        self.GetUIPanel().AddPage("GradientAnisotropicDiffusionFilter","GradientAnisotropicDiffusionFilter","")
        pageWidget = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter")
        helpText = "Flex, dude!"
        aboutText = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details."
        self.BuildHelpAndAboutFrame(pageWidget,helpText,aboutText)
    
        moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        moduleFrame.SetParent(self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter"))
        moduleFrame.Create()
        moduleFrame.SetLabelText("Gradient Anisotropic Diffusion Filter")
        moduleFrame.ExpandFrame()
        widgetName = moduleFrame.GetWidgetName()
        pageWidgetName = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter").GetWidgetName()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))
    
        self.GadNodeSelector.SetNodeClass("vtkMRMLScriptedModuleNode", "ScriptedModuleName", self.GetLogic().GetModuleName(), "GADParameters")
        self.GadNodeSelector.NewNodeEnabledOn()
        self.GadNodeSelector.NoneEnabledOn()
        self.GadNodeSelector.ShowHiddenOn()
        self.GadNodeSelector.SetParent(moduleFrame.GetFrame())
        self.GadNodeSelector.Create()
        self.GadNodeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self.GadNodeSelector.UpdateMenu()
        self.GadNodeSelector.SetBorderWidth(2)
        self.GadNodeSelector.SetLabelText("GAD Parameters")
        self.GadNodeSelector.SetBalloonHelpString("select a GAD node from the current mrml scene.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.GadNodeSelector.GetWidgetName())
    
        self.ConductanceScale.SetParent(moduleFrame.GetFrame())
        self.ConductanceScale.SetLabelText("Conductance")
        self.ConductanceScale.Create()
        w = self.ConductanceScale.GetScale().GetWidth()
        self.ConductanceScale.SetRange(0,10)
        self.ConductanceScale.SetResolution(0.1)
        self.ConductanceScale.SetValue(1.0) 
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.ConductanceScale.GetWidgetName())
    
        self.TimeStepScale.SetParent(moduleFrame.GetFrame())
        self.TimeStepScale.SetLabelText("Time Step")
        self.TimeStepScale.Create()
        self.TimeStepScale.GetScale().SetWidth(w)
        self.TimeStepScale.SetRange(0.0,1.0)
        self.TimeStepScale.SetValue(0.1)
        self.TimeStepScale.SetResolution(0.01)
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.TimeStepScale.GetWidgetName())
    
        self.NumberOfIterationsScale.SetParent(moduleFrame.GetFrame())
        self.NumberOfIterationsScale.SetLabelText("Iterations")
        self.NumberOfIterationsScale.Create()
        self.NumberOfIterationsScale.GetScale().SetWidth(w)
        self.NumberOfIterationsScale.SetValue(1)
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.NumberOfIterationsScale.GetWidgetName())
    
        self.VolumeSelector.SetNodeClass("vtkMRMLScalarVolumeNode","","","")
        self.VolumeSelector.SetParent(moduleFrame.GetFrame())
        self.VolumeSelector.Create()
        self.VolumeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self.VolumeSelector.UpdateMenu()
        self.VolumeSelector.SetBorderWidth(2)
        self.VolumeSelector.SetLabelText("Input Volume: ")
        self.VolumeSelector.SetBalloonHelpString("select an input volume from the current mrml scene.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % self.VolumeSelector.GetWidgetName())
    
        self.OutVolumeSelector.SetNodeClass("vtkMRMLScalarVolumeNode","","","GADVolumeOut")
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
        self.GadNodeSelector.SetParent(None)
        self.GadNodeSelector = None
        self.ConductanceScale.SetParent(None)
        self.ConductanceScale = None
        self.TimeStepScale.SetParent(None)
        self.TimeStepScale = None
        self.NumberOfIterationsScale.SetParent(None)
        self.NumberOfIterationsScale = None
        self.VolumeSelector.SetParent(None)
        self.VolumeSelector = None
        self.OutVolumeSelector.SetParent(None)
        self.OutVolumeSelector = None
        self.ApplyButton.SetParent(None)
        self.ApplyButton = None
   
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter")
            self.GetUIPanel().RemovePage("GradientAnisotropicDiffusionFilter")

