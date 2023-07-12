# Extension Wizard

## Overview

The Extension Wizard modules provides a graphical interface within Slicer to aid in the creation of Slicer extensions.

## Panels and their use

- **Extension Tools**:
  - **Create Extension**: Create a new extension from a specified template, given a name and destination. The newly created template is automatically selected for editing. 
  - **Select Extension**: Choose an existing extension to edit. If the extension provides scripted modules that are not already loaded, an option to load such modules is provided.

- **Extension Editor**:
  - **Name**: The name of the currently selected extension.
  - **Location**: The location (on disk) of the currently selected extension.
  - **Repository**: If available, the upstream URL of the repository hosting the extension.
  - **Contents**: A tree view showing the file contents of the currently selected extension.
  - **Add Module to Extension**: Create a new module from a specified template, given a name, and add it to the selected extension.
  - **Edit Extension Metadata**: Edit metadata associated with the extension (such as the name, contributors, etc.).

## Settings

The Extension Wizard module provides a settings page, which is accessible via the [Application Settings](/user_guide/settings.md#application-settings).

* **Built-in template path**: If found, displays the location of the built-in templates.
* **Additional template paths**: A list of additional locations containing categorized templates.
* **Additional template paths for &lt;category&gt;**: A list of additional locations containing templates for a particular category (e.g. extensions, modules).

A "template" is a directory containing a collection of files which comprise that template. Additional paths should point to directories which _contain_ such template directories, not the directory of the template itself. A categorized template directory should have directories for one or more categories, which in turn contain templates.

The built-in templates provide an example of the correct layout for a categorized template directory. (Each category directory is in turn an example of a template collection for that category.)

## Contributors

- Matthew Woehlke (Kitware)
- Jean-Christophe Fillion-Robin (Kitware)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NA-MIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Information on NA-MIC can be obtained from the [NA-MIC website](https://www.na-mic.org/).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_kitware.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
