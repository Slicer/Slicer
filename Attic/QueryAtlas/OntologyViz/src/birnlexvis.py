from prefuse.data import Graph, Table, Tree
from prefuse.util import TypeLib, ColorLib, FontLib, GraphicsLib
from prefuse.util.display import DisplayLib
from prefuse.action.assignment import ColorAction, FontAction
from prefuse.action.filter import *
from prefuse.visual import VisualItem
from prefuse.action.animate import *
from prefuse.render import *
from prefuse.data.io import GraphMLReader
from prefuse.data.expression.parser import ExpressionParser

from prefuse import Visualization, Display, Constants
from prefuse.controls import *
from prefuse.action import ActionList, RepaintAction, Action
from prefuse.activity import Activity, SlowInSlowOutPacer
from prefuse.action.layout.graph import *
from prefuse.action.layout import *
from prefuse.visual.expression import InGroupPredicate
from prefuse.visual.sort import TreeDepthItemSorter
from prefuse.data.search import *
from prefuse.data.event import TupleSetListener
from prefuse.util.ui import JFastLabel, JSearchPanel
import prefuse
import java.lang
from javax.swing import *
from javax.swing.event import *
from java.awt import *
from java.awt.event import *
from java.awt.geom import Point2D
from java.io import *
import sys
import socket
import PrefixIntersectionSearchTupleSet
import JythonAbstractListModel
import runutils
from org.json import *

GRAPH_WIDTH=800
APP_HEIGHT=700
SIDEBAR_WIDTH=275

class UniqueNumberGenerator(object):
    def __init__(self, value=0):
        self.value = value
        return

    def __call__(self):
        ret = self.value
        self.value += 1
        return ret

GenerateId = UniqueNumberGenerator()

class DataSetInfo(object):
    """Simple container for data set info"""
    pass

def loadDataSets(filenames):
    """Load data from a list of files into a composite tree."""
    datasets = []
    tree = createTree()
    for f in filenames:
        dinfo = DataSetInfo()
        dinfo.filename = f
        data, metaInfo = readJSON(f)
        dinfo.data = data
        dinfo.metadata = metaInfo
        addToTree(tree, data, metaInfo['title'])
        datasets.append(dinfo)

    rootNodeNums = [n for n in range(0, tree.nodeCount) if tree.getParentEdge(n) == -1]

    if len(rootNodeNums) > 1:
        nodeTable = tree.nodeTable
        edgeTable = tree.edgeTable
        
        rootnum = nodeTable.addRow()
        nodeTable.set(rootnum, 'id', GenerateId())
        nodeTable.set(rootnum, 'label', 'ROOT')
        nodeTable.set(rootnum, 'title', 'ROOT')
        nodeTable.set(rootnum, 'data_source', '_')

        for n in rootNodeNums:
            source = nodeTable.getTuple(n).getString('data_source')

            parentnum = nodeTable.addRow()
            nodeTable.set(parentnum, 'id', GenerateId())        
            nodeTable.set(parentnum, 'label', "%s ROOT" % source)
            nodeTable.set(parentnum, 'title', "%s ROOT" % source)
            nodeTable.set(parentnum, 'data_source', "_")

            parent_edgenum = edgeTable.addRow()
            edgeTable.set(parent_edgenum, 'parent', rootnum)
            edgeTable.set(parent_edgenum, 'child', parentnum)

            edge_rownum = edgeTable.addRow()
            edgeTable.set(edge_rownum, 'parent', parentnum)
            edgeTable.set(edge_rownum, 'child', n)
            edgeTable.set(edge_rownum, 'data_source', source)
    
    return datasets, tree


class ChangeLevelOfDetail(ChangeListener):
    def __init__(self, vis, fisheye, action):
        self.vis = vis
        self.fisheye = fisheye
        self.action = action
        return

    def stateChanged(self, event):
        slider = event.source
        self.fisheye.setDistance(slider.getValue())
        self.vis.cancel('animatePaint')        
        self.vis.run('filter')
        return

