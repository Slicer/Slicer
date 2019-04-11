#!/usr/bin/env python
""" Midas NA-MIC data tree release versioning script.
    This script can be used to duplicate all the Nightly data on Midas to a new folder named by the release version.
    The script does not accept any input arguments. All arguments are to be provided using the option flags. For a list of the option flags, run
        python release.py --help"""

from __future__ import print_function

from optparse import OptionParser
import re, sys

try:
  import pydas
except ImportError as e:
  print(e, "\nInstall pydas or update PYTHONPATH")
  sys.exit(1)

def _error(message):
  """ Print an error message and exit the program """
  sys.stderr.write("error: %s\n" % message)
  sys.exit(1)

def _getFolderIndex(folderChildren, name):
  """Get the index of a subfolder based on its name.
    folderChildren -- Dictionary specifying children of the folder.
    name -- Name of the folder to look for.
    Returns the index of the folder in the dictionary."""
  folders = folderChildren["folders"]
  index = -1
  for folder in folders:
    index = index + 1
    if folder["name"] == name:
      return index
  return -1

def _getIDfromIndex(childrenFolders, entityType, index):
  """ Get the folder_id for a subfolder based on its index in the folder.
    childrenFolders -- Dictionary specifying children of the folder.
    entityType -- folder or item.
    index -- Index of the item in the folder to find the id.
    Returns the integer ID."""
  entity_key = entityType + "s"
  entity = childrenFolders[entity_key]
  id_key = entityType + "_id"
  ID = entity[index][id_key]
  return ID

def itemExists(folderID, itemName, token, communicator):
  """Check if an item exists in a folder based on its name
    folderID -- ID of the folder.
    itemName -- Name of the item to check for.
    token -- Authentication token.
    communicator -- Midas session communicator.
    Returns a boolean indicating if the item exists or not."""
  folderChildren = communicator.folder_children(token, folderID)
  folder_children_items = folderChildren["items"]
  for item in folder_children_items:
    if item["name"] == itemName:
      return True
  return False

def deleteItemByName(folderID, itemName, token, communicator):
  """Delete an item from a folder based on its name
    folderID -- ID of the folder.
    itemName -- Name of the item to delete.
    token -- Authentication token.
    communicator -- Midas session communicator.
    Returns a boolean indicating success or failure."""
  folderChildren = communicator.folder_children(token, folderID)
  folder_children_items = folderChildren["items"]
  for item in folder_children_items:
    if item["name"] == itemName:
      communicator.delete_item(token, item["item_id"])
      return True
  return False

def duplicateItem(itemID, destID, token, communicator):
  """Duplicate an item to a destination folder
    Uses the request web-api as pydas does not expose all methods yet.
    itemID -- ID of item to duplicate.
    destID -- ID of folder to duplicate item in.
    token -- Authentication token.
    communicator -- Midas session communicator."""
  duplicate_params = {}
  duplicate_params["token"] = token
  duplicate_params["id"] = itemID
  duplicate_params["dstfolderid"] = destID
  communicator.request("midas.item.duplicate", duplicate_params)

def duplicateFolderItems(sourceID, destID, token, communicator, overwrite):
  """Duplicate all the child items from a source to a destination
    sourceID -- ID of source folder.
    destID -- ID of destination folder.
    token -- Authentication token.
    communicator -- Midas session communicator.
    overwrite -- Boolean indicating whether to overwrite existing items."""
  folderChildren = communicator.folder_children(token, sourceID)
  folder_children_items = folderChildren["items"]
  if len(folder_children_items) > 0:
    for folder_item in folder_children_items:
      # If item exists and overwrite is True, delete item and duplicate
      folder_item_name = folder_item["name"]
      folder_item_id = folder_item["item_id"]
      item_exists = itemExists(destID, folder_item_name, token, communicator)
      if item_exists:
        if overwrite:
          deleted = deleteItemByName(destID, folder_item_name, token, communicator)
          if not deleted:
            _error("Could not delete existing item: " + folder_item_name + " in dest. folder with ID: " + destID)
          duplicateItem(folder_item_id, destID, token, communicator)
      else:
        duplicateItem(folder_item_id, destID, token, communicator)

