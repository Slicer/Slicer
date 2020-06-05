import os
import re

from . import CMakeParser

from .Utilities import detectEncoding

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

  _referencedVariables = re.compile(r"\$\{([\w_\/\.\+\-]+)\}")

  #---------------------------------------------------------------------------
  def __init__(self, path, encoding=None, filename="CMakeLists.txt", ):
    """
    :param path: Top level directory of the extension project.
    :type path: :class:`str`
    :param encoding: Encoding of extension CMakeLists.txt.
    :type encoding: :class:`str` or ``None``
    :param filename: CMake file to parse. Default is `CMakeLists.txt`.
    :type filename: :class:`str`

    If ``encoding`` is ``None``, the encoding will be guessed using
    :meth:`~SlicerWizard.Utilities.detectEncoding`.
    """
    cmakeFile = os.path.join(path, filename)
    if not os.path.exists(cmakeFile):
      raise IOError("%s not found" % filename)

    self._scriptContents, self._encoding = self._parse(cmakeFile, encoding=encoding)
    try:
      self._scriptPath = cmakeFile
      self.getValue("EXTENSION_HOMEPAGE")
    except KeyError:
      for cmakeFile in self._collect_cmakefiles(path, filename):
        self._scriptContents, self._encoding = self._parse(cmakeFile, encoding=encoding)
        try:
          self._scriptPath = cmakeFile
          self.getValue("EXTENSION_HOMEPAGE")
          break
        except KeyError:
          continue

  @staticmethod
  def _collect_cmakefiles(path, filename="CMakeLists.txt"):
    """Return list of `filename` found in `path` at depth=1"""
    cmakeFiles = []
    dirnames = []
    for _, dirnames, _ in os.walk(path):
      break
    for dirname in dirnames:
      cmakeFile = os.path.join(path, dirname, filename)
      if os.path.exists(cmakeFile):
        cmakeFiles.append(cmakeFile)
    return cmakeFiles

  #---------------------------------------------------------------------------
  @staticmethod
  def _parse(cmakeFile, encoding=None):
    with open(cmakeFile, "rb") as fp:
      contents = fp.read()

      if encoding is None:
        encoding, confidence = detectEncoding(contents)

        if encoding is not None:
          if confidence < 0.5:
            logging.warning("%s: encoding detection confidence is %f:"
                            " project contents might be corrupt" %
                            (path, confidence))

      if encoding is None:
        # If unable to determine encoding, skip unicode conversion... users
        # must not feed any unicode into the script or things will likely break
        # later (e.g. when trying to save the project)
        contents = CMakeParser.CMakeScript(contents)

      else:
        # Otherwise, decode the contents into unicode
        contents = contents.decode(encoding)
        contents = CMakeParser.CMakeScript(contents)

    return contents, encoding

  #---------------------------------------------------------------------------
  def __enter__(self):
    return self

  #---------------------------------------------------------------------------
  def __exit__(self, exc_type, exc_value, traceback):
    self.save()


  #---------------------------------------------------------------------------
  @property
  def encoding(self):
    """Character encoding of the extension project CMakeLists.txt.

    :type: :class:`str` or ``None``

    This provides the character encoding of the CMakeLists.txt file from which
    the project instance was created. If the encoding cannot be determined, the
    property will have the value ``None``.

    .. 'note' directive needs '\' to span multiple lines!
    .. note:: If ``encoding`` is ``None``, the project information is stored \
              as raw bytes using :class:`str`. In such case, passing a \
              non-ASCII :class:`unicode` to  any method or property \
              assignment that modifies the project may  make it impossible to \
              write the project back to disk.
    """

    return self._encoding

  #---------------------------------------------------------------------------
  @property
  def project(self):
    """Name of extension project.

    :type: :class:`str`

    :raises:
      :exc:`~exceptions.EOFError` if no ``project()`` command is present in the
      build script.

    This provides the name of the extension project, i.e. the identifier passed
    to ``project()`` in the extension's build script.

    Assigning the property modifies the build script.
    """

    for t in self._scriptContents.tokens:
      if _isCommand(t, "project") and len(t.arguments):
        return t.arguments[0].text

    # Support older extension that do not call "project(Name)"
    # in top-level CMakeLists.txt
    try:
      return self.getValue("EXTENSION_NAME")
    except KeyError:
      pass

    raise EOFError("could not find project")

  #---------------------------------------------------------------------------
  @project.setter
  def project(self, value):

    for t in self._scriptContents.tokens:
      if _isCommand(t, "project"):
        if len(t.arguments):
          t.arguments[0].text = value
        else:
          t.arguments.append(CMakeParser.String(text=value))

        return

    raise EOFError("could not find project")

  #---------------------------------------------------------------------------
  def substituteVariableReferences(self, text):
    """Return a copy of ``text`` where all valid '``${var}``' occurrences
    have been replaced.

    Note that variable references can nest and are evaluated from the inside
    out, e.g. '``${outer_${inner_variable}_variable}``'.

    :param text: A text with zero or more variable references.
    :type text: :class:`str`
    """

    def _substitue(text):
      variableNames = self._referencedVariables.findall(text)
      if len(variableNames) == 0:
        return text

      prefinedVariables = {}
      try:
         prefinedVariables["PROJECT_NAME"] = self.project
      except EOFError:
        pass

      for name in prefinedVariables.keys():
        try:
          text = text.replace("${%s}" % name, prefinedVariables[name])
        except KeyError:
          continue

      for name in variableNames:
        try:
          text = text.replace("${%s}" % name, self.getValue(name))
        except KeyError:
          text = text.replace("${%s}" % name, "%s-NOTFOUND" % name)

      return text

    while len(self._referencedVariables.findall(text)) > 0:
      text = _substitue(text)
    return text

  #---------------------------------------------------------------------------
  def getValue(self, name, default=None, substitute=False):
    """Get value of CMake variable set in project.

    :param name: Name of the variable.
    :type name: :class:`str`
    :param default: Value to return if no such variable exists.
    :param substitute: If ``True``, expand variable references in value.
    :type substitute: :class:`bool`

    :returns: Value of the variable, or ``default`` if not set.
    :rtype: :class:`str` or ``type(default)``

    :raises:
      :exc:`~exceptions.KeyError` if no such ``name`` is set and ``default`` is
      ``None``.

    This returns the raw value of the variable ``name`` which is set in the
    build script. By default, no substitution is performed (the result is taken
    from the raw argument). If more than one ``set()`` command sets the same
    ``name``, the result is the raw argument to the last such command. If the
    value consists of more than one argument, they are all concatenated together
    while stripping newlines, tabs and extra whitespaces.

    If ``substitute`` is ``True``, each occurrence of '``${var}``' will be
    replaced with the corresponding variable if it has been set. Variable
    references can nest and are evaluated from the inside out,
    e.g. '``${outer_${inner_variable}_variable}``'. If a variable
    reference is not found, it will be replaced with '``<var>-NOTFOUND``'.

    If no ``set()`` command sets ``name``, and ``default`` is not ``None``,
    ``default`` is returned. Otherwise a :exc:`~exceptions.KeyError` is raised.

    .. note::

      Variables set using a nested reference are not supported.
      For example, if the underlying CMake code is ``set(foo \"world\")``
      and ``set(hello_${foo} \"earth\")``. Occurrences of
      '``${hello_${foo}}``' will be replaced by '``hello_world-NOTFOUND``'

    .. seealso:: :func:`.substituteVariableReferences`
    """

    for t in reversed(self._scriptContents.tokens):
      if _isCommand(t, "set") and len(t.arguments) and \
         t.arguments[0].text == name:
        if len(t.arguments) < 2:
          return None
        value = " ".join([argument.text for argument in t.arguments[1:] if isinstance(argument, CMakeParser.String)])
        if substitute:
          value = self.substituteVariableReferences(value)
        return value

    if default is not None:
      return default

    raise KeyError("script does not set %r" % name)

  #---------------------------------------------------------------------------
  def setValue(self, name, value):
    """Change value of CMake variable set in project.

    :param name: Name of the variable.
    :type name: :class:`str`
    :param value: Value to assign to the variable.
    :type value: :class:`str`

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
    :type name: :class:`str`

    :raises: :exc:`~exceptions.EOFError` if no insertion point can be found.

    This adds an ``add_subdirectory()`` call for ``name`` to the build script.
    If possible, the new call is inserted immediately before a placeholder
    comment which is designated for this purpose. Otherwise, the new call is
    inserted after the last existing call to ``add_subdirectory()``.
    """

    indent = ""
    after = -1

    for n in range(len(self._scriptContents.tokens)):
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
  def save(self, destination=None, encoding=None):
    """Save the project.

    :param destination: Location to which to write the build script.
    :type destination: :class:`str` or ``None``
    :param encoding: Encoding with which to write the build script.
    :type destination: :class:`str` or ``None``

    This saves the extension project CMake script to the specified file:

    .. code-block:: python

      # Open a project
      p = ExtensionProject('.')

      # Set a value in the project
      p.setValue('EXTENSION_DESCRIPTION', 'This is an awesome extension!')

      # Save the changes
      p.save()

    If ``destination`` is ``None``, the CMakeLists.txt file from which the
    project instance was created is overwritten. Similarly, if ``encoding`` is
    ``None``, the file is written with the original encoding of the
    CMakeLists.txt file from which the project instance was created, if such
    encoding is other than ASCII; otherwise the file is written in UTF-8.
    """

    if destination is None:
      destination = self._scriptPath

    if encoding is None and self.encoding is not None:
      encoding = self.encoding if self.encoding.lower() != "ascii" else "utf-8"

    if encoding is None:
      # If no encoding is specified and we don't know the original encoding,
      # perform no conversion and hope for the best (will only work if there
      # are no unicode instances in the script)
      with open(destination, "w") as fp:
        fp.write(str(self._scriptContents))

    else:
      # Otherwise, write the file using full encoding conversion
      with open(destination, "wb") as fp:
        fp.write(str(self._scriptContents).encode(encoding))