class SelectStructureInList(ListSelectionListener):
    def __init__(self, field, vis, group):
        self.field = field
        self.vis = vis
        self.group = group
        
    def valueChanged(self, event):
        if event.valueIsAdjusting:
            return
        
        jlist = event.getSource()
        tuple = jlist.getSelectedValue()
        tset = self.vis.getFocusGroup(self.group)
        tset.clearInternal()
        tset.addTuple(tuple)
        self.vis.cancel('animatePaint')
        self.vis.run('filter')
        return

def getDataSetNames(dset):
    return [d.metadata['label'] for d in dset]

def getDataSet(dsets, name):
    for d in dsets:
        if name.lower() == d.metadata['label'].lower():
            return d
    return None

def setSearchQueryCommand(data, panel, search, vis, dataSets):
    data = data.strip()
    if not data: return ""
    if data == "@quit":
    sys.exit(0)

    dsplit = data.split()
    if dsplit[0] == "@query":
        queryString = " ".join(dsplit[1:])
        runutils.invokeAndWait(panel.setQuery, queryString)
        return "{%s}\n" % " ".join([("{{%s} {%s}}" % (s.getString('title'), s.getString('data_source')))
                                    for s in search.tuples()])

    if dsplit[0] == "@listdatasets":
        return "{%s}\n" % " ".join([("{%s}" % d.metadata['label']) for d in dataSets])

    if data[0] == '@':
        return "!Unknown Command %s\n" % data

    runutils.invokeAndWait(panel.setQuery, data)
    return "{%s}\n" % " ".join([("{{%s} {%s}}" % (s.getString('title'), s.getString('data_source')))
                                for s in search.tuples()])

    return "!Malformed request (must start with @)\n"
    

class TupleListCellRenderer(DefaultListCellRenderer):
    """Renders a selected field of a tuple into a list"""
    def __init__(self, field):
        DefaultListCellRenderer.__init__(self)
        self.field = field
        return

    def getListCellRendererComponent(self, jlist, obj, index, isSelected, hasFocus):
        t = obj.getString(self.field)
        c = DefaultListCellRenderer.getListCellRendererComponent(self, jlist, t, index, isSelected, hasFocus)
        return c

class MySocketHandlerThread(java.lang.Thread):
    def __init__(self, handler, clientsock, clientaddr, hargs, hkwargs):
        self.sock = clientsock
        self.addr = clientaddr
        self.handler = handler
        self._args = hargs
        self._kwargs = hkwargs

    def run(self):
        while 1:
            data = self.sock.recv(4096)
            if len(data) == 0:
                self.sock.close()
                break
            ret = self.handler(data, *self._args, **self._kwargs)
            if ret == None:
                self.sock.close()
                break
            else:
                self.sock.send(ret)
        return


