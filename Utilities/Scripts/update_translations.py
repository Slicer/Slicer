#!/usr/bin/env python

"""Update all translation source (.ts) files from the source code

Example use:

    python.exe c:/D/S5/Utilities/Scripts/update_translations.py -c Slicer -s c:/D/S5 -t c:/D/SlicerLanguageTranslations/translations
      --lupdate c:/Qt/6.3.0/msvc2019_64/bin/lupdate.exe
      -p c:/D/S5/Utilities/Scripts/lupdate_preprocess.py
      -f "/modules/,/Base/Python/slicer/util.py,/Base/Python/slicer/ScriptedLoadableModule.py"

"""

import argparse
import glob
import logging
import os
import re
import shutil
import subprocess
import sys


def get_python_files(source_code_dir, with_full_path=True):
    """
    Get python files path to transform. Assumes that a 'TranslatableFilesList.txt' that
    contains the list of files to process is directly present in the source code folder.
    """
    translatableFilesListPath = source_code_dir + '/TranslatableFilesList.txt'

    with open(translatableFilesListPath) as file:
        input_files = file.readlines()

    input_files = [file.strip() for file in input_files if file.strip().endswith('.py')]

    if with_full_path:
        input_files = [os.path.join(source_code_dir, file) for file in input_files]

    return input_files


def filter_source_files(source_files, files_pattern_list):
    # Filter out Python files that don't match files list patterns. This is necessary because
    # lupdate crashes when it tries to parse some complex Python files.
    if files_pattern_list:
        from pathlib import fnmatch
        py_files = [source_file for source_file in source_files if (str(source_file).endswith('.py'))]
        filtered_source_files = [source_file for source_file in source_files if (not str(source_file).endswith('.py'))]
        for files_pattern in files_pattern_list:
            files_pattern = f"**{files_pattern}**"
            filtered_source_files += [py_file for py_file in py_files if (fnmatch.fnmatch(py_file, files_pattern))]
        # Remove any potential duplicated file path
        filtered_source_files = list(set(filtered_source_files))
        return filtered_source_files

    return source_files


def save_files_to_transform(source_code_dir):
    input_files = get_python_files(source_code_dir)

    for input_file in input_files:
        # backup files have .pyTrSource extension (py file translation source)
        backup_file = input_file + 'TrSource'

        try:
            shutil.copy(input_file, backup_file)
        except Exception:
            raise RuntimeError(f"Error saving the python file '{input_file}'")


def restore_transformed_files(source_code_dir):
    input_files = get_python_files(source_code_dir)

    for input_file in input_files:
        backup_file = input_file + 'TrSource'  # backup files have .pyTrSource extension

        try:
            shutil.move(backup_file, input_file)
        except Exception:
            raise RuntimeError(f"Error restoring the python file '{input_file}'")


def preprocess_scripted_modules_files(source_code_dir, preprocessor_path):
    translatableFilesListPath = source_code_dir + '/TranslatableFilesList.txt'
    command = ["python", preprocessor_path, "-i", '@' + translatableFilesListPath, "-r", source_code_dir]

    proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=sys.stderr, cwd=source_code_dir)
    data, err = proc.communicate()
    if proc.returncode != 0:
        raise RuntimeError(f"lupdate_preprocess failed with exit code {proc.returncode}:\n\nCommand: {' '.join(command)}\n\n{data}\n\n{err}\n")


def get_lupdate_version(lupdate_path):
    """Get version of lupdate. There are significant differences between lupdate capabilities in Qt5 and Qt6.
    """
    output = subprocess.check_output([lupdate_path, "-version"]).decode()  # returns 'lupdate version 5.15.2\r'
    m = re.match(r"lupdate version ([0-9]+)\.([0-9]+)\.([0-9]+).*", output)
    return [int(m.groups()[0]), int(m.groups()[1]), int(m.groups()[2])]


