import Tkinter
import Console
import string
from __main__ import tk
import _slicer

def StartConsole():
    top = Tkinter.Toplevel ( tk )
    c = Console.Console(parent=top,dict={})
    c.dict["console"] = c
    c.pack(fill=Tkinter.BOTH, expand=1)
    c.master.title("Python Console v%s" % Console.VERSION)
    return

def TkCall(commandString):
    tk.tk.call(*commandString.split())

class SlicerWrapper(object):

    def __init__ (self, obj):
        self.obj = obj

    def __repr__ ( self ):
        return "<Slicer object: " + self.obj + ">"

    def __str__ ( self ):
        return self.__repr__()

    def __convertString (self, inString):
        outList = []
        isList = True
        areFloats = None
        try:
            for ii in inString.split():
                foo = eval(ii)
                if type(foo) == int or type(foo) == float:
                    if type(foo) == float:
                        areFloats = True
                    outList.append(foo)
                else:
                    isList = None
                    break
            if isList:
                if areFloats:
                    outList = [float(ii) for ii in outList]
                if len(outList) == 1:
                    return outList[0]
                return outList
            else:
                return inString
        except Exception:
            pass
        return inString

    def __eval (self, inString):
        result = tk.tk.call(*string.split(inString))
        return self.__convertString(result)

    def __callVTKmethod (self, m, *a):
        if m == 'New':
            cstring = []
            cstring.append(str(self.obj))
            cstring.append(str(m))
            value = tk.tk.call(*cstring)
            if tk.tk.call('info','command',value):
               return SlicerWrapper(value)
        cstring = []
        cstring.append(str(self.obj))
        cstring.append(str(m))
        for arg in a:
            if arg == None:
                arg = ""
            cstring.append(str(arg))
        value = tk.tk.call(*cstring)
        if tk.tk.call('info','command',value):
            if not eval(tk.tk.eval('catch "%s GetClassName" res' % value)):
                return GetPythonWrapper(value)
            else:
                return self.__convertString(value)
        else:
            return self.__convertString(value)

    def __repr__ (self):
        return str(self.obj)

    def __getattr__ (self, name):
        """Returns a function object suitable for calling the wrapped function"""
        return lambda *a: self.__callVTKmethod(str(name),*a)


from new import classobj

def __init(self, tclName=None):
    if self.__class__.__name__ != self.__class__.ClassName:
        raise Exception, "Subclassing wrapped classes is not allowed."
    if not tclName:
        self.SlicerWrapper = SlicerWrapper(self.__class__.ClassName).New()
        slicer.addInstance(self.SlicerWrapper)
        self.OwnWrapper = True
    else:
        self.SlicerWrapper = SlicerWrapper(tclName)
        self.OwnWrapper = False

def __repr(self):
    return self.SlicerWrapper.Print()

def __del(self):
#   TODO: need to call superclass' __del__?
# TODO: I'm not convinced this gets called... (SP)
    if self.OwnWrapper:
        self.SlicerWrapper.Delete()

def __getTclName(self):
    return self.SlicerWrapper.obj

def __convertArgumentList(self,a):
    aTcl = []
    for arg in a:
        if hasattr(arg,'__dict__') and arg.__dict__.has_key('GetTclName') != -1:
            arg = arg.GetTclName()
        aTcl.append(arg)
    return aTcl

def __eq (self, other):
    if other is None:
        return False
    eq = False
    if self is other:
        eq = True
    if other.__class__ == self.__class__ and other.SlicerWrapper:
        if self.SlicerWrapper.obj == other.SlicerWrapper.obj:
            eq = True
    return eq

def __ne (self, other):
    if other is None:
        return True
    eq = False
    if self is other:
        eq = True
    if other.__class__ == self.__class__ and other.SlicerWrapper:
        if self.SlicerWrapper.obj == other.SlicerWrapper.obj:
            eq = True
    return not eq

def __listInstances(self):
    tclInstances = (tk.tk.eval("%s ListInstances" % self.__name__)).split()
    instances = []
    for tclInstance in tclInstances:
        instance = GetPythonWrapper(tclInstance)
        instances.append(instance)
    return instances

def __vtkImageData_ToArray (self):
    try:
        return _slicer.vtkImageDataToArray(tk.tk.interpaddr(), self.GetTclName())
    except Exception, e:
        print e
        raise

def __vtkImageData_FromArray (self,numpyArray,dimensionality=3):
    try:
        return _slicer.ArrayTovtkImageData(tk.tk.interpaddr(), numpyArray, dimensionality, self.GetTclName())
    except Exception, e:
        print e
        raise

