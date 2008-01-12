
XML = """<?xml version="1.0" encoding="utf-8"?>
<executable>

  <category>Python Modules</category>
  <title>Python Script</title>
  <description>
Run external Python code on a surface or image.
</description>
  <version>1.0</version>
  <documentation-url></documentation-url>
  <license></license>
  <contributor>Luca Antiga and Daniel Blezek</contributor>

  <parameters>
    <label>Python Script Parameters</label>
    <description>Parameters for Python script</description>
    <file>
      <name>scriptFileName</name>
      <longflag>scriptFileName</longflag>
      <description>File containing Python code to run</description>
      <label>Script file</label>
    </file>
  </parameters>

  <parameters>
    <label>IO</label>
    <description>Input/output parameters</description>

    <geometry>
      <name>inputSurface</name>
      <longflag>inputSurface</longflag>
      <label>Input Surface</label>
      <channel>input</channel>
      <description>Input surface to be filtered</description>
    </geometry>

    <image>
      <name>inputVolume</name>
      <longflag>inputVolume</longflag>
      <label>Input Volume</label>
      <channel>input</channel>
      <description>Input image to be filtered</description>
    </image>

    <geometry>
      <name>outputSurface</name>
      <longflag>outputSurface</longflag>
      <label>Output Surface</label>
      <channel>output</channel>
      <description>Output filtered surface</description>
    </geometry>

    <image>
      <name>outputVolume</name>
      <longflag>outputVolume</longflag>
      <label>Output Volume</label>
      <channel>output</channel>
      <description>Output filtered volume</description>
    </image>


  </parameters>

</executable>
"""


def Execute (scriptFileName="", inputSurface="", inputVolume="", outputSurface="", outputVolume=""):

    Slicer = __import__("Slicer")
    slicer = Slicer.Slicer()
    scene = slicer.MRMLScene

    if inputSurface:
        inputSurface = scene.GetNodeByID(inputSurface)

    if inputVolume:
        inputVolume = scene.GetNodeByID(inputVolume)

    if outputSurface:
        outputSurface = scene.GetNodeByID(outputSurface)

    if outputVolume:
        outputVolume = scene.GetNodeByID(outputVolume)

    try:
        execfile(scriptFileName)
    except Exception, error:
        slicer.Application.ErrorMessage("Python script error: %s" % error)

    return

