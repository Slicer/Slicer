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


def getThisNodesInfoAsTextTableLine(executableNode, label):
    r"""
    Create a formatted text string suitable for inclusion in a MediaWiki table
    """
    labelNodeList = executableNode.getElementsByTagName(label)
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        line = "|Program {0} || {1}\n {2}".format(label,
            getTextValuesFromNode(labelNode.childNodes), "|-")
        return line


def getThisNodesInfoAsText(executableNode, label):
    r"""
    Only get the text info for the matching label at this level of the tree
    """
    labelNodeList = executableNode.getElementsByTagName(label)
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return getTextValuesFromNode(labelNode.childNodes)
    return ""


def getLongFlagDefinition(executableNode):
    r"""
    Extract the long flag, and color the text string
    """
    labelNodeList = executableNode.getElementsByTagName("longflag")
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return "{0}{1}{2}".format("[<span style=\"color:orange\">--",
            getTextValuesFromNode(labelNode.childNodes), "</span>]")
    return ""


def getFlagDefinition(executableNode):
    r"""
    Extract the (short) flag, and color the text string
    """
    labelNodeList = executableNode.getElementsByTagName("flag")
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return "{0}{1}{2}".format("[<span style=\"color:pink\">-",
            getTextValuesFromNode(labelNode.childNodes), "</span>]")
    return ""


def getLabelDefinition(executableNode):
    r"""
    Extract the nodes label, and color the text string
    """
    labelNodeList = executableNode.getElementsByTagName("label")
    if labelNodeList.length > 0:
        labelNode = labelNodeList[0]  # Only get the first one
        return "{0}{1}{2}".format("** <span style=\"color:green\">'''",
            getTextValuesFromNode(labelNode.childNodes), "'''</span>")
    return ""


def getDefaultValueDefinition(executableNode):
    r"""
    Extract the default value
    """
    labelNodeList = executableNode.getElementsByTagName("default")
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
    executableNode = doc.getElementsByTagName("executable")[0]
    return executableNode


def DumpSEMMediaWikiHeader(executableNode, outfile):
    r"""
    Just dump the header section of the MediaWikiPage
    """
    outfile.write("__NOTOC__\n\n")
    outfile.write("==={0}===\n".format(
            getThisNodesInfoAsText(executableNode, "title")))
    outfile.write("{0}\n".format(
            getThisNodesInfoAsText(executableNode, "title")))

    outfile.write("{|\n")
    outfile.write("{0}{1}".format("|[[Image:screenshotBlankNotOptional.png",
            "|thumb|280px|User Interface]]\n"))
    outfile.write("|[[Image:screenshotBlank.png|thumb|280px|Output]]\n")
    outfile.write("|[[Image:screenshotBlank.png|thumb|280px|Caption]]\n")
    outfile.write("|}\n")

    # Print information about the program in general
    outfile.write("== General Information ==\n\n")
    outfile.write("===Module Type & Category===\n\n")
    outfile.write("Type: CLI\n\n")
    outfile.write("Category: {0}\n".format(
            getThisNodesInfoAsText(executableNode, "category")))
    outfile.write("\n\n")

    outfile.write("===Authors, Collaborators & Contact===\n\n")
    outfile.write(getThisNodesInfoAsText(
            executableNode, "contributor").format("Author: {0}\n"))
    outfile.write("Contributors: \n\n")
    outfile.write("Contact: name, email\n")
    outfile.write("\n\n")

    outfile.write("===Module Description===\n")
    outfile.write("{| style=\"color:green\" border=\"1\"\n")
    outfile.write("{0}".format(
            getThisNodesInfoAsTextTableLine(executableNode, "title")))
    outfile.write("{0}".format(
            getThisNodesInfoAsTextTableLine(executableNode, "description")))
    outfile.write("{0}".format(
            getThisNodesInfoAsTextTableLine(executableNode, "version")))
    outfile.write("{0}".format(
            getThisNodesInfoAsTextTableLine(executableNode,
                "documentation-url")))
    outfile.write("{0}".format(
            getThisNodesInfoAsTextTableLine(executableNode,
                "doesnotexiststest")))
    outfile.write("|}\n\n")

    outfile.write("== Usage ==\n\n")

    outfile.write("===Use Cases, Examples===\n\n")

    outfile.write("{0}{1}".format("This module is especially appropriate ",
            "for these use cases:\n\n"))

    outfile.write("* Use Case 1:\n")
    outfile.write("* Use Case 2:\n\n")

    outfile.write("Examples of the module in use:\n\n")

    outfile.write("* Example 1:\n")
    outfile.write("* Example 2:\n\n")

    outfile.write("===Tutorials===\n")

    outfile.write("* Tutorial 1\n")
    outfile.write("** Data Set 1\n\n")


