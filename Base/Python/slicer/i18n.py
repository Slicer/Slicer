import inspect
import logging
import os
import re

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

    def findBracedStrings(text):
        """Get all placeholders (replacement fields) in the input format string text.

        All placeholders delimited by curly braces are returned except the ones enclosed in
        double-braces.

        See https://docs.python.org/3/library/string.html#formatstrings
        """
        pattern = r"(?<!\{)\{([^\{\}]+)\}(?!\})"
        matches = re.findall(pattern, text)
        return matches

    filename = inspect.stack()[1][1]
    contextName = getContext(filename)
    translatedText = translate(contextName, text)

    # Accept the translation only if all placeholders are present in the translated text to prevent runtime errors.
    # For example:
    #   text = f"delete {count} files"
    #   translatedText = f"supprimer {compter} fichiers" (incorrect, because `count` should not have been translated)
    # would fail at runtime with a KeyError when `_("delete {count} files").format(count=numberOfSomeItems)` is called.
    # The check prevents the runtime error: only a warning is logged and the incorrect translation is ignored.
    if set(findBracedStrings(text)) != set(findBracedStrings(translatedText)):
        logging.warning(f"In context '{contextName}', translation of '{text}' to '{translatedText}' is incorrect: placeholders do not match")
        return text

    return translatedText
