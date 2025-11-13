#!/usr/bin/env python
# -*- i18n: disabled -*-

"""Update all translation source (.ts) files from the source code

Example use:

    python.exe c:/D/S5/Utilities/Scripts/update_translations.py -c Slicer -s c:/D/S5 -t c:/D/SlicerLanguageTranslations/translations
      --lupdate c:/Qt/6.3.0/msvc2019_64/bin/lupdate.exe
      -f "/modules/,/Base/Python/slicer/util.py,/Base/Python/slicer/ScriptedLoadableModule.py"

If a Python source file contains a `-*- i18n: disabled -*-` string then the file will not be patched.
This can be used to prevent translation of development and test files.

"""

import argparse
import glob
import json
import logging
import os
import re
import shutil
import subprocess
import sys

def detect_tr_function_name(source_code_bytes):
    """
    Detect if the file uses tr() or translate() as translation function with _() alias.
    Returns 'tr' if tr is found, 'translate' if translate is found, otherwise 'tr'.
    """
    source_code = source_code_bytes.decode(errors="ignore")
    # Look for function import of tr
    if re.search(r'(^|\n)\s*from\s+.+\s+import\s+tr(\s)as _', source_code):
        return "tr"
    # Look for function import of translate
    if re.search(r'(^|\n)\s*from\s+.+\s+import\s+translate(\s)as _', source_code):
        return "translate"
    return "tr"

def get_python_context(source_file):
    if os.path.isfile(source_file):
        parent_folder = os.path.dirname(source_file)
        init_file_path = parent_folder + os.path.sep + "__init__.py"

        if os.path.isfile(init_file_path):
            context_name = os.path.basename(parent_folder)
            context_name += "." + os.path.basename(source_file).replace(".py", "")
            return context_name
        else:
            return os.path.basename(source_file).replace(".py", "")
    else:
        return os.path.basename(source_file)


def patch_python_source(source_code, context_name, tr_function_name_before, tr_function_name="translate"):
    """
    Search in the source code any occurrence of the translation function call pattern and replace
    it with the transformed translation function call.
    """

    # The translation function call pattern is as follows : _(FUNCTION_PARAMETER)
    # The FUNCTION_PARAMETER is as follows :
    # ZERO_OR_MANY_BLANK_CHARACTERS + STRING_DELIMITER + TEXT + STRING_DELIMITER + ZERO_OR_MANY_BLANK_CHARACTERS
    # The STRING_DELIMITER might be ", ', """ or '''.
    # That's why four different replacements that take into account those delimiters are made
    #
    # The proposed function call pattern excludes cases where the _() call is preceded by an underscore.
    # It allows to avoid false positives like in : super().__init__()
    #
    # Example 1 : _("Hello")     ==> translate("ContextName", "Hello")
    # Example 2 : _('Hello')     ==> translate("ContextName", 'Hello')
    # Example 3 : _("""Hello""") ==> translate("ContextName", """Hello""")
    # Example 4 : _('''Hello''') ==> translate("ContextName", '''Hello''')
    # Example 5 : super().__init__("Hello") ==> super().__init__("Hello")

    # \1 refers to any character that comes before the _(...), except _
    # \2 refers to the translation function parameter, like Y in _(Y)
    if tr_function_name_before == "tr":
        transformed_function_text = r"\1" + tr_function_name + '("' + context_name + '", ' + r"\2" + ")"
    else:  # translate: do not add context
        transformed_function_text = r"\1" + tr_function_name + '(' + r"\2" + ")"

    source_code = source_code.decode()

    source_code = re.sub(r'([^_])_\((\s*?".+?"\s*?)\)', transformed_function_text, source_code, flags=re.DOTALL)
    source_code = re.sub(r"([^_])_\((\s*?'.+?'\s*?)\)", transformed_function_text, source_code, flags=re.DOTALL)
    source_code = re.sub(r'([^_])_\((\s*?""".+?"""\s*?)\)', transformed_function_text, source_code, flags=re.DOTALL)
    source_code = re.sub(r"([^_])_\((\s*?'''.+?'''\s*?)\)", transformed_function_text, source_code, flags=re.DOTALL)

    source_code = source_code.encode()

    return source_code


