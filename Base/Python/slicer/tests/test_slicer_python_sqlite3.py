import unittest
import slicer
import os
import shutil

class SlicerPythonSqlite3Tests(unittest.TestCase):
  """This test verifies that Python is build with sqlite3 enabled.
  """

  def setUp(self):
    self.tempDir = slicer.util.tempDirectory()

  def tearDown(self):
    shutil.rmtree(self.tempDir, True)

  def test_sqliteDatabase(self):
    import sqlite3

    database_filename = os.path.join(self.tempDir, 'database.sql')
    print("database_filename="+database_filename)

    self.connection = sqlite3.connect(database_filename)
    print(sqlite3.version)
    self.assertIsNotNone(self.connection)
    self.assertTrue(os.path.exists(database_filename))

    create_table_sql = """ CREATE TABLE IF NOT EXISTS projects (
                                        id integer PRIMARY KEY,
                                        name text NOT NULL,
                                        begin_date text,
                                        end_date text
                                    ); """
    c = self.connection.cursor()
    c.execute(create_table_sql)

    self.connection.close()
