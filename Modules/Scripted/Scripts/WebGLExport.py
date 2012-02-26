from __main__ import vtk, qt, ctk, slicer

import os
import shutil
import time
import uuid
import webbrowser


# webserver support for easy display of local WebGL content
import socket
import SimpleHTTPServer
import SocketServer
import multiprocessing as m

# this module uses the following from http://www.quesucede.com/page/show/id/python_3_tree_implementation
#
#
# Python 3 Tree Implementation
#
# Copyright (C) 2011, Brett Alistair Kromkamp - brettkromkamp@gmail.com
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this list
# of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice, this
# list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# Neither the name of the copyright holder nor the names of the contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
def sanitize_id( id ): return id.strip().replace( " ", "" )

( _ADD, _DELETE, _INSERT ) = range( 3 )
( _ROOT, _DEPTH, _WIDTH ) = range( 3 )

class Node:
  def __init__( self, name, identifier=None, expanded=True ):
      self.__identifier = ( str( uuid.uuid1() ) if identifier is None else
              sanitize_id( str( identifier ) ) )
      self.name = name
      self.expanded = expanded
      self.__bpointer = None
      self.__fpointer = []

  @property
  def identifier( self ):
      return self.__identifier

  @property
  def bpointer( self ):
      return self.__bpointer

  @bpointer.setter
  def bpointer( self, value ):
      if value is not None:
          self.__bpointer = sanitize_id( value )

  @property
  def fpointer( self ):
      return self.__fpointer

  def update_fpointer( self, identifier, mode=_ADD ):
      if mode is _ADD:
          self.__fpointer.append( sanitize_id( identifier ) )
      elif mode is _DELETE:
          self.__fpointer.remove( sanitize_id( identifier ) )
      elif mode is _INSERT:
          self.__fpointer = [sanitize_id( identifier )]

class Tree:
    def __init__( self ):
        self.nodes = []

    def get_index( self, position ):
        for index, node in enumerate( self.nodes ):
            if node.identifier == position:
                break
        return index

    def create_node( self, name, identifier=None, parent=None ):

        node = Node( name, identifier )
        self.nodes.append( node )
        self.__update_fpointer( parent, node.identifier, _ADD )
        node.bpointer = parent
        return node

    def show( self, position, level=_ROOT ):
        queue = self[position].fpointer
        if level == _ROOT:
            print( "{0} [{1}]".format( self[position].name,
                                     self[position].identifier ) )
        else:
            print( "\t" * level, "{0} [{1}]".format( self[position].name,
                                                 self[position].identifier ) )
        if self[position].expanded:
            level += 1
            for element in queue:
                self.show( element, level )  # recursive call

    def expand_tree( self, position, mode=_DEPTH ):
        # Python generator. Loosly based on an algorithm from 'Essential LISP' by
        # John R. Anderson, Albert T. Corbett, and Brian J. Reiser, page 239-241
        yield position
        queue = self[position].fpointer
        while queue:
            yield queue[0]
            expansion = self[queue[0]].fpointer
            if mode is _DEPTH:
                queue = expansion + queue[1:]  # depth-first
            elif mode is _WIDTH:
                queue = queue[1:] + expansion  # width-first

    def is_branch( self, position ):
        return self[position].fpointer

    def __update_fpointer( self, position, identifier, mode ):
        if position is None:
            return
        else:
            self[position].update_fpointer( identifier, mode )

    def __update_bpointer( self, position, identifier ):
        self[position].bpointer = identifier

    def __getitem__( self, key ):
        return self.nodes[self.get_index( key )]

    def __setitem__( self, key, item ):
        self.nodes[self.get_index( key )] = item

    def __len__( self ):
        return len( self.nodes )

    def __contains__( self, identifier ):
        return [node.identifier for node in self.nodes
                if node.identifier is identifier]


#
# WebGLExport
#

class WebGLExport:
  def __init__( self, parent ):
    parent.title = "WebGL Export"
    parent.categories = ["Work in progress"]
    parent.contributors = ["Daniel Haehn"]
    parent.helpText = """
Export the models in the 3D Slicer scene to WebGL. The WebGL visualization is powered by XTK (<a href='http://goXTK.com'>http://goXTK.com</a>).
<br><br>
Currently color, visibility and opacity of individual models are supported.
<br><br>
More information: <a href='http://github.com/xtk/SlicerWebGLExport'>http://github.com/xtk/SlicerWebGLExport</a>
    """
    parent.acknowledgementText = """
    Flex, dude!
    """
    self.parent = parent

#
# qSlicerPythonModuleExampleWidget
#