def patch_python_files(source_files, root_dir):
    """Patch translation function in all files in the source_files list.
    Original (unpatched) files are saved as *.pyTrSource.
    """

    logging.debug("Patch Python files for translation")

    for source_file in source_files:
        # Skip non-Python files
        if not str(source_file).endswith(".py"):
            continue

        source_file = os.path.join(root_dir, source_file)
        original_source_file = os.path.splitext(source_file)[0] + ".pyTrSource"

        if os.path.isfile(original_source_file):
            # Original file already exists, probably remained from a previous abrupted
            # patching process, so use that.
            with open(original_source_file, "rb") as file_object:
                original_source_code = file_object.read()
            with open(source_file, "wb") as file_object:
                file_object.write(original_source_code)
        else:
            # Original file content is in the source file
            with open(source_file, "rb") as file_object:
                original_source_code = file_object.read()

        context_name = get_python_context(source_file)
        tr_function_name = detect_tr_function_name(original_source_code)
        patched_source_code = patch_python_source(original_source_code, context_name, tr_function_name_before=tr_function_name)
        if patched_source_code == original_source_code:
            # The file has not needed patching, remove backup copy if existed
            if os.path.isfile(original_source_file):
                os.remove(original_source_file)
        else:
            # The file was patched, save original file content so that we can later restore it
            with open(original_source_file, "wb") as file_object:
                file_object.write(original_source_code)
            with open(source_file, "wb") as file_object:
                file_object.write(patched_source_code)


def restore_patched_python_files(source_files, root_dir):
    """Restore Python files patched by patch_python_files()
    from the saved *.pyTrSource files.
    """

    logging.debug("Restore patched Python files after for translation")

    for source_file in source_files:
        # Skip non-Python files
        if not str(source_file).endswith(".py"):
            continue

        source_file = os.path.join(root_dir, source_file)
        original_source_file = os.path.splitext(source_file)[0] + ".pyTrSource"
        if os.path.isfile(original_source_file):
            # Restore Python file from source
            with open(original_source_file, "rb") as file_object:
                source_code = file_object.read()
            with open(source_file, "wb") as file_object:
                file_object.write(source_code)
            os.remove(original_source_file)


def get_lupdate_version(lupdate_path):
    """Get version of lupdate. There are significant differences between lupdate capabilities in Qt5 and Qt6."""
    output = subprocess.check_output([lupdate_path, "-version"]).decode()  # returns 'lupdate version 5.15.2\r'
    m = re.match(r"lupdate version ([0-9]+)\.([0-9]+)\.([0-9]+).*", output)
    return [int(m.groups()[0]), int(m.groups()[1]), int(m.groups()[2])]


