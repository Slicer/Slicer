#!/usr/bin/env python
""" Unit Test for NA-MIC data tree versioning script.
    This test requires the user to enter the Midas server URL, the authentication email, the authentication API key and folder ID of an empty folder. The test first creates a mock data tree in the empty folder provided and runs tests off of that.
    Usage: python midasdata_test.py"""
from __future__ import print_function

import midasdata
import re
import sys
import time
import unittest

try:
  import pydas
except ImportError as e:
  print(e, "\nInstall pydas or update PYTHONPATH")

class TestVersioning(unittest.TestCase):

  @classmethod
  def setUpClass(self):
    """ setUpClass method """
    self.url = input("URL for Midas server: ")
    self.cur_email = input("Authentication email: ")
    self.cur_apikey = input("Authentication API key: ")
    self.data_id = input("Folder ID of empty folder: ")
    self.sourceVersion = "source"
    self.destVersion = "dest"
    self.ignore_modules = ""
    self.overwrite = True
    self.list_modules = False
    self.modulesList = ["Module1", "Module2", "Module3"]

  def setUp(self):
    """ setUp method """
    self.authenticateMidas()
    self.checkEmptyFolder()

  def authenticateMidas(self):
    """ Authentication on Midas server """
    self.communicator = pydas.core.Communicator(self.url)
    self.token = self.communicator.login_with_api_key(self.cur_email, self.cur_apikey)

  def checkEmptyFolder(self):
    """ Check if the data folder, entered by user, is empty """
    children = self.communicator.folder_children(self.token, self.data_id)
    if (len(children["folders"]) != 0) or (len(children["items"] != 0)):
      midasdata._error("Folder with id: %s is not empty" %self.data_id)

  def createFolderStructure(self):
    """ Creates a mock NA-MIC data tree for testing on the server """
    msg = "Creating testing data tree..."
    print(msg)
    self.ApplicationFolder = self.communicator.create_folder(self.token, "Application", self.data_id)
    self.ModulesFolder = self.communicator.create_folder(self.token, "Modules", self.data_id)
    srcApplication = self.communicator.create_folder(self.token, self.sourceVersion, self.ApplicationFolder["folder_id"])
    srcItem = self.communicator.create_item(self.token, "item_Application", srcApplication["folder_id"])
    for module in self.modulesList:
      srcModules = self.communicator.create_folder(self.token, module, self.ModulesFolder["folder_id"])
      sourceModule = self.communicator.create_folder(self.token, self.sourceVersion, srcModules["folder_id"])
      itemName = "item_" + module
      sourceItem = self.communicator.create_item(self.token, itemName, sourceModule["folder_id"])
    print(msg + "[DONE]")

  def cleanUpFolder(self):
    """ Cleaning the mock NA-MIC data tree created for testing """
    msg = "Cleaning up testing data tree..."
    print(msg)
    self.communicator.delete_folder(self.token, self.ApplicationFolder["folder_id"])
    self.communicator.delete_folder(self.token, self.ModulesFolder["folder_id"])
    print(msg + "[DONE]")

  def test_dryRun(self):
    """ Test for the dry run of the script """
    msg = "Testing dry run..."
    print("\n", msg)
    self.createFolderStructure()
    sys.stdout = open("logfile","w")
    midasdata.printSourceStructure(self.ModulesFolder["folder_id"], self.ApplicationFolder["folder_id"], self.sourceVersion, self.token, self.communicator)
    sys.stdout = sys.__stdout__
    f = open("logfile")
    lines = f.readlines()
    self.assertEqual(len(lines), 16)
    expr = "Application"
    regexp = re.compile(expr)
    self.assertRegexpMatches(lines[0], regexp)
    expr = "'-'-item_Module3"
    regexp = re.compile(expr)
    self.assertRegexpMatches(lines[14], regexp)
    self.cleanUpFolder()
    print(msg + "[DONE]")

  def test_versionAllData(self):
    """ Test for versioning mock data tree """
    msg = "Testing versionData..."
    print("\n", msg)
    self.createFolderStructure()
    midasdata.versionData(self.url, self.cur_email, self.cur_apikey, self.sourceVersion, self.destVersion, self.data_id)
    applicationChildren = self.communicator.folder_children(self.token, self.ApplicationFolder["folder_id"])
    index = midasdata._getFolderIndex(applicationChildren, self.destVersion)
    self.assertTrue(index > -1)
    destID = midasdata._getIDfromIndex(applicationChildren, "folder", index)
    self.assertTrue(midasdata.itemExists(destID, "item_Application", self.token, self.communicator))
    self.cleanUpFolder()
    print(msg + "[DONE]")

  def test_versionAllData_ignoreModules(self):
    """ Test for ignoring modules while versioning """
    msg = "Testing versionData ignoring modules..."
    print("\n", msg)
    self.createFolderStructure()
    midasdata.versionData(self.url, self.cur_email, self.cur_apikey, self.sourceVersion, self.destVersion, self.data_id, ["Module2"], False, False)
    modulesChildren = self.communicator.folder_children(self.token, self.ModulesFolder["folder_id"])
    index = midasdata._getFolderIndex(modulesChildren, "Module2")
    self.assertTrue(index > -1)
    module2ID = midasdata._getIDfromIndex(modulesChildren, "folder", index)
    module2Children = self.communicator.folder_children(self.token, module2ID)
    self.assertTrue(len(module2Children["folders"]) == 1)
    self.cleanUpFolder()
    print(msg + "[DONE]")

  def test_versionAllData_overwrite(self):
    """ Test for overwriting items while versioning """
    msg = "Testing versionData with overwrite..."
    print("\n", msg)
    self.createFolderStructure()
    midasdata.versionData(self.url, self.cur_email, self.cur_apikey, self.sourceVersion, self.destVersion, self.data_id)
    midasdata.versionData(self.url, self.cur_email, self.cur_apikey, self.sourceVersion, self.destVersion, self.data_id, [], True, False)
    applicationChildren = self.communicator.folder_children(self.token, self.ApplicationFolder["folder_id"])
    index = midasdata._getFolderIndex(applicationChildren, self.destVersion)
    self.assertTrue(index > -1)
    destID = midasdata._getIDfromIndex(applicationChildren, "folder", index)
    dest_items = self.communicator.folder_children(self.token, destID)
    self.assertEqual(len(dest_items["items"]), 1)
    self.cleanUpFolder()
    print(msg + "[DONE]")

if __name__ == "__main__":
  suite = unittest.TestLoader().loadTestsFromTestCase(TestVersioning)
  unittest.TextTestRunner(verbosity=2).run(suite)
