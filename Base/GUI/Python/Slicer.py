import Tkinter
import Console
import string
from __main__ import tk
import _slicer

def StartConsole():
    top = Tkinter.Toplevel ( tk );
    c = Console.Console(parent=top,dict={})
    c.dict["console"] = c
    c.pack(fill=Tkinter.BOTH, expand=1)
    c.master.title("Python Console v%s" % Console.VERSION)
    return

def TkCall(commandString):
    tk.tk.call(*commandString.split())

class SlicerWrapper:
    def ToArray (self):
        try:
            return _slicer.vtkImageDataToArray ( tk.tk.interpaddr(), str(self) )
        except Exception, e:
            print e
    def __repr__ ( self ):
        return "<Slicer object: " + self.obj + ">"
    def __str__ ( self ):
        return self.__repr__ ()
    def __init__ ( self, slicer, obj ):
        self.obj = obj
        self.slicer = slicer
    def __convertString ( self, inString ):
        # print "inString: ", inString
        outList = []
        IsList = True
        AreFloats = None
        try:
            for ii in string.split ( inString ):
                # print ii
                foo = eval ( ii )
                if type ( foo ) == int or type ( foo ) == float:
                    if type ( foo ) == float:
                        AreFloats = True
                    outList.append ( foo )
                else:
                    IsList = None
                    break
            if IsList:
                if AreFloats:
                    outList = [float ( ii ) for ii in outList]
                if len ( outList ) == 1:
                    return outList[0]
                return outList
            else:
                return inString
        except Exception:
            pass
        return inString
    def __eval ( self, inString ):
        # print "calling: ", inString
        result = tk.tk.call ( *string.split ( inString ) )
        return self.__convertString ( result )
    def __callVTKmethod ( self, m, *a ):
        cstring = []
        cstring.append ( str(self.obj) )
        cstring.append ( str(m) )
        for idx in range(len(a)):
            cstring.append ( str(a[idx]) )
        value = tk.tk.call ( *cstring )
        if tk.tk.call ( 'info', 'command', value ):
            return SlicerWrapper ( self, value )
        else:
            return self.__convertString ( value )
    def __repr__ ( self ):
        return str ( self.obj )
    def __getattr__ ( self, name ):
        """Returns a function object suitable for calling the wrapped function"""
        return lambda *a: self.__callVTKmethod ( str(name), *a )
    def __eq__ ( self, other ):
        """Allows for equality testing"""
        if other is None:
            return False
        return (other.__class__ is self.__class__ and other.__dict__ == self.__dict__)
    def __ne__ ( self, other ):
        """Allows for equality testing"""
        if other is None:
            return True
        return not (other.__class__ is self.__class__ and other.__dict__ == self.__dict__)



class Slicer:
    """Main slicer object, responsible for fetching things out of the ::slicer3 namespace in the Tcl interp"""
    def __init__ ( self ):
        self.callTk = tk.tk.call
        self.ns = "::slicer3"
    def eval ( self, inString ):
        return self.__eval ( inString )
    def __eval ( self, inString ):
        # print inString
        return self.callTk ( *string.split ( inString ) )
    def __getattr__ ( self, inName ):
        # Get the variable or command from the namespace
        qname = self.ns + "::" + inName
        for name in ( inName, qname ):
            # if it's a variable, find the value, and see if it's a command
            if self.__eval ( 'info exists ' + name ):
                # print "found name: ", name
                cname = str ( self.__eval ( 'set ' + name ) );
                # print "found cname: ", cname
                if self.__eval ( 'info command ' + cname ):
                    # print "Returning Wrapped object: ", cname
                    return SlicerWrapper ( self, cname )
                else:
                    # print "Returning object: ", cname
                    return cname
            if self.__eval ( 'info command ' + name ):
                # print "Returning Wrapped object, wo/lookup"
                return SlicerWrapper ( self, name )
        raise Exception ( "attribute " + qname + " does not exist" )

