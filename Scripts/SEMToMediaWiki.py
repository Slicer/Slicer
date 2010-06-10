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

# header
print "__NOTOC__"
print "===%s===" % getThisNodesInfoAsText(executableNode,"title")
print "%s\n" % getThisNodesInfoAsText(executableNode,"title")

print "{|"
print "|[[Image:screenshotBlankNotOptional.png|thumb|280px|User Interface]]"
print "|[[Image:screenshotBlank.png|thumb|280px|Output]]"
print "|[[Image:screenshotBlank.png|thumb|280px|Caption]]"
print "|}"


# Print information about the program in general
print "== General Information ==\n"
print "===Module Type & Category===\n"
print "Type: CLI\n"
print "Category: %s\n" % getThisNodesInfoAsText(executableNode,"category")
print "\n"

print "===Authors, Collaborators & Contact===\n"
print "Author: %s\n" % getThisNodesInfoAsText(executableNode,"contributor")
print "Contributors: \n"
print "Contact: name, email"
print "\n"

print "===Module Description===\n"
print "{| style=\"color:green\" border=\"1\"\n"
getThisNodesInfoAsTextTableLine(executableNode,"title")
getThisNodesInfoAsTextTableLine(executableNode,"description")
getThisNodesInfoAsTextTableLine(executableNode,"version")
getThisNodesInfoAsTextTableLine(executableNode,"documentation-url")
getThisNodesInfoAsTextTableLine(executableNode,"doesnotexiststest")
print "|}\n"

print "== Usage ==\n"

print "===Use Cases, Examples===\n"

print "This module is especially appropriate for these use cases:\n"

print "* Use Case 1:"
print "* Use Case 2:\n"

print "Examples of the module in use:\n"

print "* Example 1:"
print "* Example 2:\n"

print "===Tutorials===\n"

print "* Tutorial 1"
print "** Data Set 1\n"

print "===Quick Tour of Features and Use===\n"
print "A list panels in the interface, their features, what they mean, and how to use them.\n"

print "{|"
print "|"
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

print "|[[Image:screenshotBlankNotOptional.png|thumb|280px|User Interface]]"
print "|}\n"

# footer
print "== Development ==\n"

print "===Notes from the Developer(s)===\n"

print "Algorithms used, library classes depended upon, use cases, etc.\n"

print "===Dependencies===\n"

print "Other modules or packages that are required for this module's use.\n"

print "===Tests===\n"

print "On the [http://www.cdash.org/CDash/index.php?project=Slicer3 Dashboard], these tests verify that the module is working on various platforms:\n"

print "* MyModuleTest1 [http://viewvc.slicer.org/viewcvs.cgi/trunk MyModuleTest1.cxx]"
print "* MyModuleTest2 [http://viewvc.slicer.org/viewcvs.cgi/trunk MyModuleTest2.cxx]\n"

print "===Known bugs===\n"

print "Links to known bugs in the Slicer3 bug tracker\n"

print "* [http://www.na-mic.org/Bug/view.php?id=000 Bug 000: description] \n"


print "===Usability issues===\n"

print "Follow this [http://na-mic.org/Mantis/main_page.php link] to the Slicer3 bug tracker. Please select the '''usability issue category''' when browsing or contributing.\n"

print "===Source code & documentation===\n"

print "Links to the module's source code:\n"

print "Source code:"
print "*[http://viewvc.slicer.org/viewcvs.cgi/trunk file.cxx ]"
print "*[http://viewvc.slicer.org/viewcvs.cgi/trunk file.h ]\n"
 
print "Doxygen documentation:"
print "*[http://www.na-mic.org/Slicer/Documentation/Slicer3-doc/html/classes.html class1]\n"

print "== More Information == \n"

print "===Acknowledgment===\n"
print "%s\n" % getThisNodesInfoAsText(executableNode,"acknowledgements")

print "===References===\n"
print "Publications related to this module go here. Links to pdfs would be useful."