def __vtkImageData_FromArray2D (self,numpyArray):
    return self.FromArray(numpyArray,2)

def __vtkImageData_FromArray3D (self,numpyArray):
    return self.FromArray(numpyArray,3)

def __vtkDataArray_ToArray (self):
    try:
        return _slicer.vtkDataArrayToArray(tk.tk.interpaddr(), self.GetTclName())
    except Exception, e:
        print e
        raise

def __vtkDataArray_FromArray (self,numpyArray,dimensionality=3):
    try:
        return _slicer.ArrayTovtkDataArray(tk.tk.interpaddr(), numpyArray, self.GetTclName())
    except Exception, e:
        print e
        raise

def CreateClass(name):
    if SlicerClassDict.has_key(name):
        return SlicerClassDict[name]
    fooName = 'foo'
    i = 0
    while tk.tk.eval('info exists %s%d' % (fooName,i)) == '1':
        i += 1
    fooName = '%s%d' % (fooName,i)
    tk.tk.eval('set %s [%s New]' % (fooName,name))
    methods = tk.tk.eval('$%s ListMethods' % fooName)
    tk.tk.eval('$%s Delete' % fooName)
    tk.tk.call(*string.split('unset %s' % fooName))
    methodLines = methods.split('\n')
    dirtyMethodNames = [classMethods.strip('\n').split('\n') for classMethods in methods.split('Methods from')][1:]
    classNames = [className.split()[2][:-1] for className in methodLines if className.find('Methods from') != -1]
    classDict = {}
    for dirtyClassMethodNames in dirtyMethodNames:
        methodNames = [dirtyMethodName.strip().split('\t')[0] for dirtyMethodName in dirtyClassMethodNames[1:]]
        classDict[dirtyClassMethodNames[0][:-1].strip()] = methodNames
    SuperClass = object
    for className in classNames:
        if SlicerClassDict.has_key(className):
            SuperClass = SlicerClassDict[className]
            continue
        methodDict = {'__init__':__init, '__del__':__del, '__repr__':__repr, '__eq__':__eq, '__ne__':__ne, '__convertArgumentList': __convertArgumentList, 'GetTclName': __getTclName, 'ListInstances': __listInstances }
        if className == 'vtkImageData':
            methodDict['ToArray'] = __vtkImageData_ToArray
            methodDict['FromArray'] = __vtkImageData_FromArray
            methodDict['FromArray2D'] = __vtkImageData_FromArray2D
            methodDict['FromArray3D'] = __vtkImageData_FromArray3D
        if className == 'vtkDataArray':
            methodDict['ToArray'] = __vtkDataArray_ToArray
            methodDict['FromArray'] = __vtkDataArray_FromArray
        for methodName in classDict[className]:
            if methodName in ('New','Delete'):
                continue
            ownWrapperLine = ''
            if methodName in ('NewInstance','CreateNodeByClass','CreateInstance','CreateNodeInstance'):
                ownWrapperLine = 'value.OwnWrapper = True;'
            exec('def %s(self,*a): aTcl = self.__convertArgumentList(a); value = self.SlicerWrapper.__getattr__("%s")(*aTcl); %s return value' % (methodName,methodName,ownWrapperLine))
            exec('methodDict["%s"] = %s' % (methodName,methodName))
        global ClassObj
        ClassObj = classobj(className,(SuperClass,),methodDict)
        ClassObj.ClassName= className
        ClassObj.ListInstances = classmethod(ClassObj.ListInstances.im_func)
        SlicerClassDict[className] = ClassObj
        SuperClass = ClassObj
    return SuperClass

def GetPythonWrapper(tclName):
    className = tk.tk.eval("%s GetClassName" % tclName)
    return CreateClass(className)(tclName)

def Test():
    a = slicer.vtkSmoothPolyDataFilter()
    b = slicer.vtkPolyData()
    print 'a ', a.SlicerWrapper.obj
    print 'b ', b.SlicerWrapper.obj
    a.SetInput(b)
    print 'a.SetInput ', a.GetInput().SlicerWrapper.obj
    print b == a.GetInput()

SlicerClassDict = {}