def update_translations(component, source_code_dir, translations_dir, lupdate_path,
                        language=None, remove_obsolete_strings=False, source_file_regex=None, keep_temporary_files=False):

    if language is None:
        ts_filename_filter = f"{component}_*.ts"
    else:
        ts_filename_filter = f"{component}_{language}.ts"

    ts_file_filter = f"{translations_dir}/{ts_filename_filter}"
    ts_file_paths = glob.glob(ts_file_filter)
    create_new_component = False
    if not ts_file_paths:
        if not language:
            language = "en-US"
        logging.warning(f"No .ts files were found at {ts_file_filter}. A new translation source file will be created for {component} in {language} language.")
        create_new_component = True

    # Get list of translatable files based on file extension

    translatableFilesListPath = source_code_dir + "/TranslatableFilesList.txt"
    logging.debug(f"Discovering translatable files, writing to {translatableFilesListPath}")

    extensions = ["java", "jui", "ui", "c", "c++", "cc", "cpp", "cxx", "ch", "h", "h++", "hh", "hpp", "hxx", "js", "qs", "qml", "qrc"]  # defaults from Qt-6.3

    # Only add py to extensions if lupdate version supports it
    translate_python_files = False
    lupdate_version = get_lupdate_version(lupdate_path)
    if lupdate_version[0] * 100 + lupdate_version[1] >= 603:
        extensions.append("py")
        translate_python_files = True
    else:
        lupdate_version_str = f"{lupdate_version[0]}.{lupdate_version[1]}.{lupdate_version[2]}"
        logging.warning(f"lupdate version {lupdate_version_str} does not support Python, therefore .py files will be ignored")

    # Traverse the source code directory and collect all translatable files
    from pathlib import Path

    source_files = []
    for ext in extensions:
        source_files.extend(Path(source_code_dir).rglob("*." + ext))

    # Remove grouping based on file extension (sort based on folder structure)
    source_files.sort()

    # Create final file list using filtering and write out paths to translatableFilesList file
    source_files_filtered = []
    logging.debug(f"Filter source files with regex: {source_file_regex}")
    with open(translatableFilesListPath, "w") as f:
        for source_file in source_files:
            relative_path = str(source_file.relative_to(Path(source_code_dir)))

            # Apply filename filter if specified. For example, some files may need to be excluded
            # because lupdate crashes when it tries to parse some complex Python files.
            if source_file_regex:
                if not re.match(source_file_regex, relative_path):
                    continue

            # Skip Python file if internationalization is explicitly disabled in it
            if str(source_file).endswith(".py"):
                with open(source_file) as file_object:
                    source_code = file_object.read()
                if "-*- i18n: disabled -*-" in source_code:
                    # Translation of the file is explicitly disabled
                    logging.debug(f"Translation of {source_file} was disabled in the source code.")
                    continue

            # Add file to list of translatable files
            source_files_filtered.append(source_file)
            f.write(relative_path + "\n")

    source_files = source_files_filtered

    ts_file_filter = f"{translations_dir}/{ts_filename_filter}"
    ts_file_paths = glob.glob(ts_file_filter)

    # Patch Python files to replace Pythonic `_(message)` translation function by Qt-style `translate(context, message)`
    # to allow lupdate to translate. Original files are backed up before any modification
    if translate_python_files:
        patch_python_files(source_files, source_code_dir)

    # Run lupdate for each language

    if create_new_component:
        ts_file_paths = [f"{translations_dir}/{component}_{language}.ts"]

    for ts_file_index, ts_file_path in enumerate(ts_file_paths):
        logging.debug(f"Updating {ts_file_path} ({ts_file_index+1}/{len(ts_file_paths)})")

        # We need to temporarily copy the .ts file to the source folder because paths in the .ts file are relative to the .ts file location
        ts_file_path_in_source_tree = os.path.join(source_code_dir, os.path.basename(ts_file_path))
        if not create_new_component:
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
            raise RuntimeError(f"lupdate failed with exit code {proc.returncode}:\n\nCommand: {' '.join(command)}\n\nData: {data}\n\nError: {err}\n")

        shutil.move(ts_file_path_in_source_tree, ts_file_path)

    if not keep_temporary_files:
        # Restore original (non-patched) Python files after lupdate extraction
        if translate_python_files:
            restore_patched_python_files(source_files, source_code_dir)

        # If transformations are made, restore transformed files after lupdate extraction
        os.remove(translatableFilesListPath)


