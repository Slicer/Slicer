import Slicer

slicer = Slicer.Slicer()

gadNodeSelector = slicer.vtkSlicerNodeSelectorWidget.New()
conductanceScale = slicer.vtkKWScaleWithEntry.New()
timeStepScale = slicer.vtkKWScaleWithEntry.New()
numberOfIterationsScale = slicer.vtkKWScaleWithEntry.New()
volumeSelector = slicer.vtkSlicerNodeSelectorWidget.New()
outVolumeSelector = slicer.vtkSlicerNodeSelectorWidget.New()
applyButton = slicer.vtkKWPushButton.New()

vtkKWScale_ScaleValueChangedEvent = 10001
vtkKWScale_ScaleValueStartChangingEvent = 10002
vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000


def Destructor(self):

    gadNodeSelector.Delete()
    conductanceScale.Delete()
    timeStepScale.Delete()
    numberOfIterationsScale.Delete()
    volumeSelector.Delete()
    outVolumeSelector.Delete()
    applyButton.Delete()


def RemoveMRMLNodeObservers(self):

    pass


def RemoveLogicObservers(self):

    pass


def AddGUIObservers(self):

    self = Slicer.SlicerWrapper(slicer,self)

    self.AddObserverByNumber(conductanceScale,vtkKWScale_ScaleValueStartChangingEvent)
    self.AddObserverByNumber(conductanceScale,vtkKWScale_ScaleValueChangedEvent)
  
    self.AddObserverByNumber(timeStepScale,vtkKWScale_ScaleValueStartChangingEvent)
    self.AddObserverByNumber(timeStepScale,vtkKWScale_ScaleValueChangedEvent)
  
    self.AddObserverByNumber(numberOfIterationsScale,vtkKWScale_ScaleValueStartChangingEvent)
    self.AddObserverByNumber(numberOfIterationsScale,vtkKWScale_ScaleValueChangedEvent)
    
    self.AddObserverByNumber(volumeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
    
    self.AddObserverByNumber(outVolumeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
    
    self.AddObserverByNumber(gadNodeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
    
    self.AddObserverByNumber(applyButton,vtkKWPushButton_InvokedEvent)


def RemoveGUIObservers(self):

    self = Slicer.SlicerWrapper(slicer,self)

    self.RemoveObserverByNumber(conductanceScale,vtkKWScale_ScaleValueStartChangingEvent)
    self.RemoveObserverByNumber(conductanceScale,vtkKWScale_ScaleValueChangedEvent)
  
    self.RemoveObserverByNumber(timeStepScale,vtkKWScale_ScaleValueStartChangingEvent)
    self.RemoveObserverByNumber(timeStepScale,vtkKWScale_ScaleValueChangedEvent)
  
    self.RemoveObserverByNumber(numberOfIterationsScale,vtkKWScale_ScaleValueStartChangingEvent)
    self.RemoveObserverByNumber(numberOfIterationsScale,vtkKWScale_ScaleValueChangedEvent)
    
    self.RemoveObserverByNumber(volumeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
    
    self.RemoveObserverByNumber(outVolumeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
    
    self.RemoveObserverByNumber(gadNodeSelector,vtkSlicerNodeSelectorWidget_NodeSelectedEvent)
    
    self.RemoveObserverByNumber(applyButton,vtkKWPushButton_InvokedEvent)


def ProcessGUIEvents(self,caller,event):

    self = Slicer.SlicerWrapper(slicer,self)
    caller = Slicer.SlicerWrapper(slicer,caller)

    if str(caller) == str(conductanceScale) and event == vtkKWScale_ScaleValueChangedEvent:
        UpdateMRML(self)
    elif str(caller) == str(timeStepScale) and event == vtkKWScale_ScaleValueChangedEvent: 
        UpdateMRML(self)
    elif str(caller) == str(numberOfIterationsScale) and event == vtkKWScale_ScaleValueChangedEvent:
        UpdateMRML(self)
    elif str(caller) == str(volumeSelector) and event == vtkSlicerNodeSelectorWidget_NodeSelectedEvent and volumeSelector.GetSelected() != []:
        UpdateMRML(self)
    elif str(caller) == str(outVolumeSelector) and event == vtkSlicerNodeSelectorWidget_NodeSelectedEvent and outVolumeSelector.GetSelected() != []: 
        UpdateMRML(self)
    elif str(caller) == str(gadNodeSelector) and event == vtkSlicerNodeSelectorWidget_NodeSelectedEvent and gadNodeSelector.GetSelected() != []:
        node = gadNodeSelector.GetSelected()
        self.GetLogic().SetAndObserveScriptedModuleNode(node)
        self.SetAndObserveScriptedModuleNode(node)
        UpdateGUI(self)
    elif str(caller) == str(applyButton) and event == vtkKWPushButton_InvokedEvent:
        UpdateMRML(self)
        Apply(self)


def Apply(self):
  
    if self.GetScriptedModuleNode() == []:
      slicer.Application.ErrorMessage("No input ScriptedModuleNode found")
      return

    scriptedModuleNode = self.GetScriptedModuleNode()

    inVolume = scriptedModuleNode.GetParameter('InputVolumeRef')
    if inVolume == []:
      slicer.Application.ErrorMessage("No input volume found")
      return
  
    outVolume = scriptedModuleNode.GetParameter('OutputVolumeRef')
    if outVolume == []:
      slicer.Application.ErrorMessage("No output volume found")
      return

    outVolume.CopyOrientation(inVolume)
    outVolume.SetAndObserveTransformNodeID(inVolume.GetTransformNodeID())

    gradientAnisotropicDiffusionImageFilter = slicer.vtkITKGradientAnisotropicDiffusionImageFilter.New()
    gradientAnisotropicDiffusionImageFilter.SetInput(inVolume.GetImageData())
    gradientAnisotropicDiffusionImageFilter.SetConductanceParameter(scriptedModuleNode.GetParameter('Conductance'))
    gradientAnisotropicDiffusionImageFilter.SetNumberOfIterations(scriptedModuleNode.GetParameter('NumberOfIterations'))
    gradientAnisotropicDiffusionImageFilter.SetTimeStep(scriptedModuleNode.GetParameter('TimeStep'))
    gradientAnisotropicDiffusionImageFilter.Update()

    image = slicer.vtkImageData.New()
    image.DeepCopy(gradientAnisotropicDiffusionImageFilter.GetOutput())
    outVolume.SetAndObserveImageData(image)
    image.Delete()
    outVolume.ModifiedSinceReadOn()
   
    gradientAnisotropicDiffusionImageFilter.Delete()

    slicer.Application.InformationMessage("Done applying GradientAnisotropicDiffusion.")


def UpdateMRML(self):

    node = self.GetScriptedModuleNode()
    if node == []:
        gadNodeSelector.SetSelectedNew("vtkMRMLScriptedModuleNode")
        gadNodeSelector.ProcessNewNodeCommand("vtkMRMLScriptedModuleNode", "GADParameters")
        node = gadNodeSelector.GetSelected()
        self.GetLogic().SetAndObserveScriptedModuleNode(node)
        self.SetScriptedModuleNode(node)
  
    self.GetLogic().GetMRMLScene().SaveStateForUndo(node)
 
    node.SetParameter('Conductance',conductanceScale.GetValue())
    node.SetParameter('TimeStep',timeStepScale.GetValue())
    node.SetParameter('NumberOfIterations',numberOfIterationsScale.GetValue())  
    
    if volumeSelector.GetSelected() != []:
        node.SetParameter('InputVolumeRef',volumeSelector.GetSelected())
          
    if outVolumeSelector.GetSelected() != []:
        node.SetParameter('OutputVolumeRef',outVolumeSelector.GetSelected())


def UpdateGUI(self):

    self = Slicer.SlicerWrapper(slicer,self)
    node = self.GetScriptedModuleNode()
    if node != []:
        conductance = node.GetParameter('Conductance')
        if conductance != []:
            conductanceScale.SetValue(conductance)
        numberOfIterations = node.GetParameter('NumberOfIterations')
        if numberOfIterations != []:
            numberOfIterationsScale.SetValue(numberOfIterations)
        timeStep = node.GetParameter('TimeStep')
        if timeStep != []:
            timeStepScale.SetValue(timeStep)


def ProcessMRMLEvents(self,caller,event):

    self = Slicer.SlicerWrapper(slicer,self)
    caller = Slicer.SlicerWrapper(slicer,caller)
    if str(self.GetScriptedModuleNode()) == str(caller):
        UpdateGUI(self)


def BuildGUI(self):

    self = Slicer.SlicerWrapper(slicer,self)

    self.GetUIPanel().AddPage("GradientAnisotropicDiffusionFilter","GradientAnisotropicDiffusionFilter","")
    pageWidget = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter")
    helpText = "Flex, dude!"
    aboutText = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details."
    self.BuildHelpAndAboutFrame(pageWidget,helpText,aboutText)

    moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame.New()
    moduleFrame.SetParent(self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter"))
    moduleFrame.Create()
    moduleFrame.SetLabelText("Gradient Anisotropic Diffusion Filter")
    moduleFrame.ExpandFrame()
    widgetName = moduleFrame.GetWidgetName()
    pageWidgetName = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter").GetWidgetName()
    Slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))

    gadNodeSelector.SetNodeClass("vtkMRMLScriptedModuleNode", "ScriptedModuleName", self.GetLogic().GetModuleName(), "GADParameters")
    gadNodeSelector.NewNodeEnabledOn()
    gadNodeSelector.NoneEnabledOn()
    gadNodeSelector.ShowHiddenOn()
    gadNodeSelector.SetParent(moduleFrame.GetFrame())
    gadNodeSelector.Create()
    gadNodeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
    gadNodeSelector.UpdateMenu()
    gadNodeSelector.SetBorderWidth(2)
    gadNodeSelector.SetLabelText("GAD Parameters")
    gadNodeSelector.SetBalloonHelpString("select a GAD node from the current mrml scene.")
    Slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % gadNodeSelector.GetWidgetName())

    conductanceScale.SetParent(moduleFrame.GetFrame())
    conductanceScale.SetLabelText("Conductance")
    conductanceScale.Create()
    w = conductanceScale.GetScale().GetWidth()
    conductanceScale.SetRange(0,10)
    conductanceScale.SetResolution(0.1)
    conductanceScale.SetValue(1.0) 
    Slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % conductanceScale.GetWidgetName())

    timeStepScale.SetParent(moduleFrame.GetFrame())
    timeStepScale.SetLabelText("Time Step")
    timeStepScale.Create()
    timeStepScale.GetScale().SetWidth(w)
    timeStepScale.SetRange(0.0,1.0)
    timeStepScale.SetValue(0.1)
    timeStepScale.SetResolution(0.01)
    Slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % timeStepScale.GetWidgetName())

    numberOfIterationsScale.SetParent(moduleFrame.GetFrame())
    numberOfIterationsScale.SetLabelText("Iterations")
    numberOfIterationsScale.Create()
    numberOfIterationsScale.GetScale().SetWidth(w)
    numberOfIterationsScale.SetValue(1)
    Slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % numberOfIterationsScale.GetWidgetName())

    volumeSelector.SetNodeClass("vtkMRMLScalarVolumeNode", "", "", "")
    volumeSelector.SetParent(moduleFrame.GetFrame())
    volumeSelector.Create()
    volumeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
    volumeSelector.UpdateMenu()
    volumeSelector.SetBorderWidth(2)
    volumeSelector.SetLabelText("Input Volume: ")
    volumeSelector.SetBalloonHelpString("select an input volume from the current mrml scene.")
    Slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % volumeSelector.GetWidgetName())

    outVolumeSelector.SetNodeClass("vtkMRMLScalarVolumeNode", "", "", "GADVolumeOut")
    outVolumeSelector.SetNewNodeEnabled(1)
    outVolumeSelector.SetParent(moduleFrame.GetFrame())
    outVolumeSelector.Create()
    outVolumeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
    outVolumeSelector.UpdateMenu()
    outVolumeSelector.SetBorderWidth(2)
    outVolumeSelector.SetLabelText("Output Volume: ")
    outVolumeSelector.SetBalloonHelpString("select an output volume from the current mrml scene.")
    Slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4" % outVolumeSelector.GetWidgetName())

    applyButton.SetParent(moduleFrame.GetFrame())
    applyButton.Create()
    applyButton.SetText("Apply")
    applyButton.SetWidth(8)
    Slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 10" % applyButton.GetWidgetName())

    moduleFrame.Delete()


def TearDownGUI(self):

    gadNodeSelector.SetParent("")
    conductanceScale.SetParent("")
    timeStepScale.SetParent("")
    numberOfIterationsScale.SetParent("")
    volumeSelector.SetParent("")
    outVolumeSelector.SetParent("")
    applyButton.SetParent("")

    if self.GetUIPanel().GetUserInterfaceManager():
        pageWidget = self.GetUIPanel().GetPageWidget("GradientAnisotropicDiffusionFilter")
        self.GetUIPanel().RemovePage("GradientAnisotropicDiffusionFilter")