class MySocketServer(java.lang.Thread):
    def __init__(self, hostname, port, handler, *args, **kwargs):
        self.setDaemon(1)
        self.hostname = hostname
        self.port = port
        self.handler = handler
        self._args = args
        self._kwargs = kwargs

    def run(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if self.hostname == None:
            self.hostname = socket.gethostname()

        self.socket.bind((self.hostname, self.port))
        self.socket.listen(5)
        while 1:
            (clientsocket, addr) = self.socket.accept()
            handlerThread = MySocketHandlerThread(self.handler, clientsocket, addr, self._args, self._kwargs)
            handlerThread.start()


def readJSON(filename):
    fp = open(filename)
    jsonInfo = JSONObject(fp.read())
    fp.close()

    jmetadata = jsonInfo.get('metadata')
    metaInfo = {}
    metaInfo['label'] = jmetadata.optString('label')
    metaInfo['version'] = jmetadata.optString('version')
    metaInfo['title'] = jmetadata.optString('title')    
    metaInfo['description'] = jmetadata.optString('description')    
    
    jdata = jsonInfo.get('data')
    info = dict()
    for objname in jdata.keys():
        obj = jdata.get(objname)
        try:
            d = info[objname]
        except KeyError:
            d = info[objname] = dict()
        for ok in obj.keys():
            field = obj.optJSONArray(ok)
            if field:
                els = []
                for i in range(0, field.length()):
                    els.append(field.get(i))
                d[ok] = els
            else:
                d[ok] = obj.getString(ok)

    # assign unique numbers:
    for structureName in info.keys():
        info[structureName]['id'] = GenerateId()

    return info, metaInfo

def readFile(filename):
    # reads a simple hierarchy file into a python dictionary
    fp = open(filename)

    info = dict()

    for line in fp.readlines():
        line = line.strip()  # strip whitespace
        if line == '' or line[0] == '#':
            # strip empty lines or comments
            continue

        # split the line on the first ":"
        key, value = line.split(':', 1)

        # strip whitespace
        key = key.strip()
        value = value.strip()

        # split apart the key into structure name and parameter name
        structureName, parameter = key.split('.', 1)

        # look to see if the structure name is in the dictionary
        if not info.has_key(structureName):
            # if not, then create an empty dictionary for its entry
            info[structureName] = dict()
            
        info[structureName][parameter] = value

    # assign unique numbers:
    for structureName in info.keys():
        info[structureName]['id'] = GenerateId()

    fp.close()
    return info


class ZoomToFitAction(prefuse.action.Action):
    def __init__(self, vis):
        prefuse.action.Action.__init__(self, vis)
        self.m_vis = vis
        return
        
    def run(self, frac):
        bounds = self.m_vis.getBounds(Visualization.SEARCH_ITEMS)
        d = self.m_vis.getDisplay(0)

        try:
            amount = 10.0+int(1/d.scale)
        except:
            return
        GraphicsLib.expand(bounds, amount)
        DisplayLib.fitViewToBounds(d, bounds, 0)
        return


def createTree():
    """Create a Prefuse Tree, setting up its Node and Edge columns."""
    # NODE TABLE
    # create table and columns
    nodeTable = Table()
    nodeTable.addColumn('id', TypeLib.getPrimitiveType(java.lang.Integer))
    nodeTable.addColumn('title', java.lang.String)
    nodeTable.addColumn('label', java.lang.String)
    nodeTable.addColumn('neuronames_id', java.lang.String)
    nodeTable.addColumn('umls_cid', java.lang.String)
    nodeTable.addColumn('definition', java.lang.String)
    nodeTable.addColumn('definition_source', java.lang.String)
    nodeTable.addColumn('synonyms', java.lang.String)
    nodeTable.addColumn('child_count', TypeLib.getPrimitiveType(java.lang.Integer))    
    nodeTable.addColumn('data_source', java.lang.String)
    nodeTable.addColumn('search_string', 'CONCAT([title], " ", "(", [data_source], ")")')

    # EDGE TABLE
    edgeTable = Table()
    edgeTable.addColumn('parent', TypeLib.getPrimitiveType(java.lang.Integer))
    edgeTable.addColumn('child', TypeLib.getPrimitiveType(java.lang.Integer))
    edgeTable.addColumn('data_source', java.lang.String)    

    # create a tree from the node and edge tables
    tree = Tree(nodeTable, edgeTable, 'id', 'parent', 'child')
    return tree

def addToTree(tree, info, source=""):
    """Add a dataset to an existing Tree."""
    # NODE TABLE
    nodeTable = tree.getNodeTable()    
    for structureName in info.keys():
        structureDict = info[structureName]

        # for each structure, create a row and populate it
        rownum = nodeTable.addRow()
        nodeTable.set(rownum, 'data_source', source)
        
        nodeTable.set(rownum, 'id', structureDict['id'])
        nodeTable.set(rownum, 'label', structureDict['label'])
        nodeTable.set(rownum, 'title', structureDict['title'])
        nodeTable.set(rownum, 'neuronames_id', structureDict.get('neuronames_id', ''))
        nodeTable.set(rownum, 'umls_cid', structureDict.get('umls_cid', ''))
        nodeTable.set(rownum, 'definition', structureDict.get('definition', ''))
        nodeTable.set(rownum, 'definition_source',
                      structureDict.get('definition_source', ''))
        nodeTable.set(rownum, 'synonyms',
                      ", ".join(structureDict.get('synonyms', [])))
        nodeTable.set(rownum, 'child_count', len(structureDict.get('children', [])))

    # EDGE TABLE
    edgeTable = tree.getEdgeTable()
    for structureName in info.keys():
        structureDict = info[structureName]
        parentID = info[structureName]['id']
        
        if not structureDict.has_key('children'):
            continue

        # get all the child names from the comma-separated list
        # children = structureDict['children'].split(',')
        children = structureDict['children']

        for child in children:
            childID = info[child]['id']
            if parentID == childID:
                continue

            # create a row for each parent-child link
            rownum = edgeTable.addRow()
            edgeTable.set(rownum, 'parent', parentID)
            edgeTable.set(rownum, 'child', childID)
            edgeTable.set(rownum, 'data_source', source)            
    
    return tree

class SearchTupleSetChanged(TupleSetListener):
    def __init__(self, vis):
        self.vis = vis

    def tupleSetChanged(self, t, add, rem):
        self.vis.cancel('animatePaint')
        self.vis.run('fullPaint')
        self.vis.run('animatePaint')
        # self.vis.run('resize')

class LabelSelectedItem(ControlAdapter):
    def __init__(self, title, label, label2):
        self.title = title
        self.label = label
        self.label2 = label2

    def itemEntered(self, item, event):
        if item.canGetString(self.label):
            self.title.setText("%s" % (item.getString(self.label)))

    def itemExited(self, item, event):
        self.title.setText("")


class DisplayItemInformation(TupleSetListener):    
    def __init__(self, vis, editorpane):
        self.vis = vis
        self.editorpane = editorpane

    def tupleSetChanged(self, ts, add, rem):
        if ts.tupleCount == 0:
            self.editorpane.setText("")
            return

        item = ts.tuples().next()
        self.editorpane.setText(getItemHTML(item))
        return
        

class AutoPanAction(prefuse.action.Action):
    def __init__(self):
        self.mstart = Point2D.Double()
        self.mend = Point2D.Double()
        self.mcur = Point2D.Double()
        self.mbias = 150
        return
    
    def run(self, frac):
        vis = self.getVisualization()
        display = vis.getDisplay(0)
        
        ts = vis.getFocusGroup(Visualization.FOCUS_ITEMS)
        if ts.tupleCount == 0:
            return

        if frac == 0.0:
            xbias = 0
            ybias = 0
            xbias = self.mbias
            vi = ts.tuples().next()
            self.mcur.setLocation(display.width/2, display.height/2)
            if vi != None:
                display.getAbsoluteCoordinate(self.mcur, self.mstart)
                self.mend.setLocation(vi.x + xbias, vi.y+ybias)
        else:
            self.mcur.setLocation(self.mstart.x + frac*(self.mend.x-self.mstart.x),
                                  self.mstart.y + frac*(self.mend.y-self.mstart.y))
            display.panToAbs(self.mcur)
                
        return
        
class NodeFillColorAction(ColorAction):
    def __init__(self, group):
        ColorAction.__init__(self, group, VisualItem.FILLCOLOR)
        return

    def getColor(self, item):
        vis = self.getVisualization()
        if vis.isInGroup(item, Visualization.FOCUS_ITEMS):
            return ColorLib.rgb(50, 50, 255)                
        elif vis.isInGroup(item, Visualization.SEARCH_ITEMS):
            return ColorLib.rgb(220, 220, 255)
        else:
            return ColorLib.rgba(255, 255, 255, 0)                
        

class TextColorAction(ColorAction):
    def __init__(self, group):
        ColorAction.__init__(self, group, VisualItem.TEXTCOLOR)
        return

    def getColor(self, item):
        vis = self.getVisualization()
        if vis.isInGroup(item, Visualization.FOCUS_ITEMS):
            return ColorLib.rgb(255, 255, 255)
        else:
            return ColorLib.rgb(0, 0, 0)
        
class NodeStrokeColorAction(ColorAction):
    def __init__(self, group):
        ColorAction.__init__(self, group, VisualItem.STROKECOLOR)
        return

    def getColor(self, item):
        vis = self.getVisualization()
        if item.getInt('child_count') > 0:
            return ColorLib.rgb(50, 50, 50)
        else:
            return ColorLib.rgba(255, 255, 255, 0)

def createVisualization(tree):
    vis = Visualization()
    vis.add('tree', tree)
    
    labelRenderer = LabelRenderer('title')
    labelRenderer.setRenderType(AbstractShapeRenderer.RENDER_TYPE_DRAW_AND_FILL)
    labelRenderer.setHorizontalAlignment(Constants.LEFT)
    labelRenderer.setRoundedCorner(8, 8)
    labelRenderer.setVerticalPadding(1)
    labelRenderer.setHorizontalPadding(4)
    edgeRenderer = EdgeRenderer()    
    edgeRenderer = EdgeRenderer(Constants.EDGE_TYPE_CURVE)

    edgeRenderer.setHorizontalAlignment1(Constants.RIGHT)
    edgeRenderer.setHorizontalAlignment2(Constants.LEFT)    
    edgeRenderer.setVerticalAlignment1(Constants.CENTER)
    edgeRenderer.setVerticalAlignment2(Constants.CENTER)        

    rendererFactory = DefaultRendererFactory(labelRenderer)
    rendererFactory.add(InGroupPredicate('tree.edges'), edgeRenderer)
    vis.setRendererFactory(rendererFactory)

    nodeFillColor = NodeFillColorAction('tree.nodes')
    nodeStrokeColor = NodeStrokeColorAction('tree.nodes')
    textColor = TextColorAction('tree.nodes')
    vis.putAction("textColor", textColor)
        
    edgeColor = ColorAction('tree.edges',
                            VisualItem.STROKECOLOR, ColorLib.rgb(200,200,200))

    vis.setInteractive('tree.edges', None, 0)
        
    repaint = ActionList()
    repaint.add(nodeFillColor)
    repaint.add(nodeStrokeColor)
    repaint.add(RepaintAction())
    vis.putAction('repaint', repaint)

    fullPaint = ActionList()
    fullPaint.add(nodeFillColor)
    fullPaint.add(nodeStrokeColor)    
    vis.putAction('fullPaint', fullPaint)

    animatePaint = ActionList(400)
    animatePaint.add(ColorAnimator('tree.nodes'))
    animatePaint.add(RepaintAction())
    vis.putAction('animatePaint', animatePaint)
    treeLayout = NodeLinkTreeLayout('tree', Constants.ORIENT_LEFT_RIGHT, 50, 2, 8)
    treeLayout.setLayoutAnchor(Point2D.Double(25, 300))
    vis.putAction('treeLayout', treeLayout)

    resize = ActionList(1000)
    z = ZoomToFitAction(vis)
    resize.add(z)
    vis.putAction('resize', resize)

    subLayout = CollapsedSubtreeLayout('tree', Constants.ORIENT_LEFT_RIGHT)
    vis.putAction('subLayout', subLayout)

    autoPan = AutoPanAction()
    filter = ActionList()
    
    fisheye = FisheyeTreeFilter('tree', 1)
    filter.add(fisheye)

    filter.add(FontAction('tree.nodes', FontLib.getFont('Tahoma', 12)))
    filter.add(nodeFillColor)
    filter.add(nodeStrokeColor)    
    filter.add(treeLayout)
    filter.add(subLayout)
    filter.add(textColor)
    filter.add(edgeColor)
    vis.putAction('filter', filter)

    animate = ActionList(500)
    animate.setPacingFunction(SlowInSlowOutPacer())
    animate.add(autoPan)
    animate.add(QualityControlAnimator())
    animate.add(VisibilityAnimator('tree'))
    animate.add(LocationAnimator('tree.nodes'))
    animate.add(ColorAnimator('tree.nodes'))
    animate.add(RepaintAction())
    vis.putAction('animate', animate)
    vis.alwaysRunAfter('filter', 'animate')
    return (vis, fisheye)

def createDisplay(vis):
    display = Display(vis)
    display.setHighQuality(1)
    display.setSize(600, 600)
    display.setItemSorter(TreeDepthItemSorter())
    display.addControlListener(ZoomToFitControl())
    display.addControlListener(ZoomControl())
    display.addControlListener(FocusControl(1, 'filter'))
    display.addControlListener(PanControl())
    return display


def createSearchPanel(vis):
    searchPanel = JSearchPanel(vis, 'tree.nodes', Visualization.SEARCH_ITEMS, ['search_string'], 1, 1)
    searchPanel.setShowResultCount(1)
    searchPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 4, 0))
    searchPanel.setFont(FontLib.getFont('Tahoma', Font.PLAIN, 11))
    searchPanel.setBackground(Color.WHITE)
    searchPanel.setForeground(Color.BLACK)
    searchPanel.setLabelText(None)
    return searchPanel

