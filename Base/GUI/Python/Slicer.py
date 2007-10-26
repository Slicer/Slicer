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
                print "found name: ", name
                cname = str ( self.__eval ( 'set ' + name ) );
                # print "found cname: ", cname
                if self.__eval ( 'info command ' + cname ):
                    print "Returning Wrapped object: ", cname
                    return SlicerWrapper ( self, cname )
                else:
                    print "Returning object: ", cname
                    return cname
            if self.__eval ( 'info command ' + name ):
                print "Returning Wrapped object, wo/lookup"
                return SlicerWrapper ( self, name )
        raise Exception ( "attribute " + qname + " does not exist" )


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

def ParseArgs ( ModuleArgs, ArgTags ):
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
        elif argtag in ['float-vector', 'double-vector']:
            argval = [float(el) for el in arg.split(',')]
        elif argtag == 'string-vector':
            argval = arg.split(',')
        else:
            argval = arg
        return argval

    FlagArgs = {}
    PositionalArgs = []

    # Check each argument in turn, if we hit one that
    # does not start with a "-", it's the positional args.
    while len ( ModuleArgs ) != 0:
        arg = ModuleArgs.pop ( 0 );
        print "Looking at: ", arg
        if arg.startswith ( "-" ):
            FlagArgs[arg.lstrip( "-" )] = CastArg(ModuleArgs.pop(0),ArgTags.pop(0))
        else:
            PositionalArgs.append(CastArg(arg,ArgTags.pop(0)))
            while len ( ModuleArgs ) != 0:
                PositionalArgs.append(CastArg(ModuleArgs.pop(0),ArgTags.pop(0)))
    return FlagArgs, PositionalArgs
