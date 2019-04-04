"""
This module allows to `version` Slicer mediawiki documentation.

It has three modes of operation: ``query``, `copy` and ``update``.

Quick-start
===========

Assuming the new release of Slicer is ``4.7``, the Slicer wiki documentation
can be updated doing the following::

  $ pip install mwdoc
  $ python wiki.py copy 4.7
  $ python wiki.py update 4.7


Usage
=====

::

  $ python wiki.py --help
  usage: wiki.py [-h] [--log-level LOG_LEVEL] [--password PASSWORD]
                 {query,copy,update} ...

  This command-line tool allows to `version` Slicer mediawiki documentation. It
  has three main modes of operation: ``query``, `copy` and ``update``.

  positional arguments:
    {query,copy,update}   available sub-commands
      query               obtain version information
      copy                copy Nightly pages into RELEASE_VERSION namespace
      update              create and/or update wiki pages with RELEASE_VERSION

  optional arguments:
    -h, --help            show this help message and exit
    --log-level LOG_LEVEL
                          Level of debug verbosity. DEBUG, INFO, WARNING, ERROR,
                          CRITICAL.
    --password PASSWORD   password for 'UpdateBot' user. By default, try to get
                          password from 'SLICER_WIKI_UPDATEBOT_PWD' environment
                          variable.

Examples
========

Example of query
----------------

::

  $ python wiki.py query
  connecting
  signing in
  Version info:
    Template:Documentation/currentversion: 4.6
    Template:Documentation/nextversion: 4.7
    Template:Documentation/prevversion: 4.5
  Next version info:
    Template:Documentation/currentversion: 4.7
    Template:Documentation/nextversion: 4.8
    Template:Documentation/prevversion: 4.6
  Versions: Nightly 4.6 4.5 4.4 4.3 4.2 4.1 4.0 3.6 3.5 3.4 3.2
  Acknowledgments main version: 4.6
  Redirect pages:
    FAQ: 4.6
    Documentation/Release: 4.6
    Documentation/Release/Announcements: 4.6
    Documentation/Release/Report a problem: 4.6
    Documentation/UserTraining: 4.6
    Documentation/UserFeedback: 4.6
    Documentation/Release/SlicerApplication/HardwareConfiguration: 4.6


Example of copy
---------------

::

  $ python wiki.py copy 4.6
  connecting
  signing in
  [INFO] Page successfully created: 'Documentation/4.6/Extensions/ModelClip'
  [WARNING] Skip page creation: Page already exists: 'Documentation/4.6/Extensions/ErodeDilateLabel'
  [...]
  [INFO] Page successfully created: 'Template:Documentation/4.6/module-header'
  [INFO] Page successfully created: 'Template:Documentation/4.6/module-section'
  [INFO] Page successfully created: 'Template:Documentation/4.6/module/footer'


Example of update
-----------------

::

  $ python wiki.py update 4.6
  connecting
  signing in
  skipping Template:Documentation/currentversion: version is 4.6
  skipping FAQ: version is 4.6
  skipping Template:Documentation/versionlist: version is 4.6
  skipping Template:Documentation/acknowledgments-versionlist: version is 4.6
  skipping Documentation: version 4.6 already added
"""
from __future__ import print_function

import argparse
import logging
import mwdoc
import os
import re

log = logging.getLogger(__name__)