def createTitleWidget():
    title = JFastLabel('     ')
    title.setPreferredSize(Dimension(350, 20))
    title.setVerticalAlignment(SwingConstants.BOTTOM)
    title.setBorder(BorderFactory.createEmptyBorder(3, 0, 0, 0))
    title.setFont(FontLib.getFont('Tahoma', Font.PLAIN, 16))
    title.setBackground(Color.WHITE)
    title.setForeground(Color.BLACK)
    return title

def layoutPanel(panel, display, title):
    panel.add(display, BorderLayout.CENTER)    
    box = Box(BoxLayout.X_AXIS)
    box.add(Box.createHorizontalStrut(10))
    box.add(title)
    box.add(Box.createHorizontalGlue())
    box.add(Box.createHorizontalStrut(3))
    box.setBackground(Color.WHITE)
    panel.add(box, BorderLayout.SOUTH)
    return panel

def createPanel():
    panel = JPanel(BorderLayout())
    panel.setBackground(Color.WHITE)
    panel.setForeground(Color.BLACK)
    return panel

def getItemHTML(item):
    html = []

    title = item.getString('title')
    umls_cid = item.getString('umls_cid')
    nn_id = item.getString('neuronames_id')
    definition = item.getString('definition')
    definition_source = item.getString('definition_source')
    synonyms = item.getString('synonyms')
    data_source = item.getString('data_source')
    
    html.append('''<div style="font: sans-serif; font-size: 10pt">
    <table>
    <tr><td><b>Name: </b></td><td>%s</td></tr>''' % title)

    if umls_cid:
        html.append('<tr><td><b>UMLS CID:</b></td><td>%s</td></tr>' % umls_cid)

    if nn_id:
        html.append('<tr><td><b>Neuronames ID:</b></td><td>%s</td></tr>' % nn_id)

    if definition:
        html.append('''
        <tr><td colspan="2"><b>Definition:</b></td></tr>
        <tr><td colspan="2">%s</td></tr>''' % definition)

    if definition_source:
        html.append('''
        <tr><td><b>Definition source:</b></td>
        <td>%s</td></tr>''' % definition_source)

    if data_source:
        html.append('''
        <tr><td><b>Data source:</b></td>
        <td>%s</td></tr>''' % data_source)

    if synonyms:
        html.append('''
        <tr><td colspan="2"><b>Synonyms:</b></td></tr>
        <tr><td colspan="2">%s</td></tr>''' % synonyms)

    html.append('</table></div>')
    return " ".join(html)
    

