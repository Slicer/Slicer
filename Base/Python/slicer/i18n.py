import os
import inspect


def translate(context, text):
    """Translate message to the current application language.
    This `translate(context, message)` function is recognized by Qt's lupdate tool.
    See example use in DICOM module.
    """
    from slicer import app

    return app.translate(context, text)


def getContext(sourceFile):
    """Get the translation context name.

    The context name is constructed from the Python package name (name of the parent folder that contains `__init__.py` file - if it exists)
    and the source file name (without the `.py` extension).

    Most Slicer modules do not have a `__init__.py` file in their folder, so the context name is simply the source file name
    (for example, `DICOMEnhancedUSVolumePlugin`).

    Most helper Python scripts in Slicer are Python packages (subfolders containing addition Python scripts and an `__init__.py` file)
    and their name is constructed as PythonPackageName.SourceFileName (for example, `SegmentEditorEffects.SegmentEditorDrawEffect`).
    """
    if os.path.isfile(sourceFile):
        parentFolder = os.path.dirname(sourceFile)
        init_file_path = parentFolder + os.path.sep + "__init__.py"

        if os.path.isfile(init_file_path):
            context_name = os.path.basename(parentFolder)
            context_name += "." + os.path.basename(sourceFile).replace(".py", "")
            return context_name
        else:
            return os.path.basename(sourceFile).replace(".py", "")
    else:
        return os.path.basename(sourceFile)


def tr(text):
    """Translation function for python scripted modules that automatically determines context name.

    This is more convenient to use than `translate(context, text)` because the developer does not need to manually specify the context.
    This function is typically imported as `_` function.

    Example::

      from slicer.i18n import tr as _
      ...
      statusText = _("Idle") if idle else _("Running")

    """
    filename = inspect.stack()[1][1]
    contextName = getContext(filename)
    return translate(contextName, text)