class Slicer(object):
    """Main slicer object, responsible for fetching things out of the ::slicer3 namespace in the Tcl interp"""

    def __init__ (self):
        self.callTk = tk.tk.call
        self.ns = "::slicer3"
        self.instances = []

    def eval (self, inString):
        return self.__eval(inString)

    def __eval (self, inString):
        return self.callTk(*string.split(inString))

    def addInstance(self,instance):
        self.instances.append(instance)

    # deleteInstances should be called by slicer before Py_Finalize() to unreference
    # all the instances created here
    def deleteInstances(self):
        for i in self.instances:
            tk.tk.eval("%s Delete" % i)

    def __getattr__ (self, inName):
        # Get the variable or command from the namespace
        qname = self.ns + "::" + inName
        for name in (inName, qname):
            # if it's a variable, find the value, and see if it's a command
            if self.__eval('info exists ' + name):
                cname = str(self.__eval('set ' + name))
                if self.__eval('info command ' + cname):
                    if not eval(tk.tk.eval('catch "%s GetClassName" res' % cname)):
                        return GetPythonWrapper(cname)
                    else:
                        return cname
                else:
                    return cname
            if self.__eval('info command '+name):
                return CreateClass(name)
        raise Exception("attribute " + qname + " does not exist")

    @classmethod
    def TkCall(self,commandString):
        tk.tk.call(*commandString.split())

    def ListVolumeNodes(self):
        return ListVolumeNodes()


slicer = Slicer()





class Plugin(object):
    """Class to interface with Slicer3 plugins"""
    def __init__ (self, name):
        idx = GetRegisteredPlugins().index(name)
        self.name = name
    def Execute ( self, *args, **keywords ):
        self.module = slicer.MRMLScene.CreateNodeByClass("vtkMRMLCommandLineModuleNode")
        self.module.SetModuleDescription(self.name)
        pargs = self.__FindPositionalArguments()
        diff = len(pargs) - len(args)
        if diff < 0:
            self.module = None
            raise Exception ( "Plugin: " + self.name + " requires " + str(len(pargs)) + "arguments, " + str(len(args)) + " given" )

        arglen = len(args)

        # Set the positional arguments
        for ii in range(len(args)):
            # Make sure we can lookup a MRML Node
            n = slicer.MRMLScene.GetNodeByID(args[ii])
            if n == []:
                self.module = None
                raise Exception("Plugin: " + self.name + " requires a MRML Node as a positional arg: found " + str(args[ii]) + " instead")
            paramName = self.module.GetParameterName(pargs[ii][0],pargs[ii][1])
            self.module.SetParameterAsString(paramName,args[ii])

        # Append empty nodes to the end...
        outputNodeIDs = []
        newargs = list(args)
        for ii in range(diff):
            # Check type
            idx = arglen + ii
            t = self.module.GetParameterTag(pargs[ii][0],pargs[ii][1])
            if t == "label":
                c = "vtkMRMLScalarVolumeNode"
            elif t == "vector":
                c = "vtkMRMLVectorVolumeNode"
            elif t == "tensor":
                c = "vtkMRMLDiffusionTensorVolumeNode"
            elif t == "diffusion-weighted":
                c = "vtkMRMLDiffusionWeightedVolumeNode"
            else:
                c = "vtkMRMLScalarVolumeNode"
            node = slicer.MRMLScene.CreateNodeByClass(c)
            node.SetScene(slicer.MRMLScene)
            node.SetName(slicer.MRMLScene.GetUniqueNameByString(c))
            slicer.MRMLScene.AddNode(node)
            newargs.append(node)
            outputNodeIDs.append(node.GetID())
            paramName = self.module.GetParameterName(pargs[idx][0],pargs[idx][1])
            print 'Setting: ' + paramName + ' to ' + node.GetName()
            self.module.SetParameterAsString(paramName,node.GetID())

        # Now set the keyword args
        for key in keywords.keys():
            print 'Setting: ' + str(key) + ' = ' + str(keywords[key])
            self.module.SetParameterAsString(key,str(keywords[key]))

        # And finally, execute the plugin
        logic = slicer.vtkCommandLineModuleLogic()
        logic.SetAndObserveMRMLScene(slicer.MRMLScene)
        logic.SetApplicationLogic(slicer.ApplicationGUI.GetApplicationLogic())
        logic.SetTemporaryDirectory(slicer.Application.GetTemporaryDirectory())
        print "Apply and Wait"
        logic.ApplyAndWait(self.module)

        status = self.module.GetStatusString()
        if status != 'Completed':
            raise Exception ( "Plugin faild with status: " + status )
       
        # Else return sucessfully!
        return outputNodeIDs

    def __FindPositionalArguments ( self ):
        """Find and return a list of (group,arg) tuples of the positional arguments"""
        args = {}
        for group in range(self.module.GetNumberOfParameterGroups()):
            for arg in range(self.module.GetNumberOfParametersInGroup(group)):
                print self.module.GetParameterIndex(group,arg)
                if self.module.GetParameterIndex(group,arg) != []:
                    print self.module.GetParameterIndex(group,arg)
                    args[int(self.module.GetParameterIndex(group,arg))] = (group,arg)
        keys = args.keys()
        keys.sort()
        print keys
        print args
        return args