def createListWidget():
    w =  JList()
    w.setFont(FontLib.getFont('Tahoma', 10))
    return w

def createSidebar(searchPanel):
    panel = JPanel(BorderLayout())
    panel.setBackground(Color.WHITE)
    panel.setForeground(Color.BLACK)
    matchList = createListWidget()
    
    scrollPane = JScrollPane(matchList)
    panel.add(searchPanel, BorderLayout.NORTH)
    panel.add(scrollPane, BorderLayout.CENTER)    

    detailSlider = JSlider(JSlider.HORIZONTAL, 1, 10, 1)
    detailSlider.setMinorTickSpacing(1)
    detailSlider.setPaintTicks(1)
    detailSlider.setSnapToTicks(1)
    panel.add(detailSlider, BorderLayout.SOUTH)
    editorPane = JEditorPane()
    editorPane.setContentType('text/html')
    editorPane.editable = 0

    panel.minimumSize = Dimension(200, 200)
    panel.preferredSize = Dimension(200, 500)        

    editorPane.minimumSize = Dimension(200, 200)    
    # panel.add(editorPane, BorderLayout.SOUTH)

    eScrollPane = JScrollPane(editorPane)

    vertPane = JSplitPane(JSplitPane.VERTICAL_SPLIT)
    vertPane.setLeftComponent(panel)    
    vertPane.setRightComponent(eScrollPane)
    
    return (vertPane, matchList, editorPane, detailSlider)
    
