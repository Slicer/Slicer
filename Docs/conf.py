#!/usr/bin/env python3
#
# 3D Slicer documentation build configuration file, created by
# sphinx-quickstart on Tue Mar 21 03:07:30 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import lxml.etree as ET
import os
import sys

sys.path.insert(0, os.path.abspath('../Base/Python'))
sys.path.append(os.path.abspath("./_sphinxext"))


# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
    'myst_parser',
    'sphinx_markdown_tables',
    'notfound.extension',  # Show a better 404 page when an invalid address is entered
]

myst_enable_extensions = [
    "colon_fence",  # Allow code fence using ::: (see https://myst-parser.readthedocs.io/en/latest/using/syntax-optional.html#syntax-colon-fence)
    "linkify",  # Allow automatic creation of links from URLs (it is sufficient to write https://google.com instead of <https://google.com>)
]

# Auto-generate header anchors up to level 6, so that it can be referenced like [](file.md#header-anchor).
# (see https://myst-parser.readthedocs.io/en/latest/using/syntax-optional.html#auto-generated-header-anchors)
myst_heading_anchors = 6

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
source_suffix = ['.rst', '.md']

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = '3D Slicer'
copyright = '2020, Slicer Community'
author = 'Slicer Community'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = ''
# The full version, including alpha/beta/rc tags.
release = ''

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', '_moduledescriptions']

# Set EXCLUDE_DEVELOPER_GUIDE=True environment variable to exclude developer guide.
# It is useful for quicker documentation generation while eiditing user manual.
if os.environ.get('EXCLUDE_API_REFERENCE', False) == 'True':
    print("API reference is excluded from documentation.")
    exclude_patterns.append('developer_guide/vtkTeem.rst')
    exclude_patterns.append('developer_guide/vtkAddon.rst')
    exclude_patterns.append('developer_guide/vtkITK.rst')
    exclude_patterns.append('developer_guide/slicer.rst')
    exclude_patterns.append('developer_guide/mrml.rst')

# sphinx-notfound-page
# https://github.com/readthedocs/sphinx-notfound-page
notfound_context = {
    'title': 'Page Not Found',
    'body': '''
<h1>Page Not Found</h1>
<p>Sorry, we couldn't find that page.</p>
<p>Try using the search box or go to the homepage.</p>
''',
}

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False

# A string of reStructuredText that will be included at the beginning of every source file that is read.
rst_prolog = open('global.rst.in').read()

# If given, this must be the name of an image file (path relative to the configuration directory) that is the logo of the docs.
# It is placed at the top of the sidebar; its width should therefore not exceed 200 pixels
html_logo = '_static/images/3D-Slicer-Mark.png'

# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'default'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
html_theme_options = {
    # Toc options
    'includehidden': False,
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# These paths are either relative to html_static_path
# or fully qualified paths (eg. https://...)
html_css_files = [
    'css/custom.css',
]

# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = '3DSlicerdoc'


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',

    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, '3DSlicer.tex', '3D Slicer Documentation',
     'Slicer Community', 'manual'),
]


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, '3Dslicer', '3D Slicer Documentation',
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, '3DSlicer', '3D Slicer Documentation',
     author, '3DSlicer', 'One line description of project.',
     'Miscellaneous'),
]


# -- Read The Docs -----------------------------------------------------

# on_rtd is whether we are on readthedocs.io
on_rtd = os.environ.get('READTHEDOCS', None) == 'True'

if not on_rtd:  # only import and set the theme if we're building docs locally
    import sphinx_rtd_theme
    html_theme = 'sphinx_rtd_theme'
    html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

# -- Convert CLI module descriptions into markdown files ----------------

# Each CLI module descriptor XML file is converted to two markdown files
# in _moduledescriptions subfolder: *Overview.md and *Parameters.md.
# Overview file contains the module title and description.
# Parameters file contains detailed description of module inputs and
# outputs, contributors, and acknowledgements.
#
# These md files are included at the top and bottom of each CLI module
# documentation file (user_guide\modules\*.md). Custom content, such as
# tutorials, screenshots, etc. can be added in between these includes.
#
# A copy of all CLI module descriptor XML files for modules that are
# not part of the Slicer repository (e.g., BRAINS toolkit) are stored in
# _extracli subfolder. Content of this folder must be updated manually
# whenever they are updated in the original location. The process could
# be automated with some effort, but since these bundled libraries do
# not change frequently, manual update takes less work overall.


# Documentation root folder (folder of this script).
docsfolder = os.path.dirname(__file__)

# List of folders that contain CLI module descriptor XML files.
inputpaths = [
    os.path.join(docsfolder, "../Modules/CLI"),
    os.path.join(docsfolder, "_extracli"),
    ]

# List of modules to be excluded from documentation generation
# (for example, testing modules only).
excludenames = [
    'CLIROITest.xml',
    'TestGridTransformRegistration.xml',
    'DiffusionTensorTest.xml',
    ]

# Output folder that contains all generated markdown files.
outpath = os.path.join(docsfolder, "_moduledescriptions")
os.makedirs(outpath, exist_ok=True)
with open(os.path.join(outpath, '_readme_.txt'), 'w') as descriptionfile:
    descriptionfile.write("Content of this folder is automatically generated by Docs/conf.py from CLI module descriptor XML files\n")
    descriptionfile.write("during documentation build. The folder can be deleted because it is automatically regenerated when needed.")


def _generatemd(dom, docsfolder, outpath, xslt, suffix):
    """Helper function to create markdown file from CLI module description XML file using XSLT"""
    xsltpath = os.path.join(docsfolder, xslt)
    transform = ET.XSLT(ET.parse(xsltpath))
    content = str(transform(dom))
    with open(os.path.join(outpath, os.path.splitext(name)[0]+suffix+'.md'), 'w', encoding='utf8') as outfile:
        outfile.write(content)


for inputpath in inputpaths:
    for root, dirs, files in os.walk(inputpath):
        for name in files:
            if name in excludenames:
                continue
            if name.endswith(".xml"):
                print(f"Generating CLI module documentation from {name}")
                dom = ET.parse(os.path.join(root, name))
                _generatemd(dom, docsfolder, outpath, "cli_module_overview_to_md.xsl", "Overview")
                _generatemd(dom, docsfolder, outpath, "cli_module_parameters_to_md.xsl", "Parameters")