def DumpSEMMediaWikiFeatures(executableNode, outfile):
    outfile.write("===Quick Tour of Features and Use===\n\n")
    outfile.write("{0}{1}".format("A list panels in the interface,",
           " their features, what they mean, and how to use them.\n"))
    outfile.write("{|\n")
    outfile.write("|\n")
    # Now print all the command line arguments and the labels
    # that showup in the GUI interface
    for parameterNode in executableNode.getElementsByTagName("parameters"):
        outfile.write(
            "* <span style=\"color:blue\">'''''{0}''''' </span>\n".format(
                getThisNodesInfoAsText(parameterNode, "label")))
        currentNode = parameterNode.firstChild
        while currentNode is not None:
            if currentNode.nodeType == currentNode.ELEMENT_NODE:
                #If this node doe not have a "label" element, then skip it.
                if getThisNodesInfoAsText(currentNode, "label") != "":
                    # if this node has a default value -- document it!
                    if getThisNodesInfoAsText(currentNode, "default") != "":
                        outfile.write("{0} {1} {2}: {3} {4}\n".format(
                                getLabelDefinition(currentNode),
                                getLongFlagDefinition(currentNode),
                                getFlagDefinition(currentNode),
                                getThisNodesInfoAsText(currentNode,
                                    "description"),
                                getDefaultValueDefinition(currentNode)))
                    else:
                        outfile.write("{0} {1} {2}: {3}\n\n".format(
                                getLabelDefinition(currentNode),
                                getLongFlagDefinition(currentNode),
                                getFlagDefinition(currentNode),
                                getThisNodesInfoAsText(currentNode,
                                    "description")))
            currentNode = currentNode.nextSibling

    outfile.write("{0}{1}\n".format("|[[Image:screenshotBlankNotOptional.png|",
            "thumb|280px|User Interface]]"))
    outfile.write("|}\n\n")


def DumpSEMMediaWikiFooter(executableNode, outfile):
    # footer
    outfile.write("== Development ==\n\n")
    outfile.write("===Notes from the Developer(s)===\n\n")
    outfile.write("{0}{1}".format("Algorithms used, library classes",
           " depended upon, use cases, etc.\n\n"))
    outfile.write("===Dependencies===\n\n")
    outfile.write("{0}{1}".format(
        "Other modules or packages that are required ",
        "for this module's use.\n\n"))
    outfile.write("===Tests===\n\n")
    outfile.write("{0}{1}{2}{3}".format("On the ",
        "[http://www.cdash.org/CDash/index.php?project=Slicer3 Dashboard], ",
        "these tests verify that the module is working on various ",
        "platforms:\n\n"))
    outfile.write("{0}{1}".format(
            "* MyModuleTest1 [http://viewvc.slicer.org/viewcvs.cgi/trunk",
            "MyModuleTest1.cxx]\n"))
    outfile.write("{0}{1}".format(
            "* MyModuleTest2 [http://viewvc.slicer.org/viewcvs.cgi/trunk",
            " MyModuleTest2.cxx]\n\n"))
    outfile.write("===Known bugs===\n\n")
    outfile.write("Links to known bugs in the Slicer3 bug tracker\n\n")
    outfile.write("{0}{1}".format(
            "* [http://www.na-mic.org/Bug/view.php?id=000 Bug 000:",
            "description] \n\n"))
    outfile.write("===Usability issues===\n\n")
    outfile.write("{0}{1}{2}".format(
            "Follow this [http://na-mic.org/Mantis/main_page.php link] to",
            "the Slicer3 bug tracker. Please select the '''usability",
            "issue category''' when browsing or contributing.\n\n"))
    outfile.write("===Source code & documentation===\n\n")
    outfile.write("Links to the module's source code:\n\n")
    outfile.write("Source code:\n")
    outfile.write("*[http://viewvc.slicer.org/viewcvs.cgi/trunk file.cxx ]\n")
    outfile.write("*[http://viewvc.slicer.org/viewcvs.cgi/trunk file.h ]\n\n")
    outfile.write("Doxygen documentation:\n")
    outfile.write("{0}{1}".format(
            "*[http://www.na-mic.org/Slicer/Documentation/Slicer3-doc/",
            "html/classes.html class1]\n\n"))
    outfile.write("== More Information == \n\n")
    outfile.write("===Acknowledgment===\n\n")
    outfile.write("{0}\n".format(
            getThisNodesInfoAsText(executableNode, "acknowledgements")))
    outfile.write("===References===\n\n")
    outfile.write("{0}{1}".format(
            "Publications related to this module go here. Links to pdfs",
            "would be useful.\n"))


def DumpSEMMediaWikiAll(ExecutableNode, outfile):
    DumpSEMMediaWikiHeader(ExecutableNode, outfile)
    DumpSEMMediaWikiFeatures(ExecutableNode, outfile)
    DumpSEMMediaWikiFooter(ExecutableNode, outfile)


if __name__ == '__main__':
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
    parser.epilog = program_description
#    print program_description
    (options, args) = parser.parse_args()

#    It may be desirable in the future to automatically push informaiton to the
#    WIKI page without having to copy and paste.
#    http://python-wikitools.googlecode.com/svn/trunk/README

    ExecutableNode = GetSEMDoc(options.xmlfilename)
    if options.xmlfilename != None:
        outfile = open(options.outfilename, 'w')
    else:
        outfile = sys.stdout

    DumpSEMMediaWikiAll(ExecutableNode, outfile)
    if options.xmlfilename != None:
        outfile.close()