def duplicateFolderfolders(sourceID, destID, token, communicator, overwrite):
  """Duplicate all the sub-folders from source to destination
    sourceID -- ID of source folder.
    destID -- ID of destination folder.
    token -- Authentication token.
    communicator -- Midas session communicator.
    overwrite -- Boolean indicating whether to overwrite existing items."""
  folderChildren = communicator.folder_children(token, sourceID)
  folder_subfolders = folderChildren["folders"]

  destFolderChildren = communicator.folder_children(token, destID)

  if len(folder_subfolders) > 0:
    for subfolder in folder_subfolders:
      # If needed, create a corresponding subfolder at the destination
      dst_index = _getFolderIndex(destFolderChildren, subfolder["name"])
      if dst_index == -1:
        dst_folder = communicator.create_folder(token, subfolder["name"], destID)
        dst_folderID = dst_folder["folder_id"]
      else:
        dst_folderID = _getIDfromIndex(destFolderChildren, "folder", dst_index)
      # Duplicate recursively
      duplicateFolderfolders(subfolder["folder_id"], dst_folderID, token, communicator, overwrite)
      # Duplicate all the items from the source subfolder to new dest subfolder
      duplicateFolderItems(subfolder["folder_id"], dst_folderID, token, communicator, overwrite)

def versionDataApplicationDirectory(sourceVersion, destVersion, token, communicator, applicationID, overwrite):
  """Version the Data/Application directory
    sourceVersion -- String indicating source version.
    destVersion -- String indicating destination version.
    token -- Authentication token.
    communicator -- Midas session communicator.
    applicationID -- ID of Application folder.
    overwrite -- Boolean indicating whether to overwrite existing items."""
  availableVersions = communicator.folder_children(token, applicationID)
  sourceIndex = _getFolderIndex(availableVersions, sourceVersion)
  if sourceIndex == -1:
    _error("No folder named " + sourceVersion + " in Application folder")
  sourceID = _getIDfromIndex(availableVersions, "folder", sourceIndex)

  # Create a new folder for destination under Application folder
  print("Creating folder %s under Application directory" % destVersion)
  dest_folder = communicator.create_folder(token, destVersion, applicationID)
  destID = dest_folder["folder_id"]

  # Duplicate the child items from source to destination
  duplicateFolderItems(sourceID, destID, token, communicator, overwrite)

  message = "Duplicating subfolders from %s to %s..." %(sourceVersion, destVersion)
  print(message)
  # Duplicate all the sub-folders from source to destination
  duplicateFolderfolders(sourceID, destID, token, communicator, overwrite)
  print(message + "[DONE]")

def versionDataModulesDirectory(sourceVersion, destVersion, token, communicator, modulesID, ignoreModules, overwrite):
  """Version the Data/Modules directory
    sourceVersion -- String indicating source version.
    destVersion -- String indicating destination version.
    token -- Authentication token.
    communicator -- Midas session communicator.
    modulesID -- ID of Modules folder.
    ignoreModules -- List of modules to ignore while versioning.
    overwrite -- Boolean indicating whether to overwrite existing items."""
  availableModules = communicator.folder_children(token, modulesID)
  availableModulesFolders = availableModules["folders"]
  ignore_indices = []

  # Take modules to be ignored into account
  if len(ignoreModules) > 0:
    for ignore_module in ignoreModules:
      ignore_module_ind = _getFolderIndex(availableModules, ignore_module)
      if ignore_module_ind != -1:
        ignore_indices.append(ignore_module_ind)

  for num_module in range(len(availableModulesFolders)):
    # Do not version if module is to be ignored
    if num_module in ignore_indices:
      continue

    moduleFolderID = _getIDfromIndex(availableModules, "folder", num_module)
    moduleName = availableModulesFolders[num_module]["name"]
    availableVersions = communicator.folder_children(token, moduleFolderID)

    sourceIndex = _getFolderIndex(availableVersions, sourceVersion)
    if sourceIndex == -1:
      _error("No folder named " + sourceVersion + " in module: " + moduleName)
    sourceID = _getIDfromIndex(availableVersions, "folder", sourceIndex)

    # If needed, create a new folder for destination under the module folder
    destIndex = _getFolderIndex(availableVersions, destVersion)
    if destIndex == -1:
      print("Creating folder %s under %s module directory" % (destVersion, moduleName))
      dest_folder = communicator.create_folder(token, destVersion, moduleFolderID)
      destID = dest_folder["folder_id"]
    else:
      print("Re-using existing folder %s under %s module directory" % (destVersion, moduleName))
      destID = _getIDfromIndex(availableVersions, "folder", destIndex)

    # Duplicate the child items from source to destination
    duplicateFolderItems(sourceID, destID, token, communicator, overwrite)

    message = "Duplicating subfolders from %s to %s for %s module..." % (sourceVersion, destVersion, moduleName)
    print(message)
    # Duplicate all the sub-folders from source to destination
    duplicateFolderfolders(sourceID, destID, token, communicator, overwrite)
    print(message + "[DONE]")