def _generate_translation_header_from_cli_xml(cli_xml_filename):
    logging.debug(f"Generating header file for Qt translation from {cli_xml_filename}")

    def to_translation_header(translation_context, text):
        """Takes a single or a list of strings or XML elements and creates a line for each item
        that can be placed in the .h file that lupdate will parse for translations.

        Example line::

            QT_TRANSLATE_NOOP("CLI_AddScalarVolumes", "Add Scalar Volumes")
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
            element_text = element_text.replace("\n", "\\n")
            element_text = element_text.replace('"', '\\"')
            result += f'QT_TRANSLATE_NOOP("{translation_context}", "{element_text}")\n'
        return result

    import xml.etree.ElementTree as ET

    try:
        tree = ET.parse(cli_xml_filename)
    except ET.ParseError:
        # Not a CLI module descriptor XML file
        return False

    root = tree.getroot()

    translation_context = "CLI_" + os.path.splitext(os.path.basename(cli_xml_filename))[0]

    if not root.find("executable") or not root.find("title"):
        # Not a CLI module descriptor XML file
        return False

    cpp_header_str = f"// Generated automatically by update_translations.py from {os.path.basename(cli_xml_filename)}\n\n"

    # Module information
    cpp_header_str += to_translation_header("qSlicerAbstractCoreModule", root.find("category").text.split("."))
    cpp_header_str += to_translation_header(translation_context, root.find("title"))
    cpp_header_str += to_translation_header(translation_context, root.find("description"))
    cpp_header_str += to_translation_header(translation_context, root.find("acknowledgements"))

    # Parameters
    for parameter in root.findall("parameters"):
        cpp_header_str += to_translation_header(translation_context, parameter.find("label"))
        cpp_header_str += to_translation_header(translation_context, parameter.find("description"))
        for elem in parameter.findall("./*/label"):
            cpp_header_str += to_translation_header(translation_context, elem)
        for elem in parameter.findall("./*/description"):
            cpp_header_str += to_translation_header(translation_context, elem)
        for elem in parameter.findall("./string-enumeration/default"):
            cpp_header_str += to_translation_header(translation_context, elem)
        for elem in parameter.findall("./string-enumeration/element"):
            cpp_header_str += to_translation_header(translation_context, elem)

    # Write to file
    cpp_header_path = os.path.splitext(cli_xml_filename)[0] + "_tr.h"
    logging.info("Writing output file: " + cpp_header_path)
    with open(cpp_header_path, "w", encoding="utf8") as cpp_header_file:
        cpp_header_file.write(cpp_header_str)


def extract_translatable_from_cli_modules(input_paths, exclude_filenames=None):
    """For each CLI *.xml files it creates a *_tr.h file that Qt lupdate can consume.
    :param input_paths: List of folders that contain CLI module descriptor XML files.
    :param exclude_names: List of modules to be excluded from documentation generation
    (for example, testing modules only).

    """

    if exclude_filenames is None:
        exclude_filenames = []

    for input_path in input_paths:
        for root, dirs, files in os.walk(input_path):
            for name in files:
                if name in exclude_filenames:
                    continue
                if name.endswith(".xml"):
                    _generate_translation_header_from_cli_xml(os.path.join(root, name))

def extract_translatable_from_json_files(input_paths):
    """For each JSON file it creates a *_tr.h file that Qt lupdate can consume.
    The JSON file is translatable if it has a schema and the schema specifies translatable properties.
    Translatable properties are designated in the schema file by the "translatable" flag.
    The schema file is searched in the the same folder, `Schema` folder, or `Schemas` folder by the filename
    myfile-schema.json ( where myfile is the name of the JSON file without extension) or the filename specified in the
    `$schema` property of the JSON file.
    :param input_paths: List of folders that may contain JSON files.
    """

    for input_path in input_paths:
        for root, dirs, files in os.walk(input_path):
            for name in files:
                if name.endswith(".json"):
                    json_filepath = os.path.join(root, name)
                    property_names = _get_translatable_property_names_from_json_file(json_filepath)
                    if not property_names:
                        # No translatable properties found in the schema, skip this file
                        continue
                    _generate_translation_header_from_json_file(json_filepath, property_names)

def _get_schema_path_from_json_file(filename):
    """Get the schema file path for a JSON file.
    The schema file is searched in the the same folder, `Schema` folder, or `Schemas` folder by the filename
    myfile-schema.json ( where myfile is the name of the JSON file without extension) or the filename specified in the
    `$schema` property of the JSON file.
    :param filename: Path to the JSON file.
    :return: Path to the schema file or None if not found.
    """

    # Try to find myfile-schema.json file
    candidate_folders = [os.path.dirname(filename), os.path.join(os.path.dirname(filename), "Schema"), os.path.join(os.path.dirname(filename), "Schemas")]
    for folder in candidate_folders:
        schema_path = os.path.join(folder, os.path.splitext(os.path.basename(filename))[0] + "-schema.json")
        if os.path.isfile(schema_path):
            # Schema file is found
            return schema_path

    # Try to find schema file that is specified in the $schema property of the JSON file
    # (network is not accessed, it is searched in local folders only)
    schema_filename = None
    try:
        with open(filename, encoding="utf8") as f:
            json_data = json.load(f)
        schema_url = json_data["$schema"]
        # schema_url example:
        # https://raw.githubusercontent.com/Slicer/Slicer/main/Modules/Loadable/Volumes/Resources/Schema/volumes-display-presets-schema-v1.0.0.json#
        schema_filename = os.path.basename(schema_url).split("#")[0]
    except:
        # No schema filename was found
        return None
    if not schema_filename:
        # No schema filename was found
        return None
    for folder in candidate_folders:
        schema_path = os.path.join(folder, schema_filename)
        if os.path.isfile(schema_path):
            # Schema file is found
            return schema_path

    # Schema file was not found
    return None

def _get_translatable_property_names_from_json_file(filename):
    """Get all translatable property names from a JSON file.
    It uses the "translatable" custom flag.
    """

    # find the file-schema.json in the same folder as the json file
    schema_filename = _get_schema_path_from_json_file(filename)
    if not schema_filename:
        return []

    try:
        with open(schema_filename) as f:
            schema = json.load(f)
    except json.JSONDecodeError as e:
        logging.error(f"Failed to decode JSON schema file '{schema_filename}': {e}")
        return []

    names = set()

    def recurse(subschema):
        if isinstance(subschema, dict):

            # Recurse through object properties
            if "properties" in subschema:
                for prop, child in subschema["properties"].items():
                    if child.get("translatable") is True:
                        names.add(prop)
                    recurse(child)

            # Recurse into array items
            if "items" in subschema:
                recurse(subschema["items"])

            # Recurse into all subschema items
            if "anyOf" in subschema:
                for item in subschema["anyOf"]:
                    recurse(item)
            if "oneOf" in subschema:
                for item in subschema["oneOf"]:
                    recurse(item)
            if "allOf" in subschema:
                for item in subschema["allOf"]:
                    recurse(item)

    recurse(schema)
    return sorted(names)

def _get_string_values_from_json(data, keys):
    """Get all string values from a JSON object that matches any of the the specified keys"""
    values = []
    if isinstance(data, dict):
        for key, value in data.items():
            if isinstance(value, (dict, list)):
                values.extend(_get_string_values_from_json(value, keys))
            elif isinstance(value, str) and key in keys:
                values.append(value)
    elif isinstance(data, list):
        for item in data:
            values.extend(_get_string_values_from_json(item, keys))
    return values

def _generate_translation_header_from_json_file(filename, property_names):
    """Generate a C++ header file from translatable properties found in a json file"""
    logging.debug(f"Generating header file for Qt translation from {filename}")

    file_path = os.path.abspath(filename)
    if not os.path.isfile(file_path):
        raise RuntimeError(f"Failed to extract translatable strings, file not found: {filename}")

    with open(file_path, encoding="utf8") as file:
        import json

        try:
            json_object = json.loads(file.read())
        except json.decoder.JSONDecodeError:
            raise RuntimeError(f"Failed to extract translatable strings, not a valid JSON file: {filename}")

        translation_context = os.path.splitext(os.path.basename(filename))[0]

        cpp_header_str = f"// Generated automatically by update_translations.py from {filename}\n\n"

        translatable_strings = _get_string_values_from_json(json_object, property_names)

        # Get unique strings with keeping the order
        seen = set()
        translatable_strings = [x for x in translatable_strings if x not in seen and not seen.add(x)]

        for translatable_string in translatable_strings:
            translatable_string = translatable_string.replace("\n", "\\n").replace('"', '\\"')
            cpp_header_str += f'QT_TRANSLATE_NOOP("{translation_context}", "{translatable_string}")\n'

        cpp_header_path = os.path.splitext(filename)[0] + "_tr.h"

        logging.info("Writing output file: " + cpp_header_path)
        with open(cpp_header_path, "w", encoding="utf8") as cpp_header_file:
            cpp_header_file.write(cpp_header_str)


def main(argv):
    parser = argparse.ArgumentParser(description="Update Qt translation files")
    parser.add_argument("--lupdate", default="lupdate", dest="lupdate_path",
                        help="location of the Qt lupdate executable. Qt-6.3 is required for Python translation")
    parser.add_argument("-f", "--source-filter", dest="source_filter_regex", default=None,
                        help="regular expression that filters file paths (file is included if the path matches the expression)")
    parser.add_argument("-s", "--source-code", metavar="DIR", dest="source_code_dir",
                        help="folder of application source code (root of repository, such as https://github.com/Slicer/Slicer)",
                        required=True)
    parser.add_argument("-c", "--component", dest="component", default="Slicer",
                        help="translation component name (Slicer, CTK, ...)")
    parser.add_argument("-t", "--translations", dest="translations_dir", metavar="DIR",
                        default="-",
                        help="folder containing .ts translation files that will be updated"
                             " (translations folder of https://github.com/Slicer/SlicerLanguageTranslations)")
    parser.add_argument("-l", "--language", default=None, dest="language",
                        help="choose specific ts file to update by language (e.g., use en-US to update only US English translation file)")
    parser.add_argument("-r", "--remove-obsolete-strings", default=False, dest="remove_obsolete_strings", action="store_true",
                        help="removes obsolete source strings (by calling lupdate with -noobsolete argument)")
    parser.add_argument("-v", "--verbose", default=False, dest="verbose", action="store_true",
                        help="show more progress information")
    parser.add_argument("-k", "--keep-temporary-files", default=False, dest="keep_temporary_files", action="store_true",
                        help="keep temporary files (for debugging the translation process)")
    args = parser.parse_args(argv)

    if args.verbose:
        logging.basicConfig(format="%(message)s", level=logging.DEBUG)

    # Component-specific defaults and additional operations
    if args.component == "Slicer":
        # In Slicer, by default, only translate Python files that are in the Modules folder
        # (some complicated Python files that are outside the Modules folder make lupdate crash)
        if not args.source_filter_regex:
            args.source_filter_regex = "|".join([
                r"^(?!.*\.py$).*$",  # non-Python files
                r"^Modules\\.+\.py$",  # Python files in the Modules folder
            ])

        # Slicer CLI module translation
        cli_input_paths = [
            os.path.join(args.source_code_dir, "Modules", "CLI"),
            os.path.join(args.source_code_dir, "Docs", "_extracli"),
        ]
        cli_exclude_names = [
            "CLIROITest.xml",
            "TestGridTransformRegistration.xml",
            "DiffusionTensorTest.xml",
        ]
        extract_translatable_from_cli_modules(cli_input_paths, cli_exclude_names)
    else:
        extract_translatable_from_cli_modules([args.source_code_dir])

    extract_translatable_from_json_files([args.source_code_dir])

    update_translations(args.component, args.source_code_dir, args.translations_dir, args.lupdate_path,
                        args.language, args.remove_obsolete_strings, args.source_filter_regex, args.keep_temporary_files)


if __name__ == "__main__":
    main(sys.argv[1:])
