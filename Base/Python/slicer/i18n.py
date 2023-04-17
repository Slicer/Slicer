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
    """Get the translation context based on the source file name."""
    if os.path.isfile(sourceFile):
        parentFolder = os.path.dirname(sourceFile)
        init_file_path = parentFolder + os.path.sep + '__init__.py'

        if os.path.isfile(init_file_path):
            context_name = os.path.basename(parentFolder)
            context_name += '.' + os.path.basename(sourceFile).replace('.py', '')
            return context_name
        else:
            return os.path.basename(sourceFile).replace('.py', '')
    else:
        return os.path.basename(sourceFile)


def tr(text):
    """Translation function for python scripted modules that gets context name from filename.
    Experimental, not used yet."""
    filename = inspect.stack()[1][1]
    contextName = getContext(filename)
    return translate(contextName, text)
