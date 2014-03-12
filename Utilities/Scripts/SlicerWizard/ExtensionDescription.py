#!/usr/bin/env python

import glob
import os
import re

from .ExtensionProject import ExtensionProject

#=============================================================================
class ExtensionDescription(object):
  _reParam = re.compile(r"([a-zA-Z][a-zA-Z0-9_]*)\s+(.+)")

  #---------------------------------------------------------------------------
  def __init__(self, repo=None, filepath=None):
    if repo is not None and filepath is not None:
      raise Exception("cannot construct %s: only one of repo or filepath"
                      " may be given" % type(self).__name__)

    if filepath is not None:
      with open(filepath) as fp:
        self._read(fp)

    elif repo is not None:
      try:
        remote = repo.remotes.origin
      except:
        if len(repo.remotes) != 1:
          raise Exception("unable to determine repository's primary remote")

        remote = repo.remotes[0]

      setattr(self, "scm", "git")
      setattr(self, "scmurl", self._remotePublicUrl(remote))
      setattr(self, "scmrevision", repo.head.commit.hexsha)

      p = ExtensionProject(repo.working_tree_dir)
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
  def _setProjectAttribute(self, name, project, default=None, required=False):
    if default is None and not required:
      default=""

    pk = "EXTENSION_" + name.upper()
    setattr(self, name, project.getValue(pk, default))

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
