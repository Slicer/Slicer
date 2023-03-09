#!/usr/bin/env python

"""Update all translation source (.ts) files from the source code

Example use:

    python.exe c:/D/S5/Utilities/Scripts/update_translations.py -c Slicer -s c:/D/S5 -t c:/D/SlicerLanguageTranslations/translations
      --lupdate c:/Qt/5.15.2/msvc2019_64/bin/lupdate.exe

"""

import argparse
import glob
import logging
import os
import shutil
import subprocess
import sys


def update_translations(component, source_code_dir, translations_dir, lupdate_path, language=None, remove_obsolete_strings=False):

    if language is None:
        ts_filename_filter = f"{component}*.ts"
    else:
        ts_filename_filter = f"{component}*_{language}.ts"

    ts_file_filter = f"{translations_dir}/{ts_filename_filter}"
    ts_file_paths = glob.glob(ts_file_filter)
    if not ts_file_paths:
        raise ValueError(f"No .ts files were found at {ts_file_filter}")

    for ts_file_index, ts_file_path in enumerate(ts_file_paths):
        logging.debug(f"Updating {ts_file_path} ({ts_file_index+1}/{len(ts_file_paths)})")

        # We need to temporarily copy the .ts file to the source folder because paths in the .ts file are relative to the .ts file location
        ts_file_path_in_source_tree = os.path.join(source_code_dir, os.path.basename(ts_file_path))
        shutil.copy(ts_file_path, ts_file_path_in_source_tree)

        # Use "." as lupdate source path (and set the current working directory to source_code_dir),
        # because if we use an absolute path then lupdate misses tr() functions in some .h files
        # (for example, loadable module names that are specified in the module file header were not found).
        command = [lupdate_path, ".", "-tr-function-alias", "QT_TRANSLATE_NOOP+=vtkMRMLTr", "-ts", ts_file_path_in_source_tree]

        if remove_obsolete_strings:
            command.append("-noobsolete")

        proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=sys.stderr, cwd=source_code_dir)
        data, err = proc.communicate()
        if proc.returncode != 0:
            sys.exit(proc.returncode)

        shutil.move(ts_file_path_in_source_tree, ts_file_path)


def _generate_translation_header_from_cli_xml(cli_xml_filename):
    logging.debug(f"Generating header file for Qt translation from {cli_xml_filename}")

    def to_translation_header(translation_context, text):
        """Takes a single or a list of strings or XML elements and creates a line for each item
        that can be placed in the .h file that lupdate will parse for translations.

        Example line::

            QT_TRANSLATE_NOOP("CLI_ACPCTransform", "ACPC Transform")
        """
        if type(text) != list:
            text_list = [text]
        else:
            text_list = text
        result = ""
        for text_item in text_list:
            if text_item is None:
                continue
            try:
                element_text = text_item.text
            except AttributeError:
                element_text = text_item
            if element_text is None:
                continue
            element_text = element_text.replace('\n', '\\n')
            element_text = element_text.replace('\"', '\\"')
            result += f'QT_TRANSLATE_NOOP("{translation_context}", "{element_text}")\n'
        return result

    import xml.etree.ElementTree as ET
    tree = ET.parse(cli_xml_filename)
    root = tree.getroot()

    translation_context = "CLI_" + os.path.splitext(os.path.basename(cli_xml_filename))[0]

    cpp_header_str = f"// Generated automatically by update_translations.py from {os.path.basename(cli_xml_filename)}\n\n"

    # Module information
    cpp_header_str += to_translation_header("qSlicerAbstractCoreModule", root.find('category').text.split('.'))
    cpp_header_str += to_translation_header(translation_context, root.find('title'))
    cpp_header_str += to_translation_header(translation_context, root.find('description'))
    cpp_header_str += to_translation_header(translation_context, root.find('acknowledgements'))

    # Parameters
    for parameter in root.findall('parameters'):
        cpp_header_str += to_translation_header(translation_context, parameter.find('label'))
        cpp_header_str += to_translation_header(translation_context, parameter.find('description'))
        for elem in parameter.findall('./*/label'):
            cpp_header_str += to_translation_header(translation_context, elem)
        for elem in parameter.findall('./*/description'):
            cpp_header_str += to_translation_header(translation_context, elem)
        for elem in parameter.findall('./string-enumeration/default'):
            cpp_header_str += to_translation_header(translation_context, elem)
        for elem in parameter.findall('./string-enumeration/element'):
            cpp_header_str += to_translation_header(translation_context, elem)

    # Write to file
    cpp_header_path = os.path.splitext(cli_xml_filename)[0] + "_tr.h"
    logging.info("Writing output file: " + cpp_header_path)
    with open(cpp_header_path, 'w', encoding='utf8') as cpp_header_file:
        cpp_header_file.write(cpp_header_str)


def extract_translatable_from_cli_modules(source_code_dir):
    """For each CLI *.xml files it creates a *_tr.h file that Qt lupdate can consume.
    """

    # List of folders that contain CLI module descriptor XML files.
    inputpaths = [
        os.path.join(source_code_dir, "Modules", "CLI"),
        os.path.join(source_code_dir, "Docs", "_extracli"),
    ]

    # List of modules to be excluded from documentation generation
    # (for example, testing modules only).
    excludenames = [
        'CLIROITest.xml',
        'TestGridTransformRegistration.xml',
        'DiffusionTensorTest.xml',
    ]

    for inputpath in inputpaths:
        for root, dirs, files in os.walk(inputpath):
            for name in files:
                if name in excludenames:
                    continue
                if name.endswith(".xml"):
                    _generate_translation_header_from_cli_xml(os.path.join(root, name))


def main(argv):
    parser = argparse.ArgumentParser(description="Update Qt translation files")
    parser.add_argument("--lupdate", default="lupdate", dest="lupdate_path",
                        help="location of the Qt lupdate executable")
    parser.add_argument("-s", "--source-code", metavar="DIR", dest="source_code_dir",
                        help="folder of application source code (root of repository, such as https://github.com/Slicer/Slicer)")
    parser.add_argument("-c", "--component", dest="component", default="Slicer",
                        help="translation component name (Slicer, CTK, ...)")
    parser.add_argument("-t", "--translations", dest="translations_dir", metavar="DIR",
                        default="-",
                        help="folder containing .ts translation files that will be updated (translations folder of https://github.com/Slicer/SlicerLanguageTranslations)")
    parser.add_argument("-l", "--language", default=None, dest="language",
                        help="choose specific ts file to update by language (e.g., use en-US to update only US English translation file)")
    parser.add_argument("-r", "--remove-obsolete-strings", default=False, dest="remove_obsolete_strings", action='store_true',
                        help="removes obsolete source strings (by calling lupdate with -noobsolete argument)")
    parser.add_argument("-v", "--verbose", default=False, dest="verbose", action='store_true',
                        help="show more progress information")
    args = parser.parse_args(argv)

    if args.verbose:
        logging.basicConfig(format='%(message)s', level=logging.DEBUG)

    if args.component == "Slicer":
        extract_translatable_from_cli_modules(args.source_code_dir)
    update_translations(args.component, args.source_code_dir, args.translations_dir, args.lupdate_path, args.language, args.remove_obsolete_strings)


if __name__ == "__main__":
    main(sys.argv[1:])
