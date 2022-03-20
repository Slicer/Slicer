# Style Guide

## General

If you are editing code, take a few minutes to look at the code around you and determine its style.
If they use spaces around their if clauses, you should, too.
If their comments have little boxes of stars around them, make your comments have little boxes of stars around them too.

The point of having style guidelines is to have a common vocabulary of coding so people can concentrate on what you are saying, rather than on how you are saying it. We present global style rules here so people know the vocabulary. But local style is also important. If code you add to a file looks drastically different from the existing code around it, the discontinuity throws readers out of their rhythm when they go to read it. Try to avoid this.

Slicer uses and extends libraries and toolkits multiple languages, which each has its own conventions. As a general rule, follow these conventions. For example, to get number of items in a VTK class use GetNumberOfXXX, but use XXXCount in a Qt class.

Line length: Preferably keep lines shorter than 80 characters. Always keep lines shorter than 120 characters. Markdown (.md) files are excluded from this restriction, to allow editing the text without worrying about line breaks.

### Toolkits and libraries

- [VTK coding conventions](https://www.vtk.org/Wiki/VTK_Coding_Standards)
- [Qt style guide](https://wiki.qt.io/Qt_Coding_Style)
- [Python style guide](https://www.python.org/dev/peps/pep-0008/)

### Languages

#### Python

- Indentation is 2 spaces per level for consistency with the rest of the code base. This may be revisited in the future. Do not use tabs.
- Text encoding: UTF-8 is preferred, Latin-1 is acceptable
- Comparisons:
  - To singletons (e.g. None): use 'is' or 'is not', never equality operations.
  - To Booleans (True, False): don't ever compare with True or False (for further explanation, see PEP 8).
- Prefix class definitions with two blank lines
- Imports
  - Grouped in order of scope/commonallity
    - Standard library imports
    - Related third party imports
    - Local apps/library specific imports
      - Slicer application imports and local/module imports may be grouped independently.
  - One package per line (with or without multiple function/module/class imports from the package)
- Avoid extraneous whitespaces
- Naming conventions: when [PEP 8](https://www.python.org/dev/peps/pep-0008/#package-and-module-names) and Slicer naming conventions conflict, Slicer wins.

#### C++

- Use the old-style VTK indentation (braces are in new line, indented by two spaces), until the entire Slicer code base will be updated to use current VTK indentation style.
- Use VTK naming conventions:
  - Local variable should start with a lower case.
    Use:
    `void vtkSlicerSliceLogic::SetForegroundLayer(vtkSlicerSliceLayerLogic *foregroundLayer)`
    Instead of:
    ` void vtkSlicerSliceLogic::SetForegroundLayer(vtkSlicerSliceLayerLogic *ForegroundLayer) // wrong!`
  - Member variable should start with a capital letter, and in implementation should be used in conjunction with `this->` convention.

Example:

```
class Node
{
  Object &Foo();
  Object Bla;
};
Object& Node::Foo()
{
  return this->Bla;
}
```

Useful information about some coding style decisions: <https://google.github.io/styleguide/cppguide.html>

#### CMake

- Macros/functions should be lower case and words separated with underscores

  `include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Logic)` instead of `INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Logic)`

  `find_package(VTK REQUIRED)` instead of `FIND_PACKAGE(VTK REQUIRED)` or `Find_Package(VTK REQUIRED)`

- Global variables are uppercase and words separated with underscores

  `CMAKE_CURRENT_SOURCE_DIR` instead of `cmake_current_source_dir`

- Local variables are lowercase and words separated with underscores

  `foreach(file ${FILES_TO_CONFIGURE})...` instead of `foreach(FILE ${FILES_TO_CONFIGURE})...`

## Naming conventions

- Acronyms should be written with the same case for each letter (all uppercase or all lowercase).
  - `RASToSlice` not `RasToSlice`
  - `vtkMRML` not `vtkMrml`
  - `vtkSlicer` not `vTKSlicer`
- Words should be spelled out and not abbreviated
  - `GetWindow` not `GetWin`
- File names must follow the [https://en.wikipedia.org/wiki/CamelCase Camel case] convention
  - `TestMyFeature.cxx` not `Test-My_Feature.cxx`
- Use US English words and spelling
  - "Millimeter" not "Millimetre"
  - "Color" not "Colour"

## Comments

- Include extensive comments in the header files
- Keep the comments up to date as the code changes
- Use the keyword <code>\todo</code> to flag spots in the code that need to be revisited
- Do not leave blocks of commented out code in the source file -- if needed insert links to prior svn versions as comments

## Functions

### Don't mix different levels of abstraction

Examples:

When dealing with files names and path, use:
- [kwsys::SystemTools](https://github.com/Kitware/VTK/blob/master/Utilities/KWSys/vtksys/SystemTools.hxx.in) in VTK classes
- [QFileInfo](https://doc.qt.io/qt-5/qfileinfo.html)/[QDir](https://doc.qt.io/qt-5/qdir.html) in Qt classes
- [https://docs.python.org/library/os.path.html os.path] in Python.

Instead of doing string manipulation manually:

    QString filePath = directoryPath + "/" + fileName + ".exe"

Prefer instead:

- VTK:

    SystemTools::JoinPath(), SystemTools::GetFilenameName()...

- Qt:

    QFileInfo(QDir directory, QString fileName), QFileInfo::suffix(), QFileInfo::absoluteFilePath()...

- Python:

    os.path.join(), os.path.splitext(), os.path.abspath()...

References: [Clean Code](https://www.amazon.com/gp/product/0132350882?ie=UTF8&tag=solisyntprog-20&linkCode=as2&camp=1789&creative=9325&creativeASIN=0132350882) from `Robert C. Martin`: `Mixing levels of abstraction within a function is always confusing. Readers may not be able to tell whether a particular expression is an essential concept or a detail. Worse, like broken windows, once details are mixed with essential concepts, more and more details tend to accrete within the functions.`

2. Use STL where you can, but:
- In VTK classes follow the [https://www.vtk.org/Wiki/VTK_FAQ#Can_I_use_STL_with_VTK.3F VTK guidelines]
  - Note that a [vtkCollection](https://www.vtk.org/doc/nightly/html/classvtkCollection.html) is somewhat equivalent to `std::list<vtkSmartPointer<vtkObject*> >`
- In Qt classes prefer [Qt Container classes](https://doc.qt.io/qt-5/containers.html)

## File layout

### Includes

- Only include the necessary files, no more.
- Group includes per library
- Alphabetically sort files within groups
- Order groups from local to global: for example, module then MRML then CTK then Qt then VTK then ITK then STL.
- Implementation files should include the header files first

Example:

```
 // header
 // ...
 // end header

 #include "qSlicerMyModule.h"

 // MyModule includes
 #include "qSlicerMyModuleWidget.h"
 #include "vtkSlicerMyModuleLogic.h"

 // MRML includes
 #include "vtkMRMLScene.h"

 // Qt includes
 #include <QDialog>

 // VTK includes
 #include <vtkSmartPointer.h>

 // STD includes
 #include <vector>
```


## Library Dependencies

- MRML classes should only depend on vtk and itk (not Slicer Logic or Qt)
- Logic classes depend on MRML to store state
- Logic classes should encapsulate vtk and itk pipelines to accomplish specific slicer tasks (such as resampling volumes for display)
- GUI classes can depend on MRML and Logic and Qt

## Development Practices

While developing code, enable VTK_DEBUG_LEAKS (ON by default) in your vtk build and be sure to clean up any leaks that arise from your contributions.

## Coordinate Systems

- World space for 3D Views is in RAS (Right Anterior Superior) space. See [[Coordinate systems]].
- All units are expressed in Millimeters (mm)

## String encoding: UTF-8 everywhere

Slicer uses [UTF-8 everywhere](https://utf8everywhere.org/): all strings in `std::string`, `char[]` arrays, files, etc. are in UTF-8 (except in rare exceptions where this is very clearly indicated). We don't use code pages or any other unicode encoding. If this leads to incorrect behavior anywhere then the underlying issue must be fixed (e.g., if a VTK function does not work correctly with UTF-8 encoded string input then a fix has to be submitted to VTK).

On Windows, process code page of all Slicer executables (main application, CLI modules, tests, etc.) are explicitly set to UTF-8 by using "ctk_add_executable_utf8" function instead of plain "add_executable" in CMake. This makes all standard API functions to use UTF-8 encoding, even in third-party libraries. This mechanism requires Windows Version 1903 (May 2019 Update) or later. On Linux and Mac, encoding is already expected to be UTF-8 (it is currently not checked or enforced in any way).

Conversion from std::string to QString:

```
std::string ss = ...
QString qs1 = QString::fromUtf8(ss); // this is slightly preferred, as it is very clear and explicit
QString qs2 = QString(ss); // same result as fromUtf8, acceptable, as it is a bit simpler and used throughout the code base anyway
```

Conversion from QString to std::string:

```
QString qs = ...
std::string ss  = QString::toUtf8(qs);
```

Printing to console: in general, VTK, Qt, or ITK logging macros are preferred but if for some reason text must be printed on console then use qPrintable macro. This macro converts the string

```
std::cerr << "Failed to create file " << qPrintable(filePath) << std::endl;
```

Qt logging macros (qDebug, qWarning, QFatal): these macros expect UTF-8 encoded strings, therefore do not use qPrintable macro.

File management: All filenames have to be passed to file functions (such as fopen) must be UTF-8 encoded. All text file content is expected to be UTF-8 encoded, except very rare cases when a different encoding is explicitly specified in the file (for example in incoming DICOM files may use different encoding).

## Error and warning messages

The ITK, VTK, Qt, std::cout, std::cerr .. all appear in the error log and can easily be filtered according to their type (debug/warning/error).

- **Errors**: Error should be used to signal something that should not happen. They usually mean that the execution of the current function/code should be stopped.
- **Warnings**: Warning should be used to signal potentially dangerous behavior. Also consider using these in deprecated methods to warn your fellow developers.
- **Debugs**: For general debug and developer aimed information, one can use the debug messages.

### In Qt-based classes

For error messages, use [qCritical()](https://qt-project.org/doc/qt-4.8/qtglobal.html#qCritical):

```
if (somethingWrongHappened)
  {
  qCritical() << "I encountered an error";
  return;
  }
```

For warnings, use [qWarning()](https://qt-project.org/doc/qt-4.8/qtglobal.html#qWarning):

```
qWarning() << "Be careful here, this is dangerous";
```

For debug, use [qDebug()](https://qt-project.org/doc/qt-4.8/qtglobal.html#qDebug):

```
qDebug() << "This variable has the value: "<< value;
```

### In VTK-based classes

- For error messages, use [vtkErrorMacro()](https://www.vtk.org/doc/release/3/html/vtkSetGet_8h.html):

```
if (somethingWrongHappened)
  {
  vtkErrorMacro("I encountered an error");
  return;
  }
```

For warnings, use [vtkWarningMacro()](https://www.vtk.org/doc/release/3/html/vtkSetGet_8h.html):

```
vtkWarningMacro("Be careful here, this is dangerous");
```

For debug, use [vtkDebugMacro()](https://www.vtk.org/doc/release/3/html/vtkSetGet_8h.html):

```
vtkDebugMacro("This variable has the value: "<< value);
```

## Commits

- Separate the subject from body with a blank line
- Limit the subject line to 50 characters
- Capitalize the subject line
- Do not end the subject line with a period
- Use the imperative mood in the subject line
- Wrap the body at 72 characters
- Use the body to explain what and why vs. how. If there was important/useful/essential conversation or information, copy or include a reference
- Prefix the commit message title with "BUG:", "COMP:", "DOC:", "ENH:", "STYLE:". Note the ':' (colon) character.
- When possible, one keyword to scope the change in the subject (i.e. "STYLE: README: ...", "BUG: vtkMRMLSliceLogic: ...")

### Commit message prefix

Subversion Commits to Slicer require commit type in the comment.

Valid commit types are:

```
   BUG:   - a change made to fix a runtime issue
            (crash, segmentation fault, exception, or incorrect result,
   COMP:  - a fix for a compilation issue, error or warning,
   DOC:   - a documentation change,
   ENH:   - new functionality added to the project,
   PERF:  - a performance improvement,
   STYLE: - a change that does not impact the logic or execution of the code.
            (improve coding style, comments).
```

Note that the ':'(colon) directly follows the commit tag. For example, it is: "STYLE:" not "STYLE :"

### Message content

A good commit message title (first line) should **explain what the commit does for the user, not ''how'' it is done**. *How* can be explained in the body of the commit message (if looking at the code of the commit is not self explanatory enough).

Examples:
- Bad: `BUG: Check pointer validity before dereferencing` -> *implementation detail*, *self-explanatory* (by looking at the code)
- Good: `BUG: Fix crash in Module X when clicking Apply button`
- Bad: `ENH: More work in qSlicerXModuleWidget` -> `more work` is *too vague*, `qSlicerXModuleWidget` is too *low level*
- Good: `ENH: Add float image outputs in module X`
- Bad: `COMP: Typo in cmake variable` -> *implementation detail*, *self-explanatory*
- Good: `COMP: Fix compilation error with Numpy on Visual Studio`

If the commit is related to an [issue](https://issues.slicer.org/) (bug or feature request), you can mention it at the end of the message body by preceding the issue number with a `#` (pound) character:

```
BUG: Fix crash in Volume Rendering module when switching view layout

vtkSetAndObserveMRMLNodeEventsMacro can't be used for observing all types of vtkObjects,
only vtkMRMLNode is expected by vtkMRMLAbstractLogic::OnMRMLNodeModified(...)

see #1641
```

Where `1641` refers to the [issue number](https://github.com/Slicer/Slicer/issues/1641) in the issue tracker. Use `see` before the issue number to refer to an issue. If the commit fixes an issue and no further testing is needed then `fixed #1641` can be added to the body, which automatically closes the issue when merged.

### Importing changes from external project/repository

When you update the git tag of any external project, explain in the commit message what the update does instead of just mentioning that an update in made.

This will avoid having a Slicer commit history made of uninformative messages such as:

```
r19180 - ENH: Update git tag
r19181 - BUG: Update svn revision
r19182 - ENH: revision updated
```

Ideally it should be the same message than the commit(s) in the external repository.

Read [Project forks page](https://www.slicer.org/wiki/Documentation/Nightly/Developers/ProjectForks) for an exhaustive list of recommendations.

Example:

```
COMP: Update MultiVolumeExplorer to fix unused-local-typedefs warnings

$ git shortlog 17a9095..d68663f --no-merges
Jean-Christophe Fillion-Robin (1):
      COMP: Fix unused-local-typedefs warnings
```

See [r23377](https://github.com/Slicer/Slicer/commit/3e04040d2e960ec4cd294cb8404169f16cfe0656).

### Resources

- Read more on [https://chris.beams.io/posts/git-commit/ How to Write a Git Commit Message]
- Discussion section of [https://git-scm.com/docs/git-commit git-commit(1)]

## UI Design Guidelines

See [UI Style Guide](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Style_Guide/UI).

## Logging

The following log levels are used in Slicer:
- Error: detected errors, conditions that should never occur
- Warning: potential errors, possible computation inaccuracies
- Info: important events, application state changes (helps to determine what the steps lead to a certain error or warning); one user action should not generate more than 1-2 info level messages
- Debug: any information that may be useful for debugging and troubleshooting

In VTK classes:
- `vtkErrorMacro("vtkMRMLClipModelsNode:: Invalid Clip Type");`
- `vtkWarningMacro("Model " << modelNode->GetName() << "'s display node is null\n");`
- `vtkDebugMacro("CreateWidget: found a glyph type already defined for this node: " << iter->second);`

In QT classes:
- `qCritical() << "qSlicerUtils::setPermissionsRecursively: Failed to set permissions on file" << info.filePath();`
- `qWarning() << "qSlicerIOManager::openScreenshotDialog: Unable to get Annotations module (annotations), using the CTK screen shot dialog.";`
- `qDebug() << "qMRMLSceneFactoryWidget::deleteNode(" <<className <<") no node";`

In Python:
- `logging.error("This is an error message. It is printed on the console (to standard error) and to the application log.")`
- `logging.warning("This is a warning message. It is printed on the console (to standard error) and to the application log.")`
- `logging.info("This is an information message. It is printed on the console (to standard output) and to the application log.")`
- `logging.debug("This is a debug message. It is only recorded in the application log but not displayed in the console. File name and line number is added to the log record.")`