class SearchBasedListModel(JythonAbstractListModel, TupleSetListener):
    def __init__(self, searcher, field):
        self.search = searcher
        self.field = field
        self.search.addTupleSetListener(self)
        self.tuples = []
        return
    
    def tupleSetChanged(self, tset, added, removed):
        self.tuples = [(s, s.getString(self.field)) for s in self.search.tuples()]
        self.tuples.sort(lambda x, y: cmp(x[1], y[1]))
        
        count = len(self.tuples)
        self.fireIntervalRemoved(self, 0, count)
        self.fireIntervalAdded(self, 0, count)
        return

    def getSize(self):
        return len(self.tuples)

    def getElementAt(self, idx):
        if not self.tuples: return None
        return self.tuples[idx][0]

    def removedAll(self):
        self.fireIntervalRemoved(self, 0, self.getSize());

    def addedAll(self):
        self.fireIntervalAdded(self, 0, self.getSize());        

    def updatedAll(self):
        self.fireIntervalChanged(self, 0, self.getSize());



def main(frameTitle, filenames, hostname, port):
    import sys
    dataSets, tree = loadDataSets(filenames)

    vis, fisheye = createVisualization(tree)
    display = createDisplay(vis)

    search = PrefixIntersectionSearchTupleSet()
    vis.addFocusGroup(Visualization.SEARCH_ITEMS, search)
    search.addTupleSetListener(SearchTupleSetChanged(vis))

    searchPanel = createSearchPanel(vis)

    title = createTitleWidget()
    display.addControlListener(LabelSelectedItem(title, 'title', 'label'))

    graphPanel = createPanel()
    layoutPanel(graphPanel, display, title)
    
    vis.run('filter')

    mainFrame = JFrame(frameTitle)
    mainFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE)

    splitPane = JSplitPane(JSplitPane.HORIZONTAL_SPLIT)

    mainFrame.add(splitPane)

    sidebarPanel, matchList, editorpane, detailSlider = createSidebar(searchPanel)
    sidebarPanel.minimumSize = Dimension(SIDEBAR_WIDTH, APP_HEIGHT)
    sidebarPanel.preferredSize = Dimension(SIDEBAR_WIDTH, APP_HEIGHT)    

    graphPanel.minimumSize = Dimension(200, 200)
    graphPanel.preferredSize = Dimension(GRAPH_WIDTH, APP_HEIGHT)
    
    splitPane.setLeftComponent(sidebarPanel)    
    splitPane.setRightComponent(graphPanel)

    vis.getFocusGroup(Visualization.FOCUS_ITEMS).addTupleSetListener(DisplayItemInformation(vis, editorpane))
    detailSlider.addChangeListener(ChangeLevelOfDetail(vis, fisheye, 'filter'))

    listModel = SearchBasedListModel(search, 'search_string')
    matchList.setModel(listModel)

    if len(dataSets) > 1:
        matchList.setCellRenderer(TupleListCellRenderer('search_string'))
    else:
        matchList.setCellRenderer(TupleListCellRenderer('title'))        
    matchList.addListSelectionListener(SelectStructureInList('search_string', vis, Visualization.FOCUS_ITEMS))
    
    mainFrame.pack()
    mainFrame.setVisible(1)
    
    socketServer = MySocketServer(hostname, port, setSearchQueryCommand, searchPanel, search, vis, dataSets)
    socketServer.start()

if __name__ == '__main__':
    import getopt
    
    frameTitle = 'Slicer/Ontology Vis'
    hostname = '127.0.0.1'
    port = 3334

    optlist, args = getopt.getopt(sys.argv[1:], 'p:h:t:')
    for flag, arg in optlist:
        if flag == '-p':
            port = int(arg)
        if flag == '-h':
            hostname = arg
        if flag == '-t':
            frameTitle = arg

    filenames = args[0:]
    if len(filenames) == 0:
        print >>sys.stdout, ("%s: usage: \n\t%s [-p port] [-h hostname] [-t app-title] datafiles..."
                             % (sys.argv[0], sys.argv[0]))
        sys.exit(1)

    runutils.invokeLater(main, frameTitle, filenames, hostname, port)