class Wiki(object):

  def __init__(self, username="UpdateBot", password=None):
    log.info("connecting")
    self.doc = mwdoc.Documentation('www.slicer.org', '/w/')
    if password is not None:
      log.info("signing in")
      self.doc.login(username, password)

  def page_content(self, page_name):
    """Return the content of ``page_name``."""
    log.debug("accessing page [%s]" % page_name)
    page = self.doc.site.pages[page_name]
    content = page.text()
    log.debug("-" * 80)
    log.debug("page content:\n%s" % content)
    log.debug("-" * 80)
    return content

  def set_page_content(self, page_name, content, summary):
    """Update ``page_name`` with ``content``."""
    log.info("updating %s" % page_name)
    log.debug("accessing page [%s]" % page_name)
    page = self.doc.site.pages[page_name]
    if page.text() == content:
      log.info("skipping %s: content up-to-date" % page_name)
      return
    page.save(content, summary=summary)

  def version_pages(self, release_version):
    """Copy ``Documentation/Nightly`` and ``Template:Documentation/Nightly``
    pages into ``release_version`` namespace."""
    self.doc.versionPages(
      'Nightly', release_version,
      ['Documentation', 'Template:Documentation'])

  VERSION_INFO_PAGES = {
    "previous": "Template:Documentation/prevversion",
    "current": "Template:Documentation/currentversion",
    "next": "Template:Documentation/nextversion"
  }

  def version_info(self, page_name):
    content = self.page_content(page_name)
    result = re.match(r"<includeonly>([0-9]\.[0-9]+)</includeonly>", content)
    return result.group(1)

  def previous_version(self):
    return self.version_info(Wiki.VERSION_INFO_PAGES["previous"])

  def next_version(self):
    return self.version_info(Wiki.VERSION_INFO_PAGES["next"])

  def current_version(self):
    return self.version_info(Wiki.VERSION_INFO_PAGES["current"])

  def compute_updated_version_info(self, target_release_version):
    target_major = int(target_release_version.split(".")[0])
    target_minor = int(target_release_version.split(".")[1])
    return {
      "previous": self.current_version(),
      "current": target_release_version,
      "next": "%d.%d" % (target_major, target_minor + 2)
    }

  def update_version_info_pages(self, release_version):
    updated_version_info = self.compute_updated_version_info(release_version)
    for page_short_name, page_name in Wiki.VERSION_INFO_PAGES.items():
      # check if update is needed
      current_version = self.version_info(page_name)
      updated_version = updated_version_info[page_short_name]
      log.debug("%s: current version: %s" % (page_short_name, current_version))
      log.debug("%s: updated version: %s" % (page_short_name, updated_version))
      if current_version == updated_version:
        log.info("skipping %s: version is %s" % (page_name, updated_version))
        break
      # update page
      content = self.page_content(page_name)
      template = "<includeonly>%s</includeonly>"
      log.debug("replacing '%s' with '%s'" % (
        template % current_version, template % updated_version))
      content = content.replace(
        template % current_version, template % updated_version)
      summary = "Update %s version from %s to %s" % (
        page_short_name, current_version, updated_version)
      self.set_page_content(page_name, content, summary)
      # sanity check
      current_version = self.version_info(page_name)
      if current_version != updated_version:
        raise RuntimeError(
          "Failed to update %s: %s version is %s" % (
            page_name, page_short_name, current_version))

  def version_list(self):
    content = self.page_content("Template:Documentation/versionlist")
    return re.findall(
      r"\[\[Documentation/(?:[.\w]+)\|([.\w]+)\]\]", content)

  def update_version_list(self, release_version):
    # check if update is needed
    current_list = self.version_list()
    page_name = "Template:Documentation/versionlist"
    if release_version in current_list:
      log.info("skipping %s: version is %s" % (page_name, release_version))
      return
    # update page
    assert current_list[0] == "Nightly"
    current_version = current_list[1]
    content = self.page_content(page_name)
    template = "[[Documentation/%s|%s]]"
    current = template % (current_version, current_version)
    updated = template % (release_version, release_version) + " " + current
    log.debug("replacing '%s' with '%s'" % (current, updated))
    content = content.replace(current, updated)
    summary = "Add %s to version list" % current_version
    self.set_page_content(page_name, content, summary)
    # sanity check
    current_list = self.version_list()
    if release_version not in current_list:
      raise RuntimeError(
        "Failed to update %s: version %s is not in the list" % (
          page_name, release_version))

  def acknowledgments_main_version(self):
    content = self.page_content(
      "Template:Documentation/acknowledgments-versionlist")
    return re.findall(
      r"\[\[Documentation/(?:[.\w]+)/Acknowledgments\|([.\w]+)\]\]", content)[0]

  REDIRECT_PAGES = [
    "FAQ",
    "Documentation/Release",
    "Documentation/Release/Announcements",
    "Documentation/Release/Report a problem",
    "Documentation/UserTraining",
    "Documentation/UserFeedback",
    "Documentation/Release/SlicerApplication/HardwareConfiguration"
  ]

  def redirect_page_version(self, page_name):
    content = self.page_content(page_name)
    result = re.match(r"#REDIRECT \[\[Documentation/([.\w]+)", content)
    return result.group(1)

  def redirect_pages_version(self):
    for redirect_page in Wiki.REDIRECT_PAGES:
      yield redirect_page, self.redirect_page_version(redirect_page)

  def update_redirect_pages(self, release_version):
    for redirect_page in Wiki.REDIRECT_PAGES:
      # check if update is needed
      current_version = self.redirect_page_version(redirect_page)
      if current_version == release_version:
        log.info("skipping %s: version is %s" % (redirect_page, release_version))
        break
      # update page
      content = self.page_content(redirect_page)
      template = "#REDIRECT [[Documentation/%s"
      log.debug("replacing %s" % (template % current_version))
      content = content.replace(
        template % current_version, template % release_version)
      summary = "Update REDIRECT from Documentation/%s to Documentation/%s" % (
        current_version, release_version)
      self.set_page_content(redirect_page, content, summary)
      # sanity check
      current_version = self.redirect_page_version(redirect_page)
      if current_version != release_version:
        raise RuntimeError(
          "Failed to update %s: version is %s" % (
            redirect_page, current_version))

  def update_top_level_documentation_page(self, release_version):
    # check if update is needed
    page_name = "Documentation"
    template = "* [[Documentation/{version}|{version}]] / " \
               "[[Documentation/{version}/ReleaseNotes|Release notes]] / " \
               "[[Documentation/{version}/Announcements | Announcement]] / " \
               "[[Documentation/{version}/Acknowledgments | Acknowledgments]]"
    marker = "<!-- NEXT RELEASE -->"
    content = self.page_content(page_name)
    if marker not in content:
      log.error(
        "failed to update %s: marker %s not found" % (page_name, marker))
      return
    if template.format(version=release_version) in content:
      log.info(
        "skipping %s: version %s already added" % (page_name, release_version))
      return
    # update page
    content = content.replace(
      marker,
      marker + "\n" + template.format(version=release_version)
    )
    summary = "Add %s" % release_version
    self.set_page_content(page_name, content, summary)

  @staticmethod
  def is_valid_version(text):
    return re.match(r"^[0-9].[0-9]+$", text) is not None


