import json
import os

from .ExtensionProject import ExtensionProject


# =============================================================================
class ExtensionDescription:
    """Representation of an extension description.

    This class provides a Python object representation of an extension
    description. The extension information is made available as attributes on the
    object. The "well known" attributes are described
    :wikidoc:`Developers/Extensions/DescriptionFile here`. Custom attributes may
    be added with :func:`setattr`. Attributes may be removed with :func:`delattr`
    or the :meth:`.clear` method.
    """

    # ---------------------------------------------------------------------------
    def __init__(self, repo=None, filepath=None, sourcedir=None, cmakefile="CMakeLists.txt"):
        """
        :param repo:
          Extension repository from which to create the description.
        :type repo:
          :class:`git.Repo <git:git.repo.base.Repo>`,
        :param filepath:
          Path to an existing ``.json`` to read.
        :type filepath:
          :class:`str` or ``None``.
        :param sourcedir:
          Path to an extension source directory.
        :type sourcedir:
          :class:`str` or ``None``.
        :param cmakefile:
          Name of the CMake file where `EXTENSION_*` CMake variables
          are set. Default is `CMakeLists.txt`.
        :type cmakefile:
          :class:`str`

        :raises:
          * :exc:`~exceptions.KeyError` if the extension description is missing a
            required attribute.
          * :exc:`~exceptions.Exception` if there is some other problem
            constructing the description.

        The description may be created from a repository instance (in which case
        the description repository information will be populated), a path to the
        extension source directory, or a path to an existing ``.json`` file.
        No more than one of ``repo``, ``filepath`` or ``sourcedir`` may be given.
        If none are provided, the description will be incomplete.
        """

        args = (repo, filepath, sourcedir)
        if args.count(None) < len(args) - 1:
            raise Exception("cannot construct %s: only one of"
                            " (repo, filepath, sourcedir) may be given" %
                            type(self).__name__)

        if filepath is not None:
            with open(filepath) as fp:
                self._read(fp)

        elif repo is not None:
            # Handle git repositories
            if hasattr(repo, "remotes"):
                remote = None

                # Get SHA of HEAD (may not exist if no commit has been made yet!)
                try:
                    sha = repo.head.commit.hexsha

                except ValueError:
                    sha = "NA"

                # Try to get git remote
                try:
                    remote = repo.remotes.origin
                except:
                    if len(repo.remotes) == 1:
                        remote = repo.remotes[0]

                if remote is None:
                    # Do we have any remotes?
                    if len(repo.remotes) == 0:
                        setattr(self, "scm", "git")
                        setattr(self, "scmurl", "NA")
                        setattr(self, "scmrevision", sha)

                    else:
                        raise Exception("unable to determine repository's primary remote")

                else:
                    setattr(self, "scm", "git")
                    setattr(self, "scmurl", self._remotePublicUrl(remote))
                    setattr(self, "scmrevision", sha)

                sourcedir = repo.working_tree_dir

            # Handle local source directory
            elif hasattr(repo, "relative_directory"):
                setattr(self, "scm", "local")
                setattr(self, "scmurl", repo.relative_directory)
                setattr(self, "scmrevision", "NA")
                sourcedir = os.path.join(repo.root, repo.relative_directory)

        else:
            setattr(self, "scm", "local")
            setattr(self, "scmurl", "NA")
            setattr(self, "scmrevision", "NA")

        if sourcedir is not None:
            p = ExtensionProject(sourcedir, filename=cmakefile)
            self._setProjectAttribute("homepage", p, required=True)
            self._setProjectAttribute("description", p)
            self._setProjectAttribute("contributors", p)

            self._setProjectAttribute("status", p)
            self._setProjectAttribute("enabled", p, default="1")
            self._setProjectAttribute("depends", p, default="NA")
            self._setProjectAttribute("build_subdirectory", p, default=".")

            self._setProjectAttribute("iconurl", p)
            self._setProjectAttribute("screenshoturls", p)

    # ---------------------------------------------------------------------------
    def __repr__(self):
        return repr(self.__dict__)

    # ---------------------------------------------------------------------------
    @staticmethod
    def _remotePublicUrl(remote):
        url = remote.url
        if url.startswith("git@"):
            return url.replace(":", "/").replace("git@", "https://")

        return url

    # ---------------------------------------------------------------------------
    def _setProjectAttribute(self, name, project, default=None, required=False,
                             elideempty=False, substitute=True):

        if default is None and not required:
            default = ""

        v = project.getValue("EXTENSION_" + name.upper(), default, substitute)

        if len(v) or not elideempty:
            setattr(self, name, v)

    # ---------------------------------------------------------------------------
    def clear(self, attr=None):
        """Remove attributes from the extension description.

        :param attr: Name of attribute to remove.
        :type attr: :class:`str` or ``None``

        If ``attr`` is not ``None``, this removes the specified attribute from the
        description object, equivalent to calling ``delattr(instance, attr)``. If
        ``attr`` is ``None``, all attributes are removed.
        """

        for key in self.__dict__.keys() if attr is None else (attr,):
            delattr(self, key)

    # ---------------------------------------------------------------------------
    def _read(self, fp):
        for key, value in json.load(fp).items():
            setattr(self, key, value)

    # ---------------------------------------------------------------------------
    @staticmethod
    def _findOccurences(a_str, sub):
        start = 0
        while True:
            start = a_str.find(sub, start)
            if start == -1:
                return
            yield start
            start += len(sub)

    # ---------------------------------------------------------------------------
    def _write(self, fp):
        # Creation of the map
        dictio = dict()
        dictio["scmurl"] = getattr(self, "scmurl")
        dictio["scmrevision"] = getattr(self, "scmrevision")
        dictio["build_dependencies"] = getattr(self, "depends")
        dictio["build_subdirectory"] = getattr(self, "build_subdirectory")
        dictio["category"] = "Examples"
        dictio["enabled"] = getattr(self, "enabled")

        fp.write(json.dumps(dictio, sort_keys=True, indent=4))
        fp.write("\n")

    # ---------------------------------------------------------------------------
    def write(self, out):
        """Write extension description to a file or stream.

        :param out: Stream or path to which to write the description.
        :type out: :class:`~io.IOBase` or :class:`str`

        This writes the extension description to the specified file path or stream
        object. This is suitable for producing a ``.json`` file from a description
        object.
        """

        if hasattr(out, "write") and callable(out.write):
            self._write(out)

        else:
            with open(out, "w") as fp:
                self._write(fp)
