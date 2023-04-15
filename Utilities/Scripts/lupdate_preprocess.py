#!/usr/bin/env python

"""
Preprocess all python scripted modules files before they get extracted by lupdate
It rewrites translation function calls so that they can be processed by ludpate.
Function calls like _("Hello") becomes translate("ContextName", "Hello")
This script is used by "update_translations.py"

Example use:

    python.exe c:/D/S5/Utilities/Scripts/lupdate_preprocess.py -i c:/D/S5/Base/Python -o c:/D/S5/Base/Python

    For more usage details:

    python.exe c:/D/S5/Utilities/Scripts/lupdate_preprocess.py -h

"""

import os
import re
import logging
import argparse


def get_context(source_file):
    if os.path.isfile(source_file):
        parent_folder = os.path.dirname(source_file)
        init_file_path = parent_folder + os.path.sep + '__init__.py'

        if os.path.isfile(init_file_path):
            context_name = os.path.basename(parent_folder)
            context_name += '.' + os.path.basename(source_file).replace('.py', '')
            return context_name
        else:
            return os.path.basename(source_file).replace('.py', '')
    else:
        return os.path.basename(source_file)


def find_all_py_files(root_dir, relative_path=True):
    python_files = []
    root_dir = os.path.realpath(root_dir)

    for root, folders, files in os.walk(root_dir):
        python_files += [os.path.join(root, file) for file in files if file.endswith('.py')]
    files = [py_file.replace(root_dir, '')[1:] for py_file in python_files] if relative_path else python_files
    return files


def get_source_code(source_file):
    file = open(source_file)
    source_code = file.read()
    file.close()

    return source_code


def save_source_code(source_code, source_file):
    file = open(source_file, 'w')
    file.write(source_code)
    file.close()


def transform_translate_function(source_file, tr_function_name='translate'):
    context_name = get_context(source_file)
    source_code = get_source_code(source_file)

    # Search in the source code any occurrence of the translation function call pattern and replace
    # it with the transformed translation function call.
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
    transformed_function_text = r'\1' + tr_function_name + '("' + context_name + '", ' + r'\2' + ')'

    source_code = re.sub(r'([^_])_\((\s*?".+?"\s*?)\)', transformed_function_text, source_code, flags=re.DOTALL)
    source_code = re.sub(r"([^_])_\((\s*?'.+?'\s*?)\)", transformed_function_text, source_code, flags=re.DOTALL)
    source_code = re.sub(r'([^_])_\((\s*?""".+?"""\s*?)\)', transformed_function_text, source_code, flags=re.DOTALL)
    source_code = re.sub(r"([^_])_\((\s*?'''.+?'''\s*?)\)", transformed_function_text, source_code, flags=re.DOTALL)

    return source_code


def transform_single_file(input_file, output_file, tr_function_name):
    transformed_code = transform_translate_function(input_file, tr_function_name)
    save_source_code(transformed_code, output_file)
    logging.debug(f"\n[+] Processed file saved in <{output_file}> file")


def transform_files_list(files_list_path, tr_function_name, root_dir=''):
    if os.path.isfile(files_list_path):
        with open(files_list_path) as file:
            input_files = file.readlines()

        # Filter out non python files
        input_files = [input_file.strip() for input_file in input_files if input_file.strip().endswith('.py')]

        processed_file_count = 0
        for input_file in input_files:
            input_file = os.path.join(root_dir, input_file)
            output_file = input_file
            try:
                transform_single_file(input_file, output_file, tr_function_name)
                processed_file_count += 1
            except Exception as e:
                logging.warning(f"\n\t[-] Error while saving '{output_file}': \n\t\t{str(e)}")
        logging.debug(f"\n[+] {processed_file_count} / {len(input_files)} file(s) processed")
    else:
        logging.warning(f"\n\t[-] Error: the specified '{files_list_path}' is not a file")


def transform_folder_files(input_folder, output_folder, tr_function_name):
    if not os.path.isdir(output_folder):
        os.mkdir(output_folder)

    python_files = find_all_py_files(input_folder)

    processed_file_count = 0
    for file in python_files:
        output_file = os.path.join(output_folder, file)
        output_file_directory = os.path.dirname(output_file)
        file = os.path.join(input_folder, file)

        try:
            if not os.path.isdir(output_file_directory):
                os.makedirs(output_file_directory)
            transform_single_file(file, output_file, tr_function_name)
            processed_file_count += 1
        except Exception as e:
            logging.warning(f"\n\t[-] Error while saving '{output_file}': \n\t\t{str(e)}")

    logging.debug(f"\n[+] {processed_file_count} / {len(python_files)} file(s) processed")
    logging.debug(f"\n[+] Processed file(s) saved in <{output_folder}> folder")


def main():
    parser = argparse.ArgumentParser(
        prog='lupdate_preprocess',
        usage='%(prog)s [options]',
        description="Make translation function calls suitable for lupdate extraction"
    )
    parser.add_argument("-i", "--input", type=str,
                        help="input file/folder path (starts with @ when specifying a files list)")
    parser.add_argument("-o", "--output", type=str,
                        help="output file/folder path. Is specified only when the input path is not a files list")
    parser.add_argument("-f", "--funcname", type=str, metavar='TR_FUNCTION_NAME', default='translate',
                        help="the translation function name (e.g. tr/translate)")
    parser.add_argument("-r", "--rootdir", type=str, metavar='FILES_LIST_ROOT_DIR', default='',
                        help="used when the input path is a files list so that to specify an optional root folder path")
    parser.add_argument("-v", "--verbose", default=False, dest="verbose", action='store_true',
                        help="show more progress information")

    args = parser.parse_args()
    files_list_is_specified = args.input.startswith('@')

    if (not args.input
            or (args.output and files_list_is_specified)
            or (not args.output and not files_list_is_specified)
            or (not files_list_is_specified and args.rootdir)):
        print("\nSyntax error, please retry by specifying the correct parameters\n")
        parser.print_help()
        return

    if args.verbose:
        logging.basicConfig(format='%(message)s', level=logging.DEBUG)

    input_path = args.input
    tr_function_name = args.funcname

    if os.path.isfile(input_path):
        transform_single_file(input_path, args.output, tr_function_name)
    elif files_list_is_specified:
        files_list_path = input_path[1:]
        transform_files_list(files_list_path, tr_function_name, args.rootdir)
    elif os.path.isdir(input_path):
        output_folder = args.output
        transform_folder_files(input_path, output_folder, tr_function_name)
    else:
        logging.warning(f"\n\t[-] Error: the input '{input_path}' doesn't match any expectation")

    logging.debug("\n[+] End of file processing...")


if __name__ == '__main__':
    main()