def handle_query(wiki, args):

  def display_version_info():
    print("Version info:")
    for page_short_name, page_name in Wiki.VERSION_INFO_PAGES.items():
      method = getattr(wiki, "%s_version" % page_short_name)
      print("  %s: %s" % (page_name, method()))

  def display_next_version_info():
    print("Next version info:")
    for page_short_name, version in \
            wiki.compute_updated_version_info(wiki.next_version()).items():
      page_name = Wiki.VERSION_INFO_PAGES[page_short_name]
      print("  %s: %s" % (page_name, version))

  def display_version_list():
    print("Versions: %s" % " ".join(wiki.version_list()))

  def display_acknowledgments_main_version():
    print(
      "Acknowledgments main version: %s" % wiki.acknowledgments_main_version())

  def display_redirect_pages_version():
    print("Redirect pages:")
    for redirect_page, version in wiki.redirect_pages_version():
      print("  %s: %s" % (redirect_page, version))

  def display_all():
    display_version_info()
    display_next_version_info()
    display_version_list()
    display_redirect_pages_version()

  query_args = []

  def _check(arg_name):
    query_args.append(getattr(args, arg_name))
    return query_args[-1]

  if _check('version_info'):
    display_version_info()

  if _check('next_version_info'):
    display_next_version_info()

  if _check('version_list'):
    display_version_list()

  if _check('redirect_pages_version'):
    display_redirect_pages_version()

  if not any(query_args):
    display_all()


def handle_copy(wiki, args):
  release_version = args.release_version
  if not Wiki.is_valid_version(release_version):
    log.error("invalid release version: %s" % release_version)
    return
  wiki.version_pages(release_version)