class WebGLExportWidget:
  def __init__( self, parent=None ):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.parent.setMRMLScene( slicer.mrmlScene )
    else:
      self.parent = parent
    self.logic = WebGLExportLogic()
    self.__httpd = None
    self.__port = 3456
    self.__p = None
    if not parent:
      self.setup()
      self.parent.show()

  def __del__( self ):
    # if we have a httpd server running, kill it
    if self.__httpd:
      self.__p.terminate()

  def setup( self ):

    # settings
    settingsButton = ctk.ctkCollapsibleButton()
    settingsButton.text = "Settings"
    settingsButton.collapsed = False
    self.parent.layout().addWidget( settingsButton )

    settingsLayout = qt.QFormLayout( settingsButton )

    self.__dirButton = ctk.ctkDirectoryButton()
    settingsLayout.addRow( "Output directory:", self.__dirButton )

    self.__viewCheckbox = qt.QCheckBox()
    self.__viewCheckbox.setChecked( True )
    settingsLayout.addRow( "View after export:", self.__viewCheckbox )

    # advanced
    advancedButton = ctk.ctkCollapsibleButton()
    advancedButton.text = "Advanced"
    advancedButton.collapsed = True
    self.parent.layout().addWidget( advancedButton )

    advancedLayout = qt.QFormLayout( advancedButton )

    self.__copyCheckbox = qt.QCheckBox()
    self.__copyCheckbox.setChecked( True )
    advancedLayout.addRow( "Copy models to output directory:", self.__copyCheckbox )

    self.__captionCombobox = qt.QComboBox()
    self.__captionCombobox.addItems( ['None', 'Model name', 'Hierarchy name'] )
    self.__captionCombobox.currentIndex = 1 # Model name by default
    advancedLayout.addRow( "Set captions from:", self.__captionCombobox )

    self.__serverCheckbox = qt.QCheckBox()
    self.__serverCheckbox.setChecked( True )
    advancedLayout.addRow( "Run internal web server:", self.__serverCheckbox )

    # Apply button
    self.__exportButton = qt.QPushButton( "Export to WebGL" )
    self.__exportButton.toolTip = "Export to WebGL using XTK."
    self.__exportButton.enabled = True
    self.parent.layout().addWidget( self.__exportButton )

    # Add vertical spacer
    self.parent.layout().addStretch( 1 )

    # connections
    self.__exportButton.connect( 'clicked()', self.onExport )

  def onExport( self ):
    """
    Export to the filesystem.
    """
    self.__exportButton.text = "Working..."
    slicer.app.processEvents()

    outputDir = os.path.abspath( self.__dirButton.directory )
    outputFile = os.path.join( outputDir, 'index.html' )

    try:
      output = self.logic.export( self.__captionCombobox.currentIndex, self.__copyCheckbox.checked, outputDir )
    except Exception as e:
      # maybe the scene was not saved?
      qt.QMessageBox.warning( None, 'Error', 'Please make sure the scene was saved before attempting to export to WebGL!' )
      self.__exportButton.text = "Export to WebGL"
      return

    if self.__serverCheckbox.checked:
      # start server
      os.chdir( outputDir )

      # if we have already a httpd running, kill it now
      # it will likely leave an orphaned process but since we mark it killed,
      # slicer will destroy it on exit
      if self.__httpd:
        self.__p.terminate()
        # increase the port
        self.__port += 1

      # check first if the port is available (since we open it as a new process we can not check later)
      portFree = False
      while not portFree:
        try:
          s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
          s.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
          s.bind( ( "", self.__port ) )
        except socket.error, e:
          portFree = False
          self.__port += 1
        finally:
          s.close()
          portFree = True

      # we need to break out of the pythonQt context here to make multiprocessing work
      sys.stdin = sys.__stdin__
      sys.stdout = sys.__stdout__
      sys.stderr = sys.__stderr__

      self.__handler = SimpleHTTPServer.SimpleHTTPRequestHandler
      self.__httpd = SocketServer.TCPServer( ( "", self.__port ), self.__handler )
      self.__p = m.Process( target=self.__httpd.serve_forever )
      self.__p.start()

      url = 'http://localhost:' + str( self.__port ) + '/index.html'
    else:
      # no server
      url = outputFile

    with open( outputFile, 'w' ) as f:
      f.write( output )

    self.__exportButton.text = "Export to WebGL"

    if self.__viewCheckbox.checked:
      time.sleep( 1 )
      webbrowser.open_new_tab( url )




