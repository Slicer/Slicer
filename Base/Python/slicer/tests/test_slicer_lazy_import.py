from slicer import lazy
from pathlib import Path
import unittest
import unittest.mock

lazy_import_packages = Path(__file__).parent.joinpath("lazy_import_packages")

dummy_1_0_0 = lazy_import_packages.joinpath("dummy-1-0-0").resolve().absolute()
dummy_1_1_0 = lazy_import_packages.joinpath("dummy-1-1-0").resolve().absolute()
dummy_2_0_0 = lazy_import_packages.joinpath("dummy-2-0-0").resolve().absolute()


class PipLogger:
    def __init__(self):
        self.commands = []

    def __call__(self, *args):
        self.commands.append(args)


class PatchedPipTest(unittest.TestCase):
    @unittest.mock.patch("slicer.lazy._pip_install", new_callable=PipLogger)
    def test_simple_import(self, pip_install):
        with lazy.ImportGroup("/requires"):
            import dummy

            try:
                dummy.magic
                raise AssertionError("Import should fail")
            except (ImportError, AttributeError):
                pass  # don't need contents for this test

        assert len(pip_install.commands) == 1


# class LazyImportTest(unittest.TestCase):
#     def setUp(self):
#         pass
#         print("setting up....")
#
#     def tearDown(self):
#         try:
#             importlib.metadata.metadata("dummy")
#             sys.modules.pop("dummy", None)
#             lazy._pip_uninstall("dummy")
#         except importlib.metadata.PackageNotFoundError:
#             pass
#
#     def test_simple_lazy_import(self):
#         with tempfile.NamedTemporaryFile("w") as f:
#             f.write(str(dummy_1_0_0))
#             f.flush()
#
#             with lazy.ImportGroup(f.name):  # absolute path
#                 import dummy
#
#             assert dummy.magic == 8765309
#             assert importlib.metadata.version("dummy") == "1.0.0"
#
#     def test_version_constraint(self):
#         with tempfile.NamedTemporaryFile("w") as f:
#             f.write(str(dummy_1_1_0))
#             f.flush()
#
#             with lazy.ImportGroup(f.name):
#                 import dummy
#
#             assert dummy.magic == 1729
#             assert importlib.metadata.version("dummy") == "1.1.0"