def TestPluginClass():
    p = Plugin ( 'Subtract Images' )
    vn = ListVolumeNodes()
    if len(vn) > 0:
        name = vn.keys()[0]
        p.Execute(vn[name].GetID(),vn[name].GetID())

def GetRegisteredPlugins ():
    n = slicer.MRMLScene.CreateNodeByClass('vtkMRMLCommandLineModuleNode')
    p = []
    for idx in range(n.GetNumberOfRegisteredModules()):
        p.append(n.GetRegisteredModuleNameByIndex(idx))
    return p


def CallPlugin ( name, *args, **keywords ):
    n = slicer.MRMLScene.CreateNodeByClass ( "vtkMRMLCommandLineModuleNode" )
    # Figure out if this is a valid plugin
    validname = None
    for idx in range(n.GetNumberOfRegisteredModules()):
        if n.GetRegisteredModuleNameByIndex(idx) == name:
            validname = True
            break
    if not validname:
        raise "Could not find a Command Line Module named: " + str(name)
    n.SetModuleDescription(name)
    


# To create a command line module node
# must be filled in with a ModuleDescriptionObject
# n = slicer.MRMLScene.CreateNodeByClass ( "vtkMRMLCommandLineModuleNode" )


# (RelWithDebInfo) 7 % [[$::slicer3::ApplicationGUI GetMainSliceGUI0]  GetSliceViewer]  GetWidgetName
# .vtkSlicerWindow3.vtkKWFrame12.vtkKWSplitFrame49.vtkKWFrame325.vtkSlicerSliceViewer378
# (RelWithDebInfo) 8 % 

def ListVolumeNodes():
    """Returns a dictionary containing the index and
    vtkMRMLVolumeNodes currently loaded by Slicer"""
    nodes = {}
    scene = slicer.MRMLScene
    count = scene.GetNumberOfNodesByClass ('vtkMRMLVolumeNode')
    for idx in range (count):
        node = scene.GetNthNodeByClass(idx,'vtkMRMLVolumeNode')
        nodes[node.GetName()] = node
    return nodes

def ParseArgs ( ModuleArgs, ArgTags, ArgFlags, ArgMultiples ):
    """This is a helper function to strip off all the flags
    and make them keyword args to the eventual Execute call
    returns a tuple of FlagArgs and PositionalArgs"""

    def CastArg (arg,argtag):
        if argtag == 'boolean':
            if arg == 'true':
                argval = True
            else:
                argval = False
        elif argtag == 'integer':
            argval = int(arg)
        elif argtag in ['float','double']:
            argval = float(arg)
        elif argtag == 'integer-vector':
            argval = [int(el) for el in arg.split(',')]
        elif argtag in ['float-vector', 'double-vector', 'point']:
            argval = [float(el) for el in arg.split(',')]
        elif argtag == 'string-vector':
            argval = arg.split(',')
        else:
            argval = arg
        return argval

    FlagArgs = {}
    PositionalArgs = []

    while len (ModuleArgs) != 0:
        arg = ModuleArgs.pop(0)
        print "Looking at: ", arg
        if arg.startswith ( "-" ):
            argflag = arg.lstrip("-")
            argtag = ArgTags[ArgFlags.index(argflag)]
            argmultiple = ArgMultiples[ArgFlags.index(argflag)]
            if argmultiple == 'true':
                if not FlagArgs.has_key(argflag):
                    FlagArgs[argflag] = []
                FlagArgs[argflag].append(CastArg(ModuleArgs.pop(0),argtag))
            else:
                FlagArgs[argflag] = CastArg(ModuleArgs.pop(0),argtag)
        else:
            nFlags = len([argflag for argflag in ArgFlags if argflag != ''])
            ArgTags = ArgTags[nFlags:]
            PositionalArgs.append(CastArg(arg,ArgTags.pop(0)))
            while len(ModuleArgs) != 0:
                PositionalArgs.append(CastArg(ModuleArgs.pop(0),ArgTags.pop(0)))

    return FlagArgs, PositionalArgs