class WebGLExportLogic:
  """
  The actual export logic.
  """

  def __init__( self ):
    self.__tree = None
    self.__nodes = {}

    # the html header
    self.__header = """
<html>
<!-- WebGL Export for 3D Slicer4 powered by XTK -- http://goXTK.com -->
  <head>
    <title>WebGL Export</title>
    <script type="text/javascript" src="http://goXTK.com/xtk_nightly.js"></script>
    <script type="text/javascript">
      var run = function() {

%s

"""

    # the html footer
    self.__footer = """

%s

      };
    </script>
  </head>
  <body style="margin:0px; padding:0px;" onload="run()">
%s
  </body>
</html>
"""

  def configureXrenderers( self ):
    """
    Grab some Slicer environment values like the camera position etc. and configure the X.renderers
    """
    init = ' ' * 8 + "r%s = new X.renderer('r%s');" + '\n' + ' ' * 8 + 'r%s.init();' + '\n'
    configuredInit = ''
    div = ' ' * 8 + '<div id="r%s" style="background-color: %s; width: %s; height: %s;%s"></div>' + '\n'
    configuredDiv = ''
    render = ' ' * 8 + '%sr%s.add(scene);' + '\n'
    render += ' ' * 8 + 'r%s.camera().setPosition%s;' + '\n'
    render += ' ' * 8 + 'r%s.camera().setUp%s;' + '\n'
    render += ' ' * 8 + 'r%s.render();%s' + '\n\n'
    configuredRender = ''

    # check the current layout
    renderers = []


    if slicer.app.layoutManager().layout == 15:
      # dual 3d
      renderers.append( 0 )
      renderers.append( 1 )
    elif slicer.app.layoutManager().layout == 19:
      # triple 3d
      renderers.append( 0 )
      renderers.append( 1 )
      renderers.append( 2 )
    else:
      # always take just the main 3d view
      renderers.append( 0 )

    threeDViews = slicer.app.layoutManager().threeDViewCount


    for r in xrange( threeDViews ):
      # grab the current 3d view background color
      threeDWidget = slicer.app.layoutManager().threeDWidget( r )
      threeDView = threeDWidget.threeDView()

      if not threeDView.isVisible():
        continue

      mrmlViewNode = threeDView.mrmlViewNode()
      bgColor = threeDView.backgroundColor.name() + ';'

      # grab the current camera position and up vector
      cameraNodes = slicer.util.getNodes( 'vtkMRMLCamera*' )
      cameraNode = None

      for c in cameraNodes.items():
        cameraNode = c[1]
        if cameraNode.GetActiveTag() == mrmlViewNode.GetID():
          # found the cameraNode
          break

      if not cameraNode:
        raise Exception( 'Something went terribly wrong..' )

      camera = cameraNode.GetCamera()
      cameraPosition = str( camera.GetPosition() )
      cameraUp = str( camera.GetViewUp() )

      width = '100%'
      height = '100%'
      float = ''
      begin = '';
      end = '';

      if ( len( renderers ) == 2 ):
        # dual 3d
        width = '49.35%'
        if threeDWidget.x == 0:
          # this is the left one
          float += 'position:absolute;left:0;bottom:0;'
        else:
          begin = 'r0.onShowtime = function() {'
          end = '}'
          float += 'position:absolute;right:0;bottom:0;'
      elif ( len( renderers ) == 3 ):
        height = '49.25%'
        # triple 3d
        if r != 0:
          # this is the second row
          width = '49.35%'
          if threeDWidget.x == 0:
            # this is the left one
            begin = ' ' * 8 + 'r0.onShowtime = function() {'
            float += 'position:absolute;left:0;bottom:0;'
          else:
            end = ' ' * 8 + '};'
            float += 'position:absolute;right:0;bottom:0;'

      configuredInit += init % ( r, r, r )
      configuredRender += render % ( begin, r, r, cameraPosition, r, cameraUp, r, end )
      configuredDiv += div % ( r, bgColor, width, height, float )


    # .. and configure the X.renderer
    header = self.__header % ( configuredInit )
    footer = self.__footer % ( configuredRender, configuredDiv )

    return [header, footer]


  def export( self, captionMode, copyFiles, outputDir ):
    """
    Run through the mrml scene and create an XTK tree based on vtkMRMLModelHierarchyNodes and vtkMRMLModelNodes
    """
    scene = slicer.mrmlScene
    nodes = scene.GetNumberOfNodes()

    self.__nodes = {}

    # 1 for model name, 2 for parent name
    self.__captionMode = captionMode
    # TRUE if we shall copy the files to the outputDir
    self.__copyFiles = copyFiles
    self.__outputDir = outputDir

    self.__tree = Tree()
    self.__tree.create_node( "Scene", "scene" )

    for n in xrange( nodes ):

        node = scene.GetNthNode( n )

        self.parseNode( node )

    [header, footer] = self.configureXrenderers()
    output = header
    output += self.createXtree( "scene" )
    output += footer

    return output


  def parseNode( self, node ):
    """
    Parse one mrml node if it is a valid vtkMRMLModelNode or vtkMRMLModelHierarchyNode and add it to our tree
    """

    if not node:
        return

    if ( not node.IsA( 'vtkMRMLModelNode' ) and not node.IsA( 'vtkMRMLModelHierarchyNode' ) ) or ( node.IsA( 'vtkMRMLModelNode' ) and node.GetHideFromEditors() ):
        return

    if self.__nodes.has_key( node.GetID() ):
        return

    parent_node = "scene"

    parentNode = None
    hNode = None

    if node.IsA( 'vtkMRMLModelNode' ):
        parentNode = slicer.app.applicationLogic().GetModelHierarchyLogic().GetModelHierarchyNode( node.GetID() )

        if parentNode:
            parentNode = parentNode.GetParentNode()

    elif node.IsA( 'vtkMRMLModelHierarchyNode' ):
        parentNode = node.GetParentNode()

    if parentNode:
        if parentNode.GetID() == node.GetID():
            return

        parent_node = parentNode.GetID()
        self.parseNode( parentNode )

    if not node.IsA( 'vtkMRMLModelHierarchyNode' ) or not node.GetModelNode():

        self.__nodes[node.GetID()] = node.GetName()
        self.__tree.create_node( node.GetName(), node.GetID(), parent=parent_node )


  def createXtree( self, position, level=_ROOT, parent="" ):
    """
    Convert the internal tree to XTK code.
    """
    queue = self.__tree[position].fpointer
    mrmlId = self.__tree[position].identifier

    output = ' ' * 8 + mrmlId + ' = new X.object();\n'

    if not level == _ROOT:

      n = slicer.mrmlScene.GetNodeByID( mrmlId )
      if n.IsA( 'vtkMRMLModelNode' ):

        # grab some properties
        s = n.GetStorageNode()
        if not s:
          # error
          raise Exception( 'Scene not saved!' )

        file = s.GetFileName()
        if not file:
          # error
          raise Exception( 'Scene not saved!' )

        d = n.GetDisplayNode()
        color = str( d.GetColor() )
        opacity = str( d.GetOpacity() )
        visible = str( bool( d.GetVisibility() ) ).lower()

        if self.__copyFiles:
          fileName = os.path.split( file )[1]
          shutil.copy( file, os.path.join( self.__outputDir, fileName ) )
          file = fileName

        output += ' ' * 8 + mrmlId + '.load(\'' + file + '\');\n'
        output += ' ' * 8 + mrmlId + '.setColor' + color + ';\n'
        output += ' ' * 8 + mrmlId + '.setOpacity(' + opacity + ');\n'
        output += ' ' * 8 + mrmlId + '.setVisible(' + visible + ');\n'

        if self.__captionMode == 1:
          # From Model Name
          output += ' ' * 8 + mrmlId + '.setCaption(\'' + n.GetName() + '\');\n'
        elif self.__captionMode == 2:
          # From Parent
          parentNode = slicer.util.getNode( parent )
          if parentNode:
            output += ' ' * 8 + mrmlId + '.setCaption(\'' + parentNode.GetName() + '\');\n'

      output += ' ' * 8 + parent + '.children().push(' + mrmlId + ');\n\n'

    level += 1
    for element in queue:
        output += self.createXtree( element, level, mrmlId )  # recursive call

    return output




