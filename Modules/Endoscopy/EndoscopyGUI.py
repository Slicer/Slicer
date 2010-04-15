from SlicerScriptedModule import ScriptedModuleGUI
from Slicer import slicer

vtkSlicerNodeSelectorWidget_NodeSelectedEvent = 11000
vtkKWPushButton_InvokedEvent = 10000

class EndoscopyGUI(ScriptedModuleGUI):

    def __init__(self):
        ScriptedModuleGUI.__init__(self)
        self.vtkScriptedModuleGUI.SetCategory("Endoscopy")
        self.FiducialsNodeSelector = slicer.vtkSlicerNodeSelectorWidget()
        self.CameraNodeSelector = slicer.vtkSlicerNodeSelectorWidget()
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

        inputCamera = self.CameraNodeSelector .GetSelected()
        if not inputCamera:
          self.ErrorDialog("No camera selected")
          return

        inputSeeds = self.FiducialsNodeSelector .GetSelected()
        if not inputSeeds:
          self.ErrorDialog("No fiducials selected")
          return

        self.Status("Calculating Path...")

        p = path(0.5, fidListName=inputSeeds.GetName(), cameraName = inputCamera.GetName())
        p.pathModel()
        p.gui()

        self.Status("Done calculating path.")

    def BuildGUI(self):
        self.GetUIPanel().AddPage("Endoscopy","Endoscopy","")
        pageWidget = self.GetUIPanel().GetPageWidget("Endoscopy")
        helpText = "Create a path model as a spline interpolation of a set of fiducial points.  See <a>http://www.slicer.org/slicerWiki/index.php/Modules:Endoscopy-Documentation-3.6</a> for more information.\n\nPick the Camera to be modified by the path and the Fiducial List defining the control points.  Clicking Apply will bring up the flythrough panel.\n\nYou can manually scroll though the path with the Frame slider.\n\nThe Play/Pause button toggles animated flythrough.\n\nThe Frame Skip slider speeds up the animation by skipping points on the path.\n\nThe Frame Delay slider slows down the animation by adding more time between frames.\n\nThe View Angle provides is used to approximate the optics of an endoscopy system.\n\nThe Close button dismisses the flyrhough panel and stops the animation."
        aboutText = "This work is supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See <a>http://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy</a>) NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.  Module implemented by Steve Pieper."
        self.BuildHelpAndAboutFrame(pageWidget,helpText,aboutText)
    
        moduleFrame = slicer.vtkSlicerModuleCollapsibleFrame()
        moduleFrame.SetParent(self.GetUIPanel().GetPageWidget("Endoscopy"))
        moduleFrame.Create()
        moduleFrame.SetLabelText("Endoscopy")
        moduleFrame.ExpandFrame()
        widgetName = moduleFrame.GetWidgetName()
        pageWidgetName = self.GetUIPanel().GetPageWidget("Endoscopy").GetWidgetName()
        slicer.TkCall("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s" % (widgetName,pageWidgetName))

        self.CameraNodeSelector.SetNodeClass("vtkMRMLCameraNode","","","")
        self.CameraNodeSelector.SetParent(moduleFrame.GetFrame())
        self.CameraNodeSelector.Create()
        self.CameraNodeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self.CameraNodeSelector.UpdateMenu()
        self.CameraNodeSelector.SetBorderWidth(2)
        self.CameraNodeSelector.SetLabelText("Camera: ")
        self.CameraNodeSelector.SetBalloonHelpString("select a camera that will fly along this path.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4 -expand true -fill x" % self.CameraNodeSelector.GetWidgetName())
    
        self.FiducialsNodeSelector.SetNodeClass("vtkMRMLFiducialListNode","","","")
        self.FiducialsNodeSelector.SetParent(moduleFrame.GetFrame())
        self.FiducialsNodeSelector.Create()
        self.FiducialsNodeSelector.SetMRMLScene(self.GetLogic().GetMRMLScene())
        self.FiducialsNodeSelector.UpdateMenu()
        self.FiducialsNodeSelector.SetBorderWidth(2)
        self.FiducialsNodeSelector.SetLabelText("Input Fiducials: ")
        self.FiducialsNodeSelector.SetBalloonHelpString("select a fiducial list to define control points for the path.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 4 -expand true -fill x" % self.FiducialsNodeSelector.GetWidgetName())
    
        self.ApplyButton.SetParent(moduleFrame.GetFrame())
        self.ApplyButton.Create()
        self.ApplyButton.SetText("Apply")
        self.ApplyButton.SetWidth(8)
        self.ApplyButton.SetBalloonHelpString("create the path and show the fly through controller.")
        slicer.TkCall("pack %s -side top -anchor e -padx 20 -pady 10" % self.ApplyButton.GetWidgetName())

    def TearDownGUI(self):
        self.FiducialsNodeSelector.SetParent(None)
        self.FiducialsNodeSelector = None
        self.ApplyButton.SetParent(None)
        self.ApplyButton = None
   
        if self.GetUIPanel().GetUserInterfaceManager():
            pageWidget = self.GetUIPanel().GetPageWidget("Endoscopy")
            self.GetUIPanel().RemovePage("Endoscopy")




