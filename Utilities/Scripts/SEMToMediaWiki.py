#!python
program_description = r"""
\author Hans J. Johnson

\description This python script is to help convert from
SEM xml documentation to a form suitable for posting on
a MediaWiki format
"""

import sys
import pprint

import xml.dom.minidom
from xml.dom.minidom import Node


def getTextValuesFromNode(nodelist):
    r"""
    Get this nodes text information.
    """
    rc = []
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            rc.append(node.data)
    return ''.join(rc)


def getThisNodesInfoAsText(currentNode, label):
    r"""
    Only get the text info for the matching label at this level of the tree
    """
    labelNodeList = [node for node in
    currentNode.childNodes if node.nodeName == label]

    if len(labelNodeList) > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return getTextValuesFromNode(labelNode.childNodes)
    return ""


def getLongFlagDefinition(currentNode):
    r"""
    Extract the long flag, and color the text string
    """
    labelNodeList = currentNode.getElementsByTagName("longflag")
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return "{0}{1}{2}".format("[<span style=\"color:orange\">--",
            getTextValuesFromNode(labelNode.childNodes), "</span>]")
    return ""


def getFlagDefinition(currentNode):
    r"""
    Extract the (short) flag, and color the text string
    """
    labelNodeList = currentNode.getElementsByTagName("flag")
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return "{0}{1}{2}".format("[<span style=\"color:pink\">-",
            getTextValuesFromNode(labelNode.childNodes), "</span>]")
    return ""


def getLabelDefinition(currentNode):
    r"""
    Extract the nodes label, and color the text string
    """
    labelNodeList = currentNode.getElementsByTagName("label")
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return "{0}{1}{2}".format("** <span style=\"color:green\">'''",
            getTextValuesFromNode(labelNode.childNodes), "'''</span>")
    return ""


def getDefaultValueDefinition(currentNode):
    r"""
    Extract the default value
    """
    labelNodeList = currentNode.getElementsByTagName("default")
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return "{0}{1}{2}".format("''Default value: ",
            getTextValuesFromNode(labelNode.childNodes), "''")
    return ""


def GetSEMDoc(filename):
    r"""
    Read the xml file from the first argument of command line
    Return the primary heirarchial tree node.
    """
    doc = xml.dom.minidom.parse(filename)
    executableNode = [node for node in doc.childNodes if
            node.nodeName == "executable"]
    #Only use the first
    return executableNode[0]


def DumpSEMMediaWikiHeader(executableNode):
    r"""
    Just dump the header section of the MediaWikiPage
    """

    outputRegionTemplate = """
__NOTOC__

==={executableTitle}===
{executableTitle}

{tblStart}
|[[Image:screenshotBlankNotOptional.png|thumb|280px|User Interface]]
|[[Image:screenshotBlank.png|thumb|280px|Output]]
|[[Image:screenshotBlank.png|thumb|280px|Caption]]
{tblStop}

== General Information ==

===Module Type & Category===

Type: CLI

Category: {executableCategory}


===Authors, Collaborators & Contact===

Author: {executableAuthor}

Contributors:

Contact: name, email

===Module Description===
{tblStart} style="color:green" border="1"
|Program title || {executableTitle}
|-
|Program description || {executableDescription}
|-
|Program version || {executableVersion}
|-
|Program documentation-url || {executableDocumentationURL}
|-
{tblStop}

== Usage ==

===Use Cases, Examples===

This module is especially appropriate for these use cases:

* Use Case 1:
* Use Case 2:

Examples of the module in use:

* Example 1:
* Example 2:

===Tutorials===
* Tutorial 1
** Data Set 1
"""

    stringDict = dict({})
    stringDict['executableTitle'] = getThisNodesInfoAsText(
        executableNode, "title")
    stringDict['executableCategory'] = getThisNodesInfoAsText(
        executableNode, "category")
    stringDict['executableAuthor'] = getThisNodesInfoAsText(
        executableNode, "contributor")
    stringDict['executableDescription'] = getThisNodesInfoAsText(
        executableNode, "description")
    stringDict['executableVersion'] = getThisNodesInfoAsText(
        executableNode, "version")
    stringDict['executableDocumentationURL'] = getThisNodesInfoAsText(
        executableNode, "documentation-url")
    stringDict['tblStart'] = "{|"  # To avoid subs the entire table as var
    stringDict['tblStop'] = "|}"   # To avoid subs the entire table as var

    outRegion = outputRegionTemplate.format(**stringDict)
    return outRegion