class Slicelet( object ):
  """A slicer slicelet is a module widget that comes up in stand alone mode
  implemented as a python class.
  This class provides common wrapper functionality used by all slicer modlets.
  """
  # TODO: put this in a SliceletLib
  # TODO: parse command line args


  def __init__( self, widgetClass=None ):
    self.parent = qt.QFrame()
    self.parent.setLayout( qt.QVBoxLayout() )

    # TODO: should have way to pop up python interactor
    self.buttons = qt.QFrame()
    self.buttons.setLayout( qt.QHBoxLayout() )
    self.parent.layout().addWidget( self.buttons )
    self.addDataButton = qt.QPushButton( "Add Data" )
    self.buttons.layout().addWidget( self.addDataButton )
    self.addDataButton.connect( "clicked()", slicer.app.ioManager().openAddDataDialog )
    self.loadSceneButton = qt.QPushButton( "Load Scene" )
    self.buttons.layout().addWidget( self.loadSceneButton )
    self.loadSceneButton.connect( "clicked()", slicer.app.ioManager().openLoadSceneDialog )

    if widgetClass:
      self.widget = widgetClass( self.parent )
      self.widget.setup()
    self.parent.show()

class WebGLExportSlicelet( Slicelet ):
  """ Creates the interface when module is run as a stand alone gui app.
  """

  def __init__( self ):
    super( WebGLExportSlicelet, self ).__init__( WebGLExportWidget )


if __name__ == "__main__":
  # TODO: need a way to access and parse command line arguments
  # TODO: ideally command line args should handle --xml

  import sys
  print( sys.argv )

  slicelet = WebGLExportSlicelet()
