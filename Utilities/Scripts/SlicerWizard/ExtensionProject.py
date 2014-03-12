import os
import re

from . import CMakeParser

#-----------------------------------------------------------------------------
def _isCommand(token, name):
  return isinstance(token, CMakeParser.Command) and token.text.lower() == name

#-----------------------------------------------------------------------------
def _trimIndent(indent):
  indent = "\n" + indent
  n = indent.rindex("\n")
  return indent[n:]

#=============================================================================
class ExtensionProject(object):
  """Convenience class for manipulating an extension project.

  This class provides an additional layer of convenience for users that wish to
  manipulate the CMakeLists.txt of an extension project. The term "build
  script" is used throughout to refer to the CMakeLists.txt so encapsulated.

  Modifications to the script are made to the in-memory, parsed representation.
  Use :meth:`.save` to write changes back to disk.

  This class may be used as a context manager. When used in this manner, any
  changes made are automatically written back to the project's CMakeLists.txt
  when the context goes out of scope.
  """

  _moduleInsertPlaceholder = "# NEXT_MODULE"

  #---------------------------------------------------------------------------
  def __init__(self, path):
    """
    :param path: Top level directory of the extension project.
    :type path: :class:`basestring`
    """

    cmakeFile = os.path.join(path, "CMakeLists.txt")
    if not os.path.exists(cmakeFile):
      raise IOError("CMakeLists.txt not found")

    self._scriptPath = cmakeFile

    with open(cmakeFile) as fp:
      self._scriptContents = CMakeParser.CMakeScript(fp.read())

  #---------------------------------------------------------------------------
  def __enter__(self):
    return self

  #---------------------------------------------------------------------------
  def __exit__(self, exc_type, exc_value, traceback):
    self.save()

  #---------------------------------------------------------------------------
  @property
  def project(self):
    """Name of extension project.

    :type: :class:`str`

    This provides the name of the extension project, i.e. the identifier passed
    to ``project()`` in the extension's build script.
    """

    for t in self._scriptContents.tokens:
      if _isCommand(t, "project") and len(t.arguments):
        return t.arguments[0].text

    raise EOFError("could not find project")

  #---------------------------------------------------------------------------
  def getValue(self, name, default=None):
    """Get value of CMake variable set in project.

    :param name: Name of the variable.
    :type name: :class:`basestring`
    :param default: Value to return if no such variable exists.

    :returns: Value of the variable, or ``default`` if not set.
    :rtype: :class:`str` or ``type(default)``

    :raises:
      :exc:`~exceptions.KeyError` if no such ``name`` is set and ``default`` is
      ``None``.

    This returns the raw value of the variable ``name`` which is set in the
    build script. No substitution is performed (the result may contain
    substitution placeholders like '``${var}``'). If more than one ``set()``
    command sets the same ``name``, the result is the raw argument to the last
    such command. If the value consists of more than one argument, only the
    first is returned.

    If no ``set()`` command sets ``name``, and ``default`` is not ``None``,
    ``default`` is returned. Otherwise a :exc:`~exceptions.KeyError` is raised.
    """

    for t in reversed(self._scriptContents.tokens):
      if _isCommand(t, "set") and len(t.arguments) and \
         t.arguments[0].text == name:
        if len(t.arguments) < 2:
          return None

        return t.arguments[1].text

    if default is not None:
      return default

    raise KeyError("script does not set %r" % name)

  #---------------------------------------------------------------------------
  def setValue(self, name, value):
    """Change value of CMake variable set in project.

    :param name: Name of the variable.
    :type name: :class:`basestring`
    :param value: Value to assign to the variable.
    :type value: :class:`basestring`

    :raises: :exc:`~exceptions.KeyError` if no such ``name`` is set.

    This modifies the build script to set the variable ``name`` to ``value``.
    If more than one ``set()`` command sets the same ``name``, only the first
    is modified. If the value of the modified ``set()`` command has more than
    one argument, only the first is modified.

    The build script must already contain a ``set()`` command which sets
    ``name``. If it does not, a :exc:`~exceptions.KeyError` is raised.
    """

    for t in self._scriptContents.tokens:
      if _isCommand(t, "set") and len(t.arguments) and \
         t.arguments[0].text == name:
        if len(t.arguments) < 2:
          t.arguments.append(CMakeParser.String(text=value, indent=" ",
                                                prefix="\"", suffix="\""))

        else:
          varg = t.arguments[1]
          varg.text = value
          varg.prefix = "\""
          varg.suffix = "\""

        return

    raise KeyError("script does not set %r" % name)

  #---------------------------------------------------------------------------
  def addModule(self, name):
    """Add a module to the build rules of the project.

    :param name: Name of the module to be added.
    :type name: :class:`basestring`

    :raises: :exc:`~exceptions.EOFError` if no insertion point can be found.

    This adds an ``add_subdirectory()`` call for ``name`` to the build script.
    If possible, the new call is inserted immediately before a placeholder
    comment which is designated for this purpose. Otherwise, the new call is
    inserted after the last existing call to ``add_subdirectory()``.
    """

    indent = ""
    after = -1

    for n in xrange(len(self._scriptContents.tokens)):
      t = self._scriptContents.tokens[n]

      if isinstance(t, CMakeParser.Comment) and \
         t.text.startswith(self._moduleInsertPlaceholder):
        indent = t.indent
        after = n
        t.indent = _trimIndent(t.indent)
        break

      if _isCommand(t, "add_subdirectory"):
        indent = _trimIndent(t.indent)
        after = n + 1

    if after < 0:
      raise EOFError("failed to find insertion point for module")

    arguments = [CMakeParser.String(text=name)]
    t = CMakeParser.Command(text="add_subdirectory", arguments=arguments,
                            indent=indent)
    self._scriptContents.tokens.insert(after, t)

  #---------------------------------------------------------------------------
  def save(self, destination=None):
    """Save the project.

    :param destination: Location to which to write the build script.
    :type destination: :class:`basestring` or ``None``

    This saves the extension project CMake script to the specified file:

    .. code-block:: python

      # Open a project
      p = ExtensionProject('.')

      # Set a value in the project
      p.setValue('EXTENSION_DESCRIPTION', 'This is an awesome extension!')

      # Save the changes
      p.save()

    If ``destination`` is ``None``, the CMakeLists.txt file from which the
    project instance was created is overwritten.
    """

    if destination is None:
      destination = self._scriptPath

    with open(destination, "w") as fp:
      fp.write(str(self._scriptContents))
