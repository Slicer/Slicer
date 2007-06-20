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
        return self.obj
    def __init__ ( self, slicer, obj ):
        self.obj = obj
        self.slicer = slicer
    def __eval ( self, inString ):
        print inString
        return tk.tk.call ( *string.split ( inString ) )
    def __callVTKmethod ( self, m, *a ):
        cstring = ''
        cstring += self.obj
        cstring += ' '
        cstring += m
        for idx in range(len(a)):
            cstring += ' ' + str(a[idx])
        value = tk.tk.call ( *string.split ( cstring ) )
        if tk.tk.call ( 'info', 'command', value ):
            return SlicerWrapper ( self, value )
        else:
            return value
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
    def __eval ( self, inString ):
        # print inString
        return self.callTk ( *string.split ( inString ) )
    def __getattr__ ( self, name ):
        # Get the variable from the namespace
        qname = self.ns + "::" + name
        if self.__eval ( 'info exists ' + qname ):
            cname = str ( self.__eval ( 'set ' + qname ) );
            # print "found cname: ", cname
            if self.__eval ( 'info command ' + cname ):
                return SlicerWrapper ( self, cname )
            else:
                return cname
        else:
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
    


import scipy.ndimage
def test():
    nodes = ListVolumeNodes()
    t2 = nodes[0]
    data = t2.GetImageData().ToArray()
    temp = scipy.ndimage.gaussian_filter ( data, 2.0 )
    data[:] = temp[:]

    
