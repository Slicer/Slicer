#!/usr/bin/env python

"""Update all translation source (.ts) files from the source code

Example use:

    python.exe c:/D/S5/Utilities/Scripts/update_translations.py -s c:/D/S5 -t c:/D/SlicerLanguageTranslations/translations --lupdate c:/Qt/5.15.2/msvc2019_64/bin/lupdate.exe

"""

import argparse
import glob
import logging
import os
import shutil
import subprocess
import sys


def update_translations(source_code_dir, translations_dir, lupdate_path, language=None):

    if language is None:
        ts_filename_filter = "*.ts"
    else:
        ts_filename_filter = f"*_{language}.ts"

    ts_file_filter = f"{translations_dir}/{ts_filename_filter}"
    ts_file_paths = glob.glob(ts_file_filter)
    if not ts_file_paths:
        raise ValueError(f"No .ts files were found at {ts_file_filter}")

    for ts_file_path in ts_file_paths:
        logging.debug(f"--- Updating {ts_file_path} ---")

        # We need to temporarily copy the .ts file to the source folder because paths in the .ts file are relative to the .ts file location
        ts_file_path_in_source_tree = os.path.join(source_code_dir, os.path.basename(ts_file_path))
        shutil.copy(ts_file_path, ts_file_path_in_source_tree)

        command = [lupdate_path, source_code_dir, "-locations", "absolute", "-ts", ts_file_path_in_source_tree]
        proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=sys.stderr)
        data, err = proc.communicate()
        if proc.returncode != 0:
            sys.exit(proc.returncode)

        shutil.move(ts_file_path_in_source_tree, ts_file_path)


def main(argv):
    parser = argparse.ArgumentParser(description="Update Qt translation files")
    parser.add_argument("--lupdate", default="lupdate", dest="lupdate_path",
                        help="location of the Qt lupdate executable")
    parser.add_argument("-s", "--source-code", metavar="DIR", dest="source_code_dir",
                        help="folder of application source code (root of https://github.com/Slicer/Slicer)")
    parser.add_argument("-t", "--translations", dest="translations_dir", metavar="DIR",
                        default="-",
                        help="folder containing .ts translation files that will be updated (translations folder of https://github.com/Slicer/SlicerLanguageTranslations)")
    parser.add_argument("-l", "--language", default=None, dest="language",
                        help="choose specific ts file to update by language (e.g., use en-US to update only US English translation file)")
    args = parser.parse_args(argv)

    update_translations(args.source_code_dir, args.translations_dir, args.lupdate_path, args.language)


if __name__ == "__main__":
    main(sys.argv[1:])