def handle_update(wiki, args):
  release_version = args.release_version
  if not Wiki.is_valid_version(release_version):
    log.error("invalid release version: %s" % release_version)
    return

  update_args = []

  def _check(arg_name):
    update_args.append(getattr(args, arg_name))
    return update_args[-1]

  if _check('version_info_pages'):
    wiki.update_version_info_pages(release_version)

  if _check('redirect_pages'):
    wiki.update_redirect_pages(release_version)

  if _check('version_list'):
    wiki.update_version_list(release_version)

  if _check('top_level_documentation_page'):
    wiki.update_top_level_documentation_page(release_version)

  if not any(update_args):
    wiki.update_version_info_pages(release_version)
    wiki.update_redirect_pages(release_version)
    wiki.update_version_list(release_version)
    wiki.update_top_level_documentation_page(release_version)


def main():
  """This command-line tool allows to `version` Slicer mediawiki documentation.

  It has three main modes of operation: ``query``, `copy` and ``update``.
  """
  parser = argparse.ArgumentParser(description=main.__doc__)
  parser.add_argument(
    '--log-level', dest='log_level',
    default='INFO',
    help='Level of debug verbosity. DEBUG, INFO, WARNING, ERROR, CRITICAL.',
  )
  parser.add_argument(
    "--password", type=str, default=os.environ.get('SLICER_WIKI_UPDATEBOT_PWD'),
    help="password for 'UpdateBot' user. By default, try to get password from "
         "'SLICER_WIKI_UPDATEBOT_PWD' environment variable."
  )

  subparsers = parser.add_subparsers(
    help='available sub-commands', dest='command')

  # sub-command parser
  parser_query = subparsers.add_parser(
    'query', help='obtain version information')

  parser_query.add_argument(
    "--version-info", action="store_true",
    help="display the version associated with pages %s" % ", ".join(
      ['%s' % page_name for page_name in Wiki.VERSION_INFO_PAGES.values()])
  )
  parser_query.add_argument(
    "--next-version-info", action="store_true",
    help="display what would be the *next* version associated "
         "with pages %s" % ", ".join(
          ['%s' % page_name for page_name in Wiki.VERSION_INFO_PAGES.values()])
  )
  parser_query.add_argument(
    "--version-list", action="store_true",
    help="display the versions associated with page "
         "'Template::Documentation/versionlist'"
  )
  parser_query.add_argument(
    "--acknowledgments-main-version", action="store_true",
    help="display the version associated with page "
         "'Template:Documentation/acknowledgments-versionlist'"
  )
  parser_query.add_argument(
    "--redirect-pages-version", action="store_true",
    help="display the version associated with pages with redirect"
  )

  # sub-command parser
  parser_copy = subparsers.add_parser(
    'copy', help='copy Nightly pages into RELEASE_VERSION namespace')
  parser_copy.add_argument(
    "release_version", type=str, metavar="RELEASE_VERSION",
    help="the release version where Nightly pages will be copied into"
  )

  # sub-command parser
  parser_update = subparsers.add_parser(
    'update', help='create and/or update wiki pages with RELEASE_VERSION')

  parser_update.add_argument(
    "release_version", type=str, metavar="RELEASE_VERSION",
    help="the release version used to update permanent pages"
  )
  parser_update.add_argument(
    "--version-info-pages", action="store_true",
    help="update the version associated with pages %s" % ", ".join(
      ['%s' % page_name for page_name in Wiki.VERSION_INFO_PAGES.values()])
  )
  parser_update.add_argument(
    "--redirect-pages", action="store_true",
    help="update the version associated with redirect pages"
  )
  parser_update.add_argument(
    "--version-list", action="store_true",
    help="add RELEASE_VERSION to page "
         "'Template::Documentation/versionlist'"
  )
  parser_update.add_argument(
    "--acknowledgments-main-version", action="store_true",
    help="add RELEASE_VERSION to page "
         "'Template:Documentation/acknowledgments-versionlist'"
  )
  parser_update.add_argument(
    "--top-level-documentation-page", action="store_true",
    help="add RELEASE_VERSION to page 'Documentation'"
  )

  args = parser.parse_args()

  log.setLevel(args.log_level.upper())
  log.addHandler(logging.StreamHandler())

  wiki = Wiki(password=args.password)

  if args.command == "query":
    handle_query(wiki, args)
  elif args.command == "copy":
    handle_copy(wiki, args)
  elif args.command == "update":
    handle_update(wiki, args)


if __name__ == "__main__":
  try:
    main()
  except KeyboardInterrupt:
    print("interrupt received, stopping...")
