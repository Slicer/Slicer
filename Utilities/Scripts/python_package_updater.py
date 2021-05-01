import subprocess
import sys
from argparse import ArgumentParser
import os
import re
import requests


def get_installed_packages():
    """Get the output of installed python packages as a list of lines."""
    output = subprocess.check_output([sys.executable, "-m", "pip", "list"])
    pip_list = output.decode()
    print(pip_list)
    lines = pip_list.split("\n")
    return lines


def get_outdated_packages():
    """Get the output of outdated installed python packages as a list of lines."""
    output = subprocess.check_output([sys.executable, "-m", "pip", "list", "--outdated"])
    pip_list_outdated = output.decode()
    print(pip_list_outdated)
    lines = pip_list_outdated.split("\n")
    return lines


def update_external_project_python_packages(packages_to_update, directory, cpython_tag):
    """
    Find and replace outdated python package versions and hashes specified in external python project files.

    Input: Packages to Update (list of str), Directory to search and update python packages (str), CPython Tag (str) (eg "cp36")
    """
    python_version_info = sys.version_info
    interpreter_cpython_tag = f"cp{python_version_info.major}{python_version_info.minor}"

    indentation = 2
    lines_to_write = {}
    for i in range(2, len(packages_to_update) - 1):  # First two lines are headings, so skip parsing these. Last line is empty
        details = packages_to_update[i].split()  # ['asn1crypto', '0.24.0', '1.4.0', 'wheel']
        package_name = details[0]
        current_version = details[1]

        if package_name in ["vtk", "simpleitk"]:
            continue  # Slicer python wraps VTK and SimpleITK instead of installing the official python package from PyPI

        url = f"https://pypi.org/pypi/{package_name}/json"
        url_request = requests.get(url)
        data = url_request.json()

        if cpython_tag == interpreter_cpython_tag:
            latest_version = details[2]
            desired_version = latest_version
        else:
            desired_version = current_version

        filenames = []
        hashes = []
        desired_version_files = data["releases"][desired_version]
        for release_file in desired_version_files:
            if release_file["python_version"] not in ["py3", "py2.py3", cpython_tag]:
                continue  # means we did 'pip list --outdated' earlier which confirmed version supports our python version
            if release_file["packagetype"] != "bdist_wheel":
                continue  # Only want to install with wheels as building from source can require complex build tools
            filename = release_file["filename"]
            if not filename.endswith(("py3-none-any.whl", "64.whl")):  # win_amd64.whl, aarch64.whl, x86_64.whl
                continue  # Only want 64-bit wheels
            wheel_hash = release_file["digests"]["sha256"]
            filenames.append(" " * indentation + f"#  - {filename}")
            hashes.append(f"--hash=sha256:{wheel_hash}")
        if not hashes:
            print(f"ERROR UPDATING '{package_name}': Unable to find latest version for specified python version." +
                  "This package might need an updated package version for the specified CPython tag.")
            continue
        simple_package_version = " " * indentation + f"{package_name}=={desired_version} "
        hashes_joiner_text = " \\\n" + " " * len(simple_package_version)  # new line for each additional hash and with indentation to line up vertically
        all_hashes = simple_package_version + hashes_joiner_text.join(hashes)

        # Only include filenames if there are multiple wheels for a given package
        if len(filenames) > 1:
            filenames.insert(0, " " * indentation + "# Hashes correspond to the following packages:")
            all_filenames = "\n".join(filenames)
            text = all_filenames + "\n" + all_hashes
        else:
            text = all_hashes

        print(text)  # Show progress in terminal
        lines_to_write[package_name] = f"{text}\n"

    for dirpath, _, filenames, in os.walk(directory):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            with open(filepath) as open_file:
                file_text = open_file.read()

            for package_name, updated_line in lines_to_write.items():
                regex = fr"(# \[{package_name}]).*?(# \[/{package_name}])"
                updated_line = f"# [{package_name}]" + "\n" + updated_line + " " * indentation + f"# [/{package_name}]"
                file_text = re.sub(regex, updated_line, file_text, flags=re.DOTALL)  # but new lines in the unlimited match messing things up
                with open(filepath, "w") as open_file:
                    open_file.write(file_text)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.description = """
        Use this script to update external python project files with the latest version
        and the download hashes for the specified python modules.

        For updating Slicer python packages for https://github.com/Slicer/Slicer/tree/master,
        run using PythonSlicer.exe from the latest Slicer preview.
        In the specified search directory it will then locally update all the external python
        projects to have the latest version of the package with updated hashes.

        This script DOES NOT handle python package version incompatibilities or the
        addition/removal of other package dependencies for a given package.
    """
    parser.add_argument('-s', '--search-directory', metavar="Path/To/Directory", required=False, help="Directory to search and replace python version info")
    parser.add_argument('-c', '--cpython-tag', metavar="cp{Major}.{Minor}", required=False, help="CPython version of python packages to check for")
    args = parser.parse_args()

    search_directory = args.search_directory
    if not search_directory:
        # Assume script is in cloned Slicer repo, so choose the SuperBuild directory to search
        search_directory = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), "SuperBuild")

    python_version_info = sys.version_info
    interpreter_cpython_tag = f"cp{python_version_info.major}{python_version_info.minor}"

    cpython_tag = args.cpython_tag
    if not cpython_tag:
        # Assume script is updating python package versions for same cpython version being used to run script
        cpython_tag = interpreter_cpython_tag
    if cpython_tag == interpreter_cpython_tag:
        packages_to_update = get_outdated_packages()
    else:
        packages_to_update = get_installed_packages()
    update_external_project_python_packages(packages_to_update, search_directory, cpython_tag)