"""
fly a slicer camera along a path defined by a fiducial list.
Hermite spline interpolation notation follows:
http://en.wikipedia.org/wiki/Cubic_Hermite_spline

execfile('../Slicer3/Modules/fly.py'); p = path(0.5); p.pathModel(); p.fly()

execfile('../Slicer3/Modules/Endoscopy/EndoscopyGUI.py'); p = path(0.5); p.pathModel(); p.gui()

import Endoscopy; p = Endoscopy.path(0.5); p.pathModel(); p.gui()
"""


import Slicer
import numpy
import numpy.linalg


class path (object):

  def __init__(self, dl=0.5, fidListName='L', cameraName='Camera'):
    self.dl = dl # desired world space step size (in mm)
    self.dt = dl # current guess of parametric stepsize
    nodes = Slicer.slicer.ListNodes()
    self.fids = nodes[fidListName]
    self.camera = nodes[cameraName].GetCamera()

    # hermite interpolation functions
    self.h00 = lambda t: 2*t**3 - 3*t**2     + 1
    self.h10 = lambda t:   t**3 - 2*t**2 + t
    self.h01 = lambda t:-2*t**3 + 3*t**2
    self.h11 = lambda t:   t**3 -   t**2

    # n is the number of control points in the piecewise curve
    self.n = self.fids.GetNumberOfFiducials()
    n = self.n

    # get control point data
    # sets self.p
    self.p = numpy.zeros((n,3))
    for i in xrange(n):
      self.p[i] = self.fids.GetNthFiducialXYZ(i)

    # calculate the tangent vectors
    # - fm is forward difference
    # - m is average of in and out vectors
    # - first tangent is out vector, last is in vector
    # - sets self.m
    fm = numpy.zeros((n,3))
    for i in xrange(0,n-1):
      fm[i] = self.p[i+1] - self.p[i]
    self.m = numpy.zeros((n,3))
    for i in xrange(1,n-1):
      self.m[i] = (fm[i-1] + fm[i]) / 2.
    self.m[0] = fm[0]
    self.m[n-1] = fm[n-2]

    self.path = [self.p[0]]
    self.calculatePath()


  def calculatePath(self):
    """ generate a flight path for of steps of length dl
    """

    #
    # calculate the actual path
    # - take steps of self.dl in world space
    # -- if dl steps into next segment, take a step of size "remainder" in the new segment
    # - put resulting points into self.path
    #
    n = self.n
    segment = 0 # which first point of current segment
    t = 0 # parametric current parametric increment
    remainder = 0 # how much of dl isn't included in current step
    while segment < n-1:
      t, p, remainder = self.step(segment, t, self.dl)
      if remainder != 0 or t == 1.:
        segment += 1
        t = 0
        if segment < n-1:
          t, p, remainder = self.step(segment, t, remainder)
      self.path.append(p)

  def point(self,segment,t):
    return (self.h00(t)*self.p[segment] + 
              self.h10(t)*self.m[segment] + 
              self.h01(t)*self.p[segment+1] + 
              self.h11(t)*self.m[segment+1])

  def step(self,segment,t,dl):
    """ take a step of dl and return the path point and new t
      return:
      t = new parametric coordinate after step 
      p = point after step
      remainder = if step results in parametic coordinate > 1.0, then
        this is the amount of world space not covered by step
    """
    p0 = self.path[self.path.__len__() - 1] # last element in path
    remainder = 0
    ratio = 100
    count = 0
    while abs(1. - ratio) > 0.05:
      t1 = t + self.dt
      pguess = self.point(segment,t1)
      dist = numpy.linalg.norm(pguess - p0)
      ratio = self.dl / dist
      self.dt *= ratio
      if self.dt < 0.00000001:
        return
      count += 1
      if count > 500:
        return (t1, pguess, 0)
    if t1 > 1.:
      t1 = 1.
      p1 = self.point(segment, t1)
      remainder = numpy.linalg.norm(p1 - pguess)
      pguess = p1
    return (t1, pguess, remainder)



  def flyto(self,f):
    """ apply the fth step in the path to the global camera
    """
    p = self.path[f]
    self.camera.SetPosition(*p)
    foc = self.path[f+1]
    self.camera.SetFocalPoint(*foc)
    self.transform.GetMatrixTransformToParent().SetElement(0,3,p[0])
    self.transform.GetMatrixTransformToParent().SetElement(1,3,p[1])
    self.transform.GetMatrixTransformToParent().SetElement(2,3,p[2])
    self.camera.SetViewAngle(self.fov.get())

  def fly(self):
    frames = self.path.__len__() - 1
    for f in xrange(frames):
      self.flyto(f)
      Slicer.tk.after(20)
      Slicer.tk.update()

  def pathModel(self):

    #
    # create a vtkPolyData for a polyline
    # - add one point per path point
    # - add a single polyline
    #
    vtk = Slicer.slicer
    self.points = vtk.vtkPoints()
    self.polyData = vtk.vtkPolyData()
    self.polyData.SetPoints(self.points)

    self.lines = vtk.vtkCellArray()
    self.polyData.SetLines(self.lines)
    self.linesIDArray = self.lines.GetData()
    self.linesIDArray.Reset()
    self.linesIDArray.InsertNextTuple1(0)

    self.polygons = vtk.vtkCellArray()
    self.polyData.SetPolys( self.polygons )
    idArray = self.polygons.GetData()
    idArray.Reset()
    idArray.InsertNextTuple1(0)

    for point in self.path:
      pointIndex = self.points.InsertNextPoint(*point)
      self.linesIDArray.InsertNextTuple1(pointIndex)
      self.linesIDArray.SetTuple1( 0, self.linesIDArray.GetNumberOfTuples() - 1 )
      self.lines.SetNumberOfCells(1)

    # path
    # create model node
    self.model = vtk.vtkMRMLModelNode()
    self.model.SetScene(Slicer.slicer.MRMLScene)
    self.model.SetName("Path-%s" % self.fids.GetName())
    self.model.SetAndObservePolyData(self.polyData)

    # create display node
    self.modelDisplay = vtk.vtkMRMLModelDisplayNode()
    self.modelDisplay.SetColor(1,1,0) # yellow
    self.modelDisplay.SetScene(Slicer.slicer.MRMLScene)
    Slicer.slicer.MRMLScene.AddNodeNoNotify(self.modelDisplay)
    self.model.SetAndObserveDisplayNodeID(self.modelDisplay.GetID())

    # add to scene
    self.modelDisplay.SetPolyData(self.model.GetPolyData())
    Slicer.slicer.MRMLScene.AddNode(self.model)

    # camera cursor
    self.sphere = vtk.vtkSphereSource()
    self.sphere.Update()
    # create model node
    self.cursor = vtk.vtkMRMLModelNode()
    self.cursor.SetScene(Slicer.slicer.MRMLScene)
    self.cursor.SetName("Cursor-%s" % self.fids.GetName())
    self.cursor.SetAndObservePolyData(self.sphere.GetOutput())

    # create display node
    self.cursorModelDisplay = vtk.vtkMRMLModelDisplayNode()
    self.cursorModelDisplay.SetColor(1,0,0) # red
    self.cursorModelDisplay.SetScene(Slicer.slicer.MRMLScene)
    Slicer.slicer.MRMLScene.AddNodeNoNotify(self.cursorModelDisplay)
    self.cursor.SetAndObserveDisplayNodeID(self.cursorModelDisplay.GetID())

    # add to scene
    self.cursorModelDisplay.SetPolyData(self.sphere.GetOutput())
    Slicer.slicer.MRMLScene.AddNode(self.cursor)

    # create transform node
    self.transform = Slicer.slicer.vtkMRMLLinearTransformNode()
    self.transform.SetName('Transform-%s' % self.fids.GetName())
    Slicer.slicer.MRMLScene.AddNode(self.transform)
    self.cursor.SetAndObserveTransformNodeID(self.transform.GetID())


  # GUI-related methods

  def scaleCB(self,value):
    self.flyto(self.frame.get())

  def flyCB(self):
    if self.playing:
      f = self.frame.get()
      f += 1 + self.skip.get()
      if f > self.frame['to']:
        f = 0
      self.frame.set(f)
      self.afterID = Slicer.tk.after(self.delay.get(),self.flyCB)


  def playCB(self):
    if self.playing:
      self.play.configure(text="Play", relief="raise")
      self.playing = False
    else:
      self.play.configure(text="Pause", relief="sunken")
      self.playing = True
      self.flyCB()

  def closeCB(self):
    Slicer.tk.after_cancel(self.afterID)
    self.toplevel.destroy()

  def gui(self):
    Tkinter = Slicer.Tkinter
    # toplevel window
    self.toplevel = Tkinter.Toplevel()
    self.toplevel.title("Flythrough")
    self.toplevel.wm_geometry("400x300")
    # frame slider
    self.frame = Tkinter.Scale(self.toplevel, 
                                orient='horizontal', to=self.path.__len__() - 2, 
                                label="Frame", command=self.scaleCB)
    self.frame.pack(side='top', expand='true', fill='x')
    # play/pause button
    self.playing = False
    self.play = Tkinter.Button(self.toplevel, text="Play", command=self.playCB)
    self.play.pack(side='top')
    # Skip slider
    self.skip = Tkinter.Scale(self.toplevel, orient='horizontal', from_=0, to=10, label="Frame Skip")
    self.skip.set(0)
    self.skip.pack(side='top', expand='true', fill='x')
    # Delay slider
    self.delay = Tkinter.Scale(self.toplevel, orient='horizontal', from_=5, to=100, label="Frame Delay (milliseconds)")
    self.delay.set(10)
    self.delay.pack(side='top', expand='true', fill='x')
    # fov slider
    self.fov = Tkinter.Scale(self.toplevel, orient='horizontal', from_=30, to=170, label="View Angle")
    self.fov.pack(side='top', expand='true', fill='x')
    
    # close
    self.close = Tkinter.Button(self.toplevel, text="Close", command=self.closeCB)
    self.close.pack(side='bottom')