def printSourceStructure(modulesID, applicationID, sourceVersion, token, communicator):
  """Print the directory structure of source version in Application and Modules under the data tree
    modulesID -- ID of Modules folder.
    applicationID -- ID of the Application folder
    sourceVersion -- Source version folder name
    token -- Authentication token.
    communicator -- Midas session communicator."""
  # Print Application source version directory structure
  applicationChildren = communicator.folder_children(token, applicationID)
  sourceVersionApplicationIndex = _getFolderIndex(applicationChildren, sourceVersion)
  if sourceVersionApplicationIndex == -1:
    msg = "No folder named " + sourceVersion + " in Application folder."
    _error(msg)
  sourceApplicationID = _getIDfromIndex(applicationChildren, "folder", sourceVersionApplicationIndex)
  print("Application ( folder_id:%s )" % applicationID)
  printFolderStructure(sourceApplicationID, token, communicator, 1)
  print("\n")

  # Print Modules and their directory structure for the source version
  availableModules = communicator.folder_children(token, modulesID)
  availableModulesFolders = availableModules["folders"]
  for module in availableModulesFolders:
    moduleChildren = communicator.folder_children(token, module["folder_id"])
    sourceVersionModuleIndex = _getFolderIndex(moduleChildren, sourceVersion)
    if sourceVersionModuleIndex == -1:
      msg = "No folder named " + sourceVersion + " in module ", module["name"], "."
      print("Warning:", msg)
      continue
    sourceModuleID = _getIDfromIndex(moduleChildren, "folder", sourceVersionModuleIndex)
    print("Module:%s( folder_id:%s )" % (module["name"], sourceModuleID))
    printFolderStructure(sourceModuleID, token, communicator, 1)
    print("\n")


def printFolderStructure(folderID, token, communicator, depth = 0):
  """Print the folder structure of the sourceVersion under the Application folder
    folderID -- ID of the Application folder.
    token -- Authentication token.
    communicator -- Midas session communicator."""
  appFolder = communicator.folder_get(token, folderID)
  for i in range(depth):
    sys.stdout.write("'-")
  sys.stdout.write(appFolder["name"])
  sys.stdout.write(" ( folder_id: ")
  sys.stdout.write(folderID)
  sys.stdout.write(" )")
  childrenFolder = communicator.folder_children(token, folderID)
  if len(childrenFolder["folders"]) > 0:
    for subfolder in childrenFolder["folders"]:
      sys.stdout.write("\n")
      cdepth = depth + 1
      printFolderStructure( subfolder["folder_id"], token, communicator, cdepth)

  if len(childrenFolder["items"]) > 0:
    for item in childrenFolder["items"]:
      sys.stdout.write("\n")
      for i in range(depth+1):
        sys.stdout.write("'-")
      sys.stdout.write(item["name"])
      sys.stdout.write(" ( item_id: ")
      sys.stdout.write(item["item_id"])
      sys.stdout.write(" )")

