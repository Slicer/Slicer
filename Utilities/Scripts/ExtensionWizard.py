#!/usr/bin/env python

import sys
sys.path.append("./SlicerWizard");

from SlicerWizard import ExtensionWizard

if __name__ == "__main__":
  w = ExtensionWizard()
  w.execute()
