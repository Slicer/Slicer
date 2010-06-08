#!python
# \author Hans J. Johnson
#
# \description This python script is to help convert from
# SEM xml documentation to a form suitable for posting on
# a MediaWiki format
#
# \usage python SEMToMediaWiki.py myProgramSEM.xml > MediaWikiFormattedText.txt
#
import sys
import pprint

import xml.dom.minidom
from xml.dom.minidom import Node

# Get this nodes text information.
def getTextValuesFromNode(nodelist):
    rc = []
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            rc.append(node.data)
    return ''.join(rc)

# Create a formatted text string suitable for inclusion in a MediaWiki table
def getThisNodesInfoAsTextTableLine(executableNode,label):
    labelNodeList=executableNode.getElementsByTagName(label)
    if labelNodeList.length > 0 :
        labelNode=labelNodeList[0]  ## Only get the first one
        print "|Program %s || %s\n" % (label,getTextValuesFromNode(labelNode.childNodes) )
        print "|-"

# Only get the text info for the matching label at this level of the tree
def getThisNodesInfoAsText(executableNode,label):
    labelNodeList=executableNode.getElementsByTagName(label)
    if labelNodeList.length > 0 :
        labelNode=labelNodeList[0]  ## Only get the first one
        return getTextValuesFromNode(labelNode.childNodes);
    return ""

# Extract the long flag, and color the text string
def getLongFlagDefinition(executableNode):
    labelNodeList=executableNode.getElementsByTagName("longflag")
    if labelNodeList.length > 0 :
        labelNode=labelNodeList[0]  ## Only get the first one
        return "[<span style=\"color:orange\">--"+getTextValuesFromNode(labelNode.childNodes)+"</span>]";
    return "";

# Extract the (short) flag, and color the text string
def getFlagDefinition(executableNode):
    labelNodeList=executableNode.getElementsByTagName("flag")
    if labelNodeList.length > 0 :
        labelNode=labelNodeList[0]  ## Only get the first one
        return "[<span style=\"color:pink\">-"+getTextValuesFromNode(labelNode.childNodes)+"</span>]";
    return "";

# Extract the nodes label, and color the text string
def getLabelDefinition(executableNode):
    labelNodeList=executableNode.getElementsByTagName("label")
    if labelNodeList.length > 0 :
        labelNode=labelNodeList[0]  ## Only get the first one
        return "** <span style=\"color:green\">'''"+getTextValuesFromNode(labelNode.childNodes)+"'''</span>";
    return "";

# Extract the default value
def getDefaultValueDefinition(executableNode):
    labelNodeList=executableNode.getElementsByTagName("default")
    if labelNodeList.length > 0 :
        labelNode=labelNodeList[0]  ## Only get the first one
        return "''Default value: "+getTextValuesFromNode(labelNode.childNodes)+"''";
    return "";

if len(sys.argv) != 2:
  print "Usage: pass SEM XML description file as parameter!"
  sys.exit()

## Read the xml file from the first argument of command line
doc = xml.dom.minidom.parse(sys.argv[1]);

# Get the primary heirarchial tree node.
executableNode=doc.getElementsByTagName("executable")[0]

# Print information about the program in general
print "===Module Type & Category===\n"
print "Type: CLI\n"
print "Category: %s\n" % getThisNodesInfoAsText(executableNode,"category")
print "\n"

print "===Authors, Collaborators & Contact===\n"
print "Author: %s\n" % getThisNodesInfoAsText(executableNode,"contributor")
print "Contributors: %s\n" % getThisNodesInfoAsText(executableNode,"acknowledgements")
print "\n"

print "===Module Description===\n"
print "{| style=\"color:green\" border=\"1\"\n"
getThisNodesInfoAsTextTableLine(executableNode,"title")
getThisNodesInfoAsTextTableLine(executableNode,"descripition")
getThisNodesInfoAsTextTableLine(executableNode,"version")
getThisNodesInfoAsTextTableLine(executableNode,"documentation-url")
getThisNodesInfoAsTextTableLine(executableNode,"doesnotexiststest")
print "|}\n"

# Now print all the command line arguments and the labels that showup in the GUI interface
for parameterNode in executableNode.getElementsByTagName("parameters"):
  print "* <span style=\"color:blue\">'''''%s''''' </span> " % getThisNodesInfoAsText(parameterNode,"label")
  currentNode = parameterNode.firstChild
  while currentNode is not None :
      if currentNode.nodeType == currentNode.ELEMENT_NODE:
        if getThisNodesInfoAsText(currentNode,"label") != "": #If this node doe not have a "label" element, then just skip it.
          if getThisNodesInfoAsText(currentNode,"default") != "": # if this node has a default value -- document it!
            print "%s %s %s: %s %s" % (getLabelDefinition(currentNode), getLongFlagDefinition(currentNode),getFlagDefinition(currentNode),getThisNodesInfoAsText(currentNode,"description"),getDefaultValueDefinition(currentNode))
          else:
            print "%s %s %s: %s" % (getLabelDefinition(currentNode), getLongFlagDefinition(currentNode),getFlagDefinition(currentNode),getThisNodesInfoAsText(currentNode,"description"))
      currentNode=currentNode.nextSibling
  print # newlines to make it more readable
  print

