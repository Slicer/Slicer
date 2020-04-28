import os

from docutils import nodes, io, utils
from docutils.parsers import rst
from docutils.core import ErrorString
from docutils.utils import SafeString

from recommonmark.parser import CommonMarkParser


class MdInclude(rst.Directive):
    """Directive class to include markdown in sphinx.
    Load a file and convert it to rst and insert as a node. Currently
    directive-specific options are not implemented.
    See https://github.com/sphinx-doc/sphinx/issues/7000
    and https://github.com/readthedocs/recommonmark/issues/191
    """
    required_arguments = 1
    optional_arguments = 0
    option_spec = {
        'start-line': int,
        'end-line': int,
    }

    def run(self):
        """Most of this method is from ``docutils.parser.rst.Directive``.
        docutils version: 0.12
        """
        if not self.state.document.settings.file_insertion_enabled:
            raise self.warning('"%s" directive disabled.' % self.name)
        source = self.state_machine.input_lines.source(
            self.lineno - self.state_machine.input_offset - 1)
        source_dir = os.path.dirname(os.path.abspath(source))
        path = rst.directives.path(self.arguments[0])
        path = os.path.normpath(os.path.join(source_dir, path))
        path = utils.relative_path(None, path)
        path = nodes.reprunicode(path)

        # get options (currently not use directive-specific options)
        encoding = self.options.get(
            'encoding', self.state.document.settings.input_encoding)
        e_handler = self.state.document.settings.input_encoding_error_handler

        # open the including file
        try:
            self.state.document.settings.record_dependencies.add(path)
            include_file = io.FileInput(source_path=path,
                                        encoding=encoding,
                                        error_handler=e_handler)
        except UnicodeEncodeError:
            raise self.severe('Problems with "%s" directive path:\n'
                              'Cannot encode input file path "%s" '
                              '(wrong locale?).' %
                              (self.name, SafeString(path)))
        except IOError as error:
            raise self.severe('Problems with "%s" directive path:\n%s.' %
                              (self.name, ErrorString(error)))

        # read from the file
        startline = self.options.get('start-line', None)
        endline = self.options.get('end-line', None)
        try:
            if startline or (endline is not None):
                lines = include_file.readlines()
                rawtext = ''.join(lines[startline:endline])
            else:
                rawtext = include_file.read()
        except UnicodeError as error:
            raise self.severe('Problem with "%s" directive:\n%s' %
                              (self.name, ErrorString(error)))

        class CustomCommonMarkParser(CommonMarkParser):
            """Temporary workaround to remove multiple build warnings caused by upstream bug.
            See https://github.com/readthedocs/recommonmark/issues/177 for details.
            """
            def visit_document(self, node):
                pass

        doc = utils.new_document(self.arguments[0])
        md_parser = CustomCommonMarkParser()
        md_parser.parse(rawtext, doc)
        return [*doc.children]


def setup(app):
    app.add_directive("mdinclude", MdInclude)

    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