class Plugin:
    """Class to interface with Slicer3 plugins"""
    def __init__ ( self, Name ):
        idx = GetRegisteredPlugins().index ( Name );
        self.slicer = Slicer();
        self.name = Name
    def Execute ( self, *args, **keywords ):
        self.module = self.slicer.MRMLScene.CreateNodeByClass ( "vtkMRMLCommandLineModuleNode" );
        self.module.SetModuleDescription ( self.name )
        pargs = self.__FindPositionalArguments()
        diff = len(pargs) - len(args)
        if diff < 0:
            self.module.Delete();
            raise Exception ( "Plugin: " + self.name + " requires " + len ( pargs ) + ", " + len ( args ) + " given" )

        print diff

        arglen = len(args);

        # Set the positional arguments
        for ii in range ( len(args) ):
            # Make sure we can lookup a MRML Node
            n = self.slicer.MRMLScene.GetNodeByID ( args[ii] )
            if n == []:
                self.module.Delete();
                raise Exception ( "Plugin: " + self.name + " requires a MRML Node as a positional arg: found " + str(args[ii]) + " instead" )
            paramName = self.module.GetParameterName ( pargs[ii][0], pargs[ii][1] );
            self.module.SetParameterAsString ( paramName, args[ii] );

        # Append empty nodes to the end...
        outputNodes = []
        newargs = list ( args )
        for ii in range ( diff ):
            # Check type
            idx = arglen + ii
            t = self.module.GetParameterTag ( pargs[ii][0], pargs[ii][1] )
            if t == "label":
                c = "vtkMRMLScalarVolumeNode";
            elif t == "vector":
                c = "vtkMRMLVectorVolumeNode"
            elif t == "tensor":
                c = "vtkMRMLDiffusionTensorVolumeNode"
            elif t == "diffusion-weighted":
                c = "vtkMRMLDiffusionWeightedVolumeNode"
            else:
                c = "vtkMRMLScalarVolumeNode"
            node = self.slicer.MRMLScene.CreateNodeByClass ( c )
            node.SetScene ( self.slicer.MRMLScene )
            node.SetName ( self.slicer.MRMLScene.GetUniqueNameByString ( c ) );
            node = self.slicer.MRMLScene.AddNode ( node );
            newargs.append ( node );
            outputNodes.append ( node );
            paramName = self.module.GetParameterName ( pargs[idx][0], pargs[idx][1] );
            print 'Setting: ' + paramName + ' to ' + node.GetName()
            self.module.SetParameterAsString ( paramName, node.GetID() );

        # Now set the keyword args
        for key in keywords.keys():
            print 'Setting: ' + str(key) + ' = ' + str(keywords[key])
            self.module.SetParameterAsString ( key, str(keywords[key]) );

        # And finally, execute the plugin
        logic = self.slicer.vtkCommandLineModuleLogic.New()
        logic.SetAndObserveMRMLScene ( self.slicer.MRMLScene )
        logic.SetApplicationLogic ( self.slicer.ApplicationGUI.GetApplicationLogic() )
        logic.SetTemporaryDirectory ( self.slicer.Application.GetTemporaryDirectory() )
        print "Apply and Wait"
        logic.ApplyAndWait ( self.module )

        status = self.module.GetStatusString()
        if status != 'Completed':
            raise Exception ( "Plugin faild with status: " + status )
        
        # Else return sucessfully!
        return outputNodes

    def __FindPositionalArguments ( self ):
        """Find and return a list of (group,arg) tuples of the positional arguments"""
        args = {}
        for group in range ( self.module.GetNumberOfParameterGroups() ):
            for arg in range ( self.module.GetNumberOfParametersInGroup ( group ) ):
                print self.module.GetParameterIndex ( group, arg );
                if self.module.GetParameterIndex ( group, arg ) != []:
                    print self.module.GetParameterIndex ( group, arg )
                    args[int(self.module.GetParameterIndex ( group, arg ))] = (group,arg)
        keys = args.keys();
        keys.sort();
        print keys
        print args
        return args

def TestPluginClass():
    slicer = Slicer();
    p = Plugin ( 'Subtract Images' );
    vn = ListVolumeNodes ();
    if len ( vn ) > 0:
        p.Execute ( vn[0].GetID(), vn[0].GetID() );


def GetRegisteredPlugins ():
    slicer = Slicer();
    n = slicer.MRMLScene.CreateNodeByClass ( "vtkMRMLCommandLineModuleNode" );
    p = [];
    for idx in range ( n.GetNumberOfRegisteredModules() ):
        p.append ( n.GetRegisteredModuleNameByIndex ( idx ) );
    n.Delete();
    return p


def CallPlugin ( name, *args, **keywords ):
    # get a slicer guy
    slicer = Slicer();
    n = slicer.MRMLScene.CreateNodeByClass ( "vtkMRMLCommandLineModuleNode" );
    # Figure out if this is a valid plugin
    validname = None
    for idx in range ( n.GetNumberOfRegisteredModules() ):
        if n.GetRegisteredModuleNameByIndex ( idx ) == name:
            validname = true
            break
    if not validname:
        n.Delete()
        raise "Could not find a Command Line Module named: " + str ( name )
    n.SetModuleDescription ( name )
    


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
    slicer = Slicer()
    scene = slicer.MRMLScene
    count = scene.GetNumberOfNodesByClass ( 'vtkMRMLVolumeNode' )
    for idx in range ( int ( count ) ):
        nodes[idx] = scene.GetNthNodeByClass ( idx, 'vtkMRMLVolumeNode' )
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