def versionData(midas_url, email, apikey, sourceVersion, destVersion, data_id, ignore_modules = [], overwrite = False, dry_run = False):
  """ Version Data folder under Midas
    midas_url -- Midas URL.
    email -- Authentication email for user on Midas server.
    apikey -- A valid api-key assigned to the user.
    sourceVersion -- The source version with a valid directory name.
    destVersion -- The destination version.
    data_id -- A valid id for the Data folder (NA-MIC/Public/Slicer/Data).
    ignore_modules -- Ignore a module while versioning. To ignore multiple modules, use this option multiple times (e.g.: -g A -g B).
    overwrite -- Overwrite items if existing. If this flag is provided, duplicates by overwriting existing items. If this flag is not provided, does not duplicate existing items.
    dry_run -- List modules and exit. If this flag is provided, a list of modules will be printed and nothing else will be done."""
  # Instantiate a communicator and login to get an authentication token
  communicator = pydas.core.Communicator(midas_url)
  token = communicator.login_with_api_key(email, apikey)

  # Get the sub-folders for the Data folder
  # Currently only versions the Application and Modules folders
  data_folders = communicator.folder_children(token, data_id)

  ModulesIndex = _getFolderIndex(data_folders, "Modules")
  if ModulesIndex == -1:
    _error("No folder named Modules in Data folder")
  ModulesID = _getIDfromIndex(data_folders, "folder", ModulesIndex)

  ApplicationIndex = _getFolderIndex(data_folders, "Application")
  if ApplicationIndex == -1:
    _error("No folder named Application in Data folder")
  ApplicationID = _getIDfromIndex(data_folders, "folder", ApplicationIndex)

  # If -l or --dry_run provided, just print the structure and exit
  if dry_run:
    printSourceStructure(ModulesID, ApplicationID, sourceVersion, token, communicator)
    sys.exit(0)

  msgData = "Versioning of the NA-MIC Data tree for release %s..." % (destVersion)
  print(msgData)
  msgModules = "Versioning Modules..."
  print(msgModules)
  versionDataModulesDirectory(sourceVersion, destVersion, token, communicator, ModulesID, ignore_modules, overwrite)
  print(msgModules + "[DONE]")

  msgApplication = "Versioning Application..."
  print(msgApplication)
  versionDataApplicationDirectory(sourceVersion, destVersion, token, communicator, ApplicationID, overwrite)
  print(msgApplication + "[DONE]")

  print(msgData + "[DONE]")

def _checkRequiredArguments(options, parser):
  """Check the input arguments to see if all REQUIRED arguments are provided by user
    options -- Dictionary of options supplied by the user.
    parser -- OptionParser."""
  missing_options = []
  for option in parser.option_list:
    if re.match(r'^\[REQUIRED\]', option.help) and eval('options.' + option.dest) is None:
      missing_options.extend(option._long_opts)
  if len(missing_options) > 0:
    _error('Missing REQUIRED parameters: ' + str(missing_options))

def _main():
  """Main function for command-line interface.
    Defines usage options
    All options with [REQUIRED] in the help string do not have default values
    and the user is "required" to provide them"""
  usage = "Usage: %prog [options]"
  parser = OptionParser(usage=usage)
  parser.add_option("-u", "--url", dest="midas_url", metavar="url", help="Midas URL", default="http://localhost/midas")
  parser.add_option("-e", "--email", dest="email", metavar="email", help="[REQUIRED] Authentication email for user on Midas server")
  parser.add_option("-k", "--apikey", dest="apikey", metavar="apikey", help="[REQUIRED] A valid api-key assigned to the user")
  parser.add_option("-s", "--source_version", dest="sourceVersion", metavar="source_version", help="[REQUIRED] The source version with a valid directory name")
  parser.add_option("-d", "--dest_version", dest="destVersion", metavar="dest_version", help="[REQUIRED] The destination version. This script creates a directory")
  parser.add_option("-i", "--data_id", dest="data_id", metavar="id", help="A valid id for the Data folder (NA-MIC/Public/Slicer/Data)", type=int, default=9)
  parser.add_option("-g", "--ignore-module", dest="ignore_modules", metavar="module", action="append", help="Ignore a module while versioning. To ignore multiple modules, use this option multiple times (e.g.: -g A -g B)", default=[])
  parser.add_option("-o", "--overwrite", dest="overwrite", action="store_true", help="Overwrite items if existing. If this flag is provided, duplicates by overwriting existing items. If this flag is not provided, does not duplicate existing items.", default=False)
  parser.add_option("-l", "--dry-run", dest="dry_run", action="store_true", help="Print structure of source version directory and exit. If this flag is provided, a list of folders/items that will be copied by the script will be printed and nothing else will be done.", default=False)

  # Parse input arguments
  (options, args) = parser.parse_args()
  _checkRequiredArguments(options,parser)
  versionData(options.midas_url, options.email, options.apikey, options.sourceVersion, options.destVersion, options.data_id, options.ignore_modules, options.overwrite, options.dry_run)

if __name__=="__main__":
  _main()
