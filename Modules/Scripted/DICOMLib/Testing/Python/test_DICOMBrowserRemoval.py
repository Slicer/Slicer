import importlib.util
import pathlib
import sys
import types
import unittest
from unittest.mock import patch


class Signal:
    def __init__(self):
        self.callbacks = []

    def connect(self, callback):
        self.callbacks.append(callback)

    def emit(self, *args):
        for callback in self.callbacks:
            callback(*args)


class Widget:
    def __init__(self, parent=None):
        pass


class Database:
    def __init__(self):
        self.files = {}
        self.connections = {}

    def connect(self, signal, callback):
        self.connections[signal] = callback

    def filesForSeries(self, uid):
        return self.files.get(uid, [])

    def removeSeries(self, uid):
        self.files.pop(uid)
        self.connections["seriesRemoved(QString)"](uid)


class TableManager:
    def __init__(self):
        self.selected = []

    def connect(self, signal, callback):
        pass

    def currentSeriesSelection(self):
        return self.selected


class CTKBrowser:
    def __init__(self, database):
        self._database = database
        self.tableManager = TableManager()
        self.databaseDirectory = ""

    def connect(self, signal, callback):
        pass

    def database(self):
        return self._database

    def dicomTableManager(self):
        return self.tableManager


class VisualBrowser:
    ThumbnailSizePresetOption = types.SimpleNamespace(Large=1, Medium=2, Small=3, Hidden=4)

    def __init__(self):
        self.seriesRetrieved = Signal()

    def setDatabaseDirectory(self, directory):
        pass

    def connect(self, signal, callback):
        pass

    def findChild(self, widgetType, name):
        return None


class LoadableTable:
    def __init__(self):
        self.checked = 0
        self.loadables = None
        self.updateSelectedCalls = 0

    def setLoadables(self, loadables):
        self.loadables = loadables
        self.checked = 0

    def getNumberOfCheckedItems(self):
        return self.checked

    def updateSelectedFromCheckstate(self):
        self.updateSelectedCalls += 1


class DICOMBrowserRemovalTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.database = Database()
        qt = types.ModuleType("qt")
        qt.QWidget = Widget
        qt.QMessageBox = Widget
        qt.QTableWidget = Widget
        qt.QPushButton = Widget
        qt.QSettings = lambda: object()
        qt.Signal = Signal

        ctk = types.ModuleType("ctk")
        ctk.ctkDICOMVisualBrowserWidget = VisualBrowser

        slicer = types.ModuleType("slicer")
        slicer.dicomDatabase = cls.database
        util = types.ModuleType("slicer.util")
        util.VTKObservationMixin = type("VTKObservationMixin", (), {})
        util.settingsValue = lambda key, default, converter=None: default
        util.toBool = bool
        util.mainWindow = lambda: None
        slicer.util = util
        i18n = types.ModuleType("slicer.i18n")
        i18n.tr = lambda text: text

        dicomLib = types.ModuleType("DICOMLib")
        dicomLib.DICOMUtils = types.SimpleNamespace()
        dicomLib.selectHighestConfidenceLoadables = lambda loadables: None

        def failIfExamined(*args):
            raise AssertionError("Removed files were examined")

        dicomLib.getLoadablesFromFileLists = failIfExamined
        modulePath = pathlib.Path(__file__).resolve().parents[2] / "DICOMBrowser.py"
        spec = importlib.util.spec_from_file_location("DICOMBrowserUnderTest", modulePath)
        module = importlib.util.module_from_spec(spec)
        with patch.dict(sys.modules, {
            "qt": qt,
            "ctk": ctk,
            "slicer": slicer,
            "slicer.util": util,
            "slicer.i18n": i18n,
            "DICOMLib": dicomLib,
        }):
            spec.loader.exec_module(module)
        cls.browserClass = module.SlicerDICOMBrowser

    def setUp(self):
        self.database.files.clear()
        self.database.connections.clear()
        browserClass = self.browserClass

        class BrowserUnderTest(browserClass):
            def setup(self):
                self.loadableTable = LoadableTable()
                self.examineButton = types.SimpleNamespace(enabled=False)
                self.loadButton = types.SimpleNamespace(enabled=False)
                self.uncheckAllButton = types.SimpleNamespace(enabled=False)
                self.advancedViewButton = types.SimpleNamespace(
                    checkState=lambda: 2 if self.advancedView else 0)

        self.ctkBrowser = CTKBrowser(self.database)
        self.browser = BrowserUnderTest(self.ctkBrowser, parent=None)

    def test_removed_selection_disables_basic_load(self):
        self.database.files["removed"] = ["removed.dcm"]
        self.ctkBrowser.tableManager.selected = ["removed"]
        self.browser.onSeriesSelected(["removed"])
        self.assertTrue(self.browser.loadButton.enabled)

        self.database.removeSeries("removed")

        self.assertEqual(self.browser.fileLists, [])
        self.assertFalse(self.browser.loadButton.enabled)
        self.assertEqual(self.browser.loadableTable.loadables, [])
        self.assertFalse(self.browser.examineForLoading())
        self.assertEqual(self.browser.loadableTable.loadables, [])

    def test_removal_clears_advanced_loadables_and_preserves_remaining_series(self):
        self.browser.advancedView = True
        self.database.files.update({"removed": ["removed.dcm"], "kept": ["kept.dcm"]})
        self.ctkBrowser.tableManager.selected = ["removed", "kept"]
        self.browser.onSeriesSelected(self.ctkBrowser.tableManager.selected)
        self.browser.loadableTable.checked = 1
        self.browser.loadablesByPlugin = {"old": ["stale loadable"]}
        self.browser.updateButtonStates()
        self.assertTrue(self.browser.examineButton.enabled)
        self.assertTrue(self.browser.loadButton.enabled)

        self.database.removeSeries("removed")

        self.assertEqual(self.browser.fileLists, [["kept.dcm"]])
        self.assertEqual(self.browser.loadableTable.loadables, [])
        self.assertEqual(self.browser.loadablesByPlugin, {})
        self.assertTrue(self.browser.examineButton.enabled)
        self.assertFalse(self.browser.loadButton.enabled)

        self.database.removeSeries("kept")

        self.assertEqual(self.browser.fileLists, [])
        self.assertFalse(self.browser.examineButton.enabled)
        self.assertFalse(self.browser.loadButton.enabled)

    def test_examination_without_removal_publishes_results(self):
        self.browser.advancedView = True
        self.database.files["kept"] = ["kept.dcm"]
        self.ctkBrowser.tableManager.selected = ["kept"]
        self.browser.onSeriesSelected(["kept"])
        loadables = {"plugin": ["loadable"]}
        self.browser.getLoadablesFromFileLists = lambda fileLists: (loadables, True)

        self.assertTrue(self.browser.examineForLoading())
        self.assertEqual(self.browser.loadablesByPlugin, loadables)
        self.assertEqual(self.browser.loadableTable.loadables, loadables)
        self.assertTrue(self.browser.examineButton.enabled)

    def test_removal_during_advanced_examination_discards_results(self):
        self.browser.advancedView = True
        self.database.files["removed"] = ["removed.dcm"]
        self.ctkBrowser.tableManager.selected = ["removed"]
        self.browser.onSeriesSelected(["removed"])

        def examine(fileLists):
            self.assertEqual(fileLists, [["removed.dcm"]])
            self.database.removeSeries("removed")
            return {"plugin": ["stale loadable"]}, True

        self.browser.getLoadablesFromFileLists = examine

        self.assertFalse(self.browser.examineForLoading())
        self.assertEqual(self.browser.fileLists, [])
        self.assertEqual(self.browser.loadablesByPlugin, {})
        self.assertEqual(self.browser.loadableTable.loadables, [])
        self.assertFalse(self.browser.examineButton.enabled)
        self.assertFalse(self.browser.loadButton.enabled)

    def test_removal_during_basic_examination_aborts_load(self):
        self.database.files["removed"] = ["removed.dcm"]
        self.ctkBrowser.tableManager.selected = ["removed"]
        self.browser.onSeriesSelected(["removed"])

        def examine(fileLists):
            self.assertEqual(fileLists, [["removed.dcm"]])
            self.database.removeSeries("removed")
            return {"plugin": ["stale loadable"]}, True

        self.browser.getLoadablesFromFileLists = examine
        self.browser.loadCheckedLoadables()

        self.assertEqual(self.browser.fileLists, [])
        self.assertEqual(self.browser.loadablesByPlugin, {})
        self.assertEqual(self.browser.loadableTable.loadables, [])
        self.assertEqual(self.browser.loadableTable.updateSelectedCalls, 0)
        self.assertFalse(self.browser.loadButton.enabled)


if __name__ == "__main__":
    unittest.main()
