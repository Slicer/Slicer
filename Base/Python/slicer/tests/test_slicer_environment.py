
import qt
import slicer
import unittest


class SlicerEnvironmentTests(unittest.TestCase):

  def setUp(self):
    pass

  def test_slicer_util_startupEnvironment(self):
    startupEnv = slicer.util.startupEnvironment()
    assert isinstance(startupEnv, dict)
    assert "PATH" not in startupEnv or "Slicer-build" not in startupEnv["PATH"]

  def test_slicer_app_startupEnvironment(self):
    startupEnv = slicer.app.startupEnvironment()
    assert isinstance(startupEnv, qt.QProcessEnvironment)
    assert "Slicer-build" not in startupEnv.value("PATH", "")

  def test_slicer_app_environment(self):
    env = slicer.app.environment()
    assert isinstance(env, qt.QProcessEnvironment)
    assert "Slicer-build" in env.value("PATH")
