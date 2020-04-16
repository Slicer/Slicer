import fnmatch
import logging
import os

from .Utilities import die, detectEncoding

_sourcePatterns = [
  "*.h",
  "*.cxx",
  "*.cpp",
  "CMakeLists.txt",
  "*.cmake",
  "*.ui",
  "*.qrc",
  "*.py",
  "*.xml",
  "*.xml.in",
  "*.md5",
  "*.png",
  "*.dox",
  "*.sha256",
]

_templateCategories = [
  "extensions",
  "modules",
]

#-----------------------------------------------------------------------------
def _isSourceFile(name):
  for pat in _sourcePatterns:
    if fnmatch.fnmatch(name, pat):
      return True

  return False

#-----------------------------------------------------------------------------
def _isTemplateCategory(name, relPath):
  if not os.path.isdir(os.path.join(relPath, name)):
    return False

  name = name.lower()
  return name in _templateCategories

#-----------------------------------------------------------------------------
def _listSources(directory):
  for root, subFolders, files in os.walk(directory):
    for f in files:
      if _isSourceFile(f):
        f = os.path.join(root, f)
        yield f[len(directory) + 1:] # strip common dir

#=============================================================================
class TemplateManager(object):
  """Template collection manager.

  This class provides a template collection and operations for managing and
  using that collection.
  """

  #---------------------------------------------------------------------------
  def __init__(self):
    self._paths = {}
    self._keys = {}

    for c in _templateCategories:
      self._paths[c] = {}

  #---------------------------------------------------------------------------
  def _getKey(self, kind):
    if kind in self._keys:
      return self._keys[kind]

    return "TemplateKey"

  #---------------------------------------------------------------------------
  def _copyAndReplace(self, inFile, template, destination, key, name):
    outFile = os.path.join(destination, inFile.replace(key, name))
    logging.info("creating '%s'" % outFile)
    path = os.path.dirname(outFile)
    if not os.path.exists(path):
      os.makedirs(path)

    # Read file contents
    p = os.path.join(template, inFile)
    with open(p, "rb") as fp:
      contents = fp.read()

    # Replace template key with copy name
    if isinstance(name, bytes):
      # If replacement is just bytes, we can just do the replacement...
      contents = contents.replace(key, name)
      contents = contents.replace(key.upper(), name.upper())

    else:
      # ...else we have to try to guess the template file encoding in order to
      # convert it to unicode and back
      encoding, confidence = detectEncoding(contents)

      if encoding is not None:
        if confidence < 0.5:
          logging.warning("%s: encoding detection confidence is %f:"
                          " copied file might be corrupt" % (p, confidence))

        contents = contents.decode(encoding)
        contents = contents.replace(key, name)
        contents = contents.replace(key.upper(), name.upper())
        contents = contents.encode(encoding)

      else:
        # Looks like a binary file; don't perform replacement
        pass

    # Write adjusted contents
    with open(outFile, "wb") as fp:
      fp.write(contents)

  #---------------------------------------------------------------------------
  def copyTemplate(self, destination, category, kind, name, createInSubdirectory=True, requireEmptyDirectory=True):
    """Copy (instantiate) a template.

    :param destination: Directory in which to create the template copy.
    :type destination: :class:`str`
    :param category: Category of template to instantiate.
    :type category: :class:`str`
    :param kind: Name of template to instantiate.
    :type kind: :class:`str`
    :param name: Name for the instantiated template.
    :type name: :class:`str`
    :param createInSubdirectory: If True then files are copied to ``destination/name/``, else ``destination/``.
    :type name: :class:`bool`
    :param requireEmptyDirectory: If True then files are only copied if the target directory is empty.
    :type name: :class:`bool`

    :return:
      Path to the new instance (``os.path.join(destination, name)``).
    :rtype:
      :class:`unicode` if either ``destination`` and/or ``name`` is also
      :class:`unicode`, otherwise :class:`str`.

    :raises:
      * :exc:`~exceptions.KeyError` if the specified template is not found.
      * :exc:`~exceptions.IOError` if a subdirectory ``name`` already exists.

    This creates a copy of the specified template in ``destination``, with
    occurrences of the template's key replaced with ``name``. The new copy is
    in a subdirectory ``name``, which must not exist.

    .. note:: The replacement of the template key is case sensitive, however \
              the upper-case key is also replaced with the upper-case ``name``.

    .. seealso:: :meth:`.setKey`
    """

    templates = self._paths[category]
    if not kind.lower() in templates:
      raise KeyError("'%s' is not a known extension template" % kind)

    kind = kind.lower()

    if createInSubdirectory:
      destination = os.path.join(destination, name)

    if requireEmptyDirectory and os.path.exists(destination):
      raise IOError("create %s: refusing to overwrite"
                    " existing directory '%s'" % (category, destination))

    template = templates[kind]
    key = self._getKey(kind)

    logging.info("copy template '%s' to '%s', replacing '%s' -> '%s'" %
          (template, destination, key, name))
    for f in _listSources(template):
      self._copyAndReplace(f, template, destination, key, name)

    return destination

  #---------------------------------------------------------------------------
  def addCategoryPath(self, category, path):
    """Add templates for a particular category to the collection.

    :param category: Category of templates to add.
    :type category: :class:`str`
    :param path: Path to a directory containing templates.
    :type path: :class:`str`

    :raises:
      :exc:`~exceptions.KeyError` if ``category`` is not a known template
      category.

    This adds all templates found in ``path`` to ``category`` to the collection,
    where each subdirectory of ``path`` is a template. If ``path`` contains any
    templates whose names already exist in the ``category`` of the collection
    (case insensitive), the existing entries are replaced.
    """

    for entry in os.listdir(path):
      entryPath = os.path.join(path, entry)
      if os.path.isdir(entryPath):
        self._paths[category][entry.lower()] = entryPath

  #---------------------------------------------------------------------------
  def addPath(self, basePath):
    """Add a template path to the collection.

    :param basePath: Path to a directory containing categorized templates.
    :type basePath: :class:`str`

    This adds categorized templates to the collection. ``basePath`` should be
    a directory which contains one or more directories whose names match a
    known template category (case insensitive). Each such subdirectory is added
    to the collection via :meth:`.addCategoryPath`.
    """

    if not os.path.exists(basePath):
      return

    basePath = os.path.realpath(basePath)

    for entry in os.listdir(basePath):
      if _isTemplateCategory(entry, basePath):
        self.addCategoryPath(entry.lower(), os.path.join(basePath, entry))

  #---------------------------------------------------------------------------
  def setKey(self, name, value):
    """Set template key for specified template.

    :param name: Name of template for which to set key.
    :type name: :class:`str`
    :param key: Key for specified template.
    :type name: :class:`str`

    This sets the template key for ``name`` to ``key``.

    .. 'note' directive needs '\' to span multiple lines!
    .. note:: Template keys depend only on the template name, and not the \
              template category. As a result, two templates with the same name \
              in different categories will use the same key.

    .. seealso:: :meth:`.copyTemplate`
    """

    self._keys[name] = value

  #---------------------------------------------------------------------------
  @classmethod
  def categories(cls):
    """Get list of known template categories.

    :rtype: :class:`list` of :class:`str`.

    .. seealso:: :meth:`templates`, :meth:`.listTemplates`
    """

    return list(_templateCategories);

  #---------------------------------------------------------------------------
  def templates(self, category=None):
    """Get collection of available templates.

    :param category: Category of templates to query.
    :type name: :class:`str`

    :return:
      List of templates for the specified category, or a dictionary of such
      (keyed by category name) if ``category`` is ``None``.
    :rtype:
      :class:`list` of :class:`str`, or :class:`dict` of
      :class:`str` |rarr| (:class:`list` of :class:`str`).

    :raises:
      :exc:`~exceptions.KeyError` if ``category`` is not ``None`` or a known
      template category.

    .. seealso:: :func:`~SlicerWizard.TemplateManager.categories`,
                 :meth:`.listTemplates`
    """

    if category is None:
      result = {}
      for c in _templateCategories:
        result[c] = list(self._paths[c].keys())
      return result

    else:
      return tuple(self._paths[category].keys())

  #---------------------------------------------------------------------------
  def listTemplates(self):
    """List available templates.

    This displays a list of all available templates, using :func:`logging.info`,
    organized by category.

    .. seealso:: :func:`~SlicerWizard.TemplateManager.categories`,
                 :meth:`.templates`
    """

    for c in _templateCategories:
      logging.info("Available templates for category '%s':" % c)

      if len(self._paths[c]):
        for t in sorted(self._paths[c].keys()):
          logging.info("  '%s' ('%s')" % (t, self._getKey(t)))

      else:
        logging.info("  (none)")

      logging.info("")

  #---------------------------------------------------------------------------
  def addArguments(self, parser):
    """Add template manager |CLI| arguments to parser.

    :param parser: Argument parser instance to which to add arguments.
    :type parser: :class:`argparse.ArgumentParser`

    This adds |CLI| arguments to the specified ``parser`` that may be used to
    interact with the template collection.

    .. 'note' directive needs '\' to span multiple lines!
    .. note:: The arguments use ``'<'`` and ``'>'`` to annotate optional \
              values. It is recommended to use :class:`.WizardHelpFormatter` \
              with the parser so that these will be displayed using the \
              conventional ``'['`` and ``']'``.

    .. seealso:: :meth:`.parseArguments`
    """

    parser.add_argument("--templatePath", metavar="<CATEGORY=>PATH",
                        action="append",
                        help="add additional template path for specified"
                             " template category; if no category, expect that"
                             " PATH contains subdirectories for one or more"
                             " possible categories")
    parser.add_argument("--templateKey", metavar="TYPE=KEY", action="append",
                        help="set template substitution key for specified"
                             " template (default key: 'TemplateKey')")

  #---------------------------------------------------------------------------
  def parseArguments(self, args):
    """Automatically add paths and keys from |CLI| arguments.

    :param args.templatePath: List of additional template paths.
    :type args.templatePath: :class:`list` of :class:`str`
    :param args.templateKey: List of user-specified template key mappings.
    :type args.templateKey: :class:`list` of :class:`str`

    This parses template-related command line arguments and updates the
    collection accordingly:

    * Additional template paths are provided in the form
      ``'[category=]path'``, and are added with either :meth:`.addPath` (if
      ``category`` is omitted) or :meth:`.addCategoryPath` (otherwise).
    * Template keys are provided in the form ``'name=value'``, and are
      registered using :meth:`.setKey`.

    If a usage error is found, the application is terminated by calling
    :func:`~.Utilities.die` with an appropriate error message.

    .. seealso:: :meth:`.parseArguments`, :meth:`.addPath`,
                 :meth:`.addCategoryPath`, :meth:`.setKey`
    """

    # Add user-specified template paths
    if args.templatePath is not None:
      for tp in args.templatePath:
        tpParts = tp.split("=", 1)

        if len(tpParts) == 1:
          if not os.path.exists(tp):
            die("template path '%s' does not exist" % tp)
          if not os.path.isdir(tp):
            die("template path '%s' is not a directory" % tp)

          self.addPath(tp)

        else:
          if tpParts[0].lower() not in _templateCategories:
            die(("'%s' is not a recognized template category" % tpParts[0],
                 "recognized categories: %s" % ", ".join(_templateCategories)))

          if not os.path.exists(tpParts[1]):
            die("template path '%s' does not exist" % tpParts[1])
          if not os.path.isdir(tpParts[1]):
            die("template path '%s' is not a directory" % tpParts[1])

          self.addCategoryPath(tpParts[0].lower(),
                               os.path.realpath(tpParts[1]))

    # Set user-specified template keys
    if args.templateKey is not None:
      for tk in args.templateKey:
        tkParts = tk.split("=")
        if len(tkParts) != 2:
          die("template key '%s' malformatted: expected 'NAME=KEY'" % tk)

        self.setKey(tkParts[0].lower(), tkParts[1])
