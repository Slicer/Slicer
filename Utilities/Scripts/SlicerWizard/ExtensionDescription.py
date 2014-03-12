import glob
import os
import re

from .ExtensionProject import ExtensionProject

#=============================================================================
class ExtensionDescription(object):
  _reParam = re.compile(r"([a-zA-Z][a-zA-Z0-9_]*)\s+(.+)")

  #---------------------------------------------------------------------------
  def __init__(self, repo=None, filepath=None, sourcedir=None):
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
        svnRemote = None

        # Try to get git remote
        try:
          remote = repo.remotes.origin
        except:
          if len(repo.remotes) == 1:
            remote = repo.remotes[0]

        if remote is None:
          # Try to get svn remote
          config = repo.config_reader()
          for s in config.sections():
            if s.startswith("svn-remote"):
              svnRemote = s[12:-1]
              break

          if svnRemote is None:
            raise Exception("unable to determine repository's primary remote")

          else:
            si = self._gitSvnInfo(repo, svnRemote)
            setattr(self, "scm", "svn")
            setattr(self, "scmurl", si["URL"])
            setattr(self, "scmrevision", si["Revision"])

        else:
          setattr(self, "scm", "git")
          setattr(self, "scmurl", self._remotePublicUrl(remote))
          setattr(self, "scmrevision", repo.head.commit.hexsha)

        sourcedir = repo.working_tree_dir

      # Handle svn repositories
      elif hasattr(repo, "wc_root"):
        setattr(self, "scm", "svn")
        setattr(self, "scmurl", repo.url)
        setattr(self, "scmrevision", repo.last_change_revision)
        sourcedir = repo.wc_root

    else:
      setattr(self, "scm", "local")
      setattr(self, "scmurl", "NA")
      setattr(self, "scmrevision", "NA")

    if sourcedir is not None:
      p = ExtensionProject(sourcedir)
      self._setProjectAttribute("homepage", p, required=True)
      self._setProjectAttribute("category", p, required=True)
      self._setProjectAttribute("description", p)
      self._setProjectAttribute("contributors", p)

      self._setProjectAttribute("status", p)
      self._setProjectAttribute("enabled", p, default="1")
      self._setProjectAttribute("depends", p, default="NA")
      self._setProjectAttribute("build_subdirectory", p, default=".")

      self._setProjectAttribute("iconurl", p)
      self._setProjectAttribute("screenshoturls", p)

      if self.scm == "svn":
        self._setProjectAttribute("svnusername", p, elideempty=True)
        self._setProjectAttribute("svnpassword", p, elideempty=True)

  #---------------------------------------------------------------------------
  def __repr__(self):
    return repr(self.__dict__)

  #---------------------------------------------------------------------------
  @staticmethod
  def _remotePublicUrl(remote):
    url = remote.url
    if url.startswith("git@"):
      return url.replace(":", "/").replace("git@", "git://")

    return url

  #---------------------------------------------------------------------------
  @staticmethod
  def _gitSvnInfo(repo, remote):
    result = {}
    for l in repo.git.svn('info', R=remote).split("\n"):
      if len(l):
        key, value = l.split(":", 1)
        result[key] = value.strip()
    return result

  #---------------------------------------------------------------------------
  def _setProjectAttribute(self, name, project, default=None, required=False,
                           elideempty=False):

    if default is None and not required:
      default=""

    v = project.getValue("EXTENSION_" + name.upper(), default)

    if len(v) or not elideempty:
      setattr(self, name, v)

  #---------------------------------------------------------------------------
  def clear(self):
    for key in self.__dict__:
      delattr(self, key)

  #---------------------------------------------------------------------------
  def _read(self, fp):
    for l in fp:
      m = self._reParam.match(l)
      if m is not None:
        setattr(self, m.group(1), m.group(2).strip())

  #---------------------------------------------------------------------------
  def read(self, path):
    self.clear()

    descriptionFiles = glob.glob(os.path.join(path, "*.[Ss]4[Ee][Xx][Tt]"))
    if len(descriptionFiles) < 1:
      raise IOError("extension description file not found")

    if len(descriptionFiles) > 1:
      raise IOError("multiple extension description files found")

    with open(descriptionFiles[0]) as fp:
      self._read(fp)

  #---------------------------------------------------------------------------
  def _write(self, fp):
    for key in sorted(self.__dict__):
      fp.write(("%s %s" % (key, getattr(self, key))).strip() + "\n")

  #---------------------------------------------------------------------------
  def write(self, out):
    if isinstance(out, file):
      self._write(out)

    else:
      with open(out, "w") as fp:
        self._write(fp)
