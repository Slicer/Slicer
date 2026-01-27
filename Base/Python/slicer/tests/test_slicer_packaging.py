import sys
import unittest.mock
from pathlib import Path

import slicer.util
from slicer.packaging import Requirements
from slicer.packaging.installer import FileIdentifier


# test_packages = Path(__file__).parent.joinpath("test_packages")
#
# dummy_1_0_0 = test_packages.joinpath("dummy-1-0-0").resolve().absolute()
# dummy_1_1_0 = test_packages.joinpath("dummy-1-1-0").resolve().absolute()
# dummy_2_0_0 = test_packages.joinpath("dummy-2-0-0").resolve().absolute()


class PipLogger:
    def __init__(self):
        self.commands = []

    def __call__(self, *args, **kwargs):
        self.commands.append(dict(args=args, kwargs=kwargs))


class PatchedPipTest(unittest.TestCase):
    @unittest.mock.patch("slicer.packaging.installer.pip_install", new_callable=PipLogger)
    def test_simple_import(self, pip_install):
        with Requirements("placeholder:requirements.txt"):
            # This anchor is not real; we patched `pip_install` so it will never be resolved anyway.
            import dummy

        try:
            # The import resolution should occur here. We patched `pip_install`, so nothing will
            # actually be installed. Thus the import error should occur at this point, and
            # `pip_install` should be called once.
            dummy.magic
            raise AssertionError("Import should fail")
        except (ImportError, AttributeError, AssertionError):
            pass  # don't need contents for this test

        assert len(pip_install.commands) == 1
        assert pip_install.commands[0]['kwargs']['requirements'].identifier == 'placeholder:requirements.txt'

class RequirementsResolverTest(unittest.TestCase):
    def test_core_constraints(self):
        reqs = FileIdentifier(
            "test_core_constraints",
            "slicer.packaging:core-constraints.txt",
            sys.modules[__name__],
        )

        # should work without failing.
        with reqs.as_file() as f:
            assert f.name == "core-constraints.txt"

            with open(f) as fh:
                content = fh.read()

            assert "numpy" in content

    def test_as_file(self):
        reqs = FileIdentifier(
            "test_slicer_packaging (test_as_file)",
            "test_resources:requirements.txt",
            sys.modules[__name__],
        )

        # should work without failing.
        with reqs.as_file() as f:
            assert f.name == "requirements.txt"


# Todo test that pip_install correctly calls `uv pip install`

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
