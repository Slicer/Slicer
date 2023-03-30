import os
import inspect


def translate(context, text):
    """Translate message to the current application language.
    This `translate(context, message)` function is recognized by Qt's lupdate tool.
    See example use in DICOM module.
    """
    from slicer import app
    return app.translate(context, text)


def get_context(source_file):
    """Get the translation context based on the source file name.
    Experimental, not used yet."""
    if os.path.isfile(source_file):
        parent_folder = os.path.dirname(source_file)
        init_file_path = parent_folder + os.path.sep + '__init__.py'

        if os.path.isfile(init_file_path):
            context_name = os.path.basename(parent_folder)
            context_name += '.' + os.path.basename(source_file).replace('.py', '')
            return context_name
        else:
            return os.path.basename(source_file).replace('.py', '')
    else:
        return os.path.basename(source_file)


def tr(text):
    """Translation function for python scripted modules that gets context name from filename.
    Experimental, not used yet."""
    filename = inspect.stack()[1][1]
    context_name = get_context(filename)
    return translate(context_name, text)
