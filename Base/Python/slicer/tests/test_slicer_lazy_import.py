from pathlib import Path

lazy_import_packages = Path(__file__).parent.joinpath('lazy-import-packages')

dummy_1_0_0 = lazy_import_packages.joinpath('dummy-1-0-0')
dummy_1_1_0 = lazy_import_packages.joinpath('dummy-1-1-0')
dummy_2_0_0 = lazy_import_packages.joinpath('dummy-2-0-0')