def update_translations(component, source_code_dir, translations_dir, lupdate_path, lupdate_preprocessor_path=None,
                        files_pattern_list=[], language=None, remove_obsolete_strings=False):

    if language is None:
        ts_filename_filter = f"{component}*.ts"
    else:
        ts_filename_filter = f"{component}*_{language}.ts"

    ts_file_filter = f"{translations_dir}/{ts_filename_filter}"
    ts_file_paths = glob.glob(ts_file_filter)
    if not ts_file_paths:
        raise ValueError(f"No .ts files were found at {ts_file_filter}")

    # Get list of translatable files based on file extension

    translatableFilesListPath = source_code_dir + '/TranslatableFilesList.txt'
    logging.debug(f"Discovering translatable files, writing to {translatableFilesListPath}")

    extensions = ["java", "jui", "ui", "c", "c++", "cc", "cpp", "cxx", "ch", "h", "h++", "hh", "hpp", "hxx", "js", "qs", "qml", "qrc"]  # defaults from Qt-6.3

    # Only add py to extensions if lupdate version supports it
    lupdate_version = get_lupdate_version(lupdate_path)
    if lupdate_version[0] * 100 + lupdate_version[1] >= 603:
        extensions.append("py")
    else:
        lupdate_version_str = f"{lupdate_version[0]}.{lupdate_version[1]}.{lupdate_version[2]}"
        logging.warning(f"lupdate version {lupdate_version_str} does not support Python, therefore .py files will be ignored")

    # Traverse the source code directory and collect all translatable files
    from pathlib import Path
    source_files = []
    for ext in extensions:
        source_files.extend(Path(source_code_dir).rglob('*.' + ext))

    # Filter out some Python files when the lupdate preprocessor is specified.
    # This is necessary because lupdate crashes when it tries to parse some complex Python files.
    if lupdate_preprocessor_path:
        source_files = filter_source_files(source_files, files_pattern_list)

    # Remove grouping based on file extension (sort based on folder structure)
    source_files.sort()

    with open(translatableFilesListPath, 'w') as f:
        for source_file in source_files:
            f.write(str(source_file.relative_to(Path(source_code_dir))) + "\n")

    ts_file_filter = f"{translations_dir}/{ts_filename_filter}"
    ts_file_paths = glob.glob(ts_file_filter)
    if not ts_file_paths:
        raise ValueError(f"No .ts files were found at {ts_file_filter}")

    # Preprocess python files to make translation function calls suitable to lupdate extraction
    # Original files are backed up before any modification
    # This transformation is done only if the lupdate preprocessor path is specified
    if lupdate_preprocessor_path:
        save_files_to_transform(source_code_dir)
        preprocess_scripted_modules_files(source_code_dir, lupdate_preprocessor_path)

    # Run lupdate for each language

    for ts_file_index, ts_file_path in enumerate(ts_file_paths):
        logging.debug(f"Updating {ts_file_path} ({ts_file_index+1}/{len(ts_file_paths)})")

        # We need to temporarily copy the .ts file to the source folder because paths in the .ts file are relative to the .ts file location
        ts_file_path_in_source_tree = os.path.join(source_code_dir, os.path.basename(ts_file_path))
        shutil.copy(ts_file_path, ts_file_path_in_source_tree)

        # Use "." as lupdate source path (and set the current working directory to source_code_dir),
        # because if we use an absolute path then lupdate misses tr() functions in some .h files
        # (for example, loadable module names that are specified in the module file header were not found).
        command = [lupdate_path, "@" + translatableFilesListPath, "-tr-function-alias", "QT_TRANSLATE_NOOP+=vtkMRMLTr", "-ts", ts_file_path_in_source_tree]

        if remove_obsolete_strings:
            command.append("-noobsolete")

        proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=sys.stderr, cwd=source_code_dir)
        data, err = proc.communicate()
        if proc.returncode != 0:
            raise RuntimeError(f"lupdate failed with exit code {proc.returncode}:\n\nCommand: {' '.join(command)}\n\n{data}\n\n{err}\n")

        shutil.move(ts_file_path_in_source_tree, ts_file_path)

    # If transformations are made, restore transformed files after lupdate extraction
    if lupdate_preprocessor_path:
        restore_transformed_files(source_code_dir)


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
                        help="location of the Qt lupdate executable. Qt-6.3 is required for Python translation")
    parser.add_argument("-p", "--preprocessor", dest="preprocessor_path", default=None,
                        help="location of the lupdate preprocessor script. It's generally present in Slicer's Utilities/Scripts folder")
    parser.add_argument("-f", "--filter-patterns", dest="filter_patterns", default=[],
                        help="file path patterns where lupdate preprocessor will search for python files that should be preprocessed (comma-separated paths)")
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

    filter_patterns = args.filter_patterns
    if args.filter_patterns:
        filter_patterns = [file_path.strip() for file_path in args.filter_patterns.split(',')]

    if args.component == "Slicer":
        extract_translatable_from_cli_modules(args.source_code_dir)
        if not args.filter_patterns:
            logging.warning("running the preprocessor on all python files (without any filter pattern) may cause lupdate crash")
    update_translations(args.component, args.source_code_dir, args.translations_dir, args.lupdate_path,
                        args.preprocessor_path, filter_patterns, args.language, args.remove_obsolete_strings)


if __name__ == "__main__":
    main(sys.argv[1:])