def DumpSEMMediaWikiFeatures(executableNode):
    outRegion = ""
    outRegion += "===Quick Tour of Features and Use===\n\n"
    outRegion += "{0}{1}".format("A list panels in the interface,",
           " their features, what they mean, and how to use them.\n")
    outRegion += "{|\n|\n"
    # Now print all the command line arguments and the labels
    # that showup in the GUI interface
    for parameterNode in executableNode.getElementsByTagName("parameters"):
        outRegion += "* <span style=\"color:blue\">'''''{0}'''''</span>: {1}\n".format(
            getThisNodesInfoAsText(parameterNode, "label"),
            getThisNodesInfoAsText(parameterNode, "description"))
        currentNode = parameterNode.firstChild
        while currentNode is not None:
            if currentNode.nodeType == currentNode.ELEMENT_NODE:
                #If this node doe not have a "label" element, then skip it.
                if getThisNodesInfoAsText(currentNode, "label") != "":
                    # if this node has a default value -- document it!
                    if getThisNodesInfoAsText(currentNode, "default") != "":
                        outRegion += "{0} {1} {2}: {3} {4}\n".format(
                                getLabelDefinition(currentNode),
                                getLongFlagDefinition(currentNode),
                                getFlagDefinition(currentNode),
                                getThisNodesInfoAsText(currentNode,
                                    "description"),
                                getDefaultValueDefinition(currentNode))
                    else:
                        outRegion += "{0} {1} {2}: {3}\n\n".format(
                                getLabelDefinition(currentNode),
                                getLongFlagDefinition(currentNode),
                                getFlagDefinition(currentNode),
                                getThisNodesInfoAsText(currentNode,
                                    "description"))
            currentNode = currentNode.nextSibling

    outRegion += "{0}{1}\n".format("|[[Image:screenshotBlankNotOptional.png|",
            "thumb|280px|User Interface]]")
    outRegion += "|}\n\n"
    return outRegion


def DumpSEMMediaWikiFooter(executableNode):
    # footer
    outRegionTemplate = """
== Development ==

===Notes from the Developer(s)===

Algorithms used, library classes depended upon, use cases, etc.

===Dependencies===

Other modules or packages that are required for this module's use.

===Tests===

On the [http://www.cdash.org/CDash/index.php?project=Slicer3 Dashboard], these tests verify that the module is working on various platforms:

* MyModuleTest1 [http://viewvc.slicer.org/viewcvs.cgi/trunkMyModuleTest1.cxx]
* MyModuleTest2 [http://viewvc.slicer.org/viewcvs.cgi/trunk MyModuleTest2.cxx]

===Known bugs===

Links to known bugs in the Slicer3 bug tracker

* [http://www.na-mic.org/Bug/view.php?id=000 Bug 000:description]

===Usability issues===

Follow this [http://na-mic.org/Mantis/main_page.php link] to
the Slicer3 bug tracker. Please select the '''usabilityissue category''' when browsing or contributing.

===Source code & documentation===

Links to the module's source code:

Source code:
*[http://viewvc.slicer.org/viewcvs.cgi/trunk file.cxx ]
*[http://viewvc.slicer.org/viewcvs.cgi/trunk file.h ]

Doxygen documentation:
*[http://www.na-mic.org/Slicer/Documentation/Slicer3-doc/html/classes.html class1]

== More Information ==

===Acknowledgment===

{executableAcknowledgment}

===References===

Publications related to this module go here. Links to pdfs would be useful.
"""

    stringDict = dict({})
    stringDict['executableAcknowledgment'] = getThisNodesInfoAsText(executableNode, "acknowledgements")
    outRegion = outRegionTemplate.format(**stringDict)
    return outRegion


def SEMToMediaWikiProg():
    from optparse import OptionParser
    usage = "%prog -x XMLFILE -o MEDIWIKIFILE"
    version = "%prog v0.1"
    parser = OptionParser()
    parser.add_option("-x", "--xmlfile", dest="xmlfilename",
        action="store", type="string",
        metavar="XMLFILE", help="The SEM formatted XMLFILE file")
    parser.add_option("-o", "--outfile", dest="outfilename",
        action="store", type="string", default=None,
        metavar="MEDIAWIKIFILE",
        help="The MEDIAWIKIFILE ascii file with media-wiki formatted text.")
    parser.add_option("-p", "--parts", dest="parts",
        action="store", type="string", default="hbf",
        help="The parts to print out, h=Header,b=body,f=footer")
    parser.epilog = program_description
#    print program_description
    (options, args) = parser.parse_args()

#    It may be desirable in the future to automatically push information to the
#    WIKI page without having to copy and paste.
#    http://python-wikitools.googlecode.com/svn/trunk/README

    ExecutableNode = GetSEMDoc(options.xmlfilename)

    docString = ""
    for stage in options.parts:
        if stage == "h":
            docString += DumpSEMMediaWikiHeader(ExecutableNode)
        elif stage == "b":
            docString += DumpSEMMediaWikiFeatures(ExecutableNode)
        elif stage == "f":
            docString += DumpSEMMediaWikiFooter(ExecutableNode)
        else:
            parser.error(
                "The only valid options are [h|b|f]: Given {0}".format(
                    stage))

    if options.xmlfilename is not None:
        outfile = open(options.outfilename, 'w')
        outfile.write(docString)
        outfile.close()
    else:
        sys.stdout.write(docString)

if __name__ == '__main__':
    SEMToMediaWikiProg()
