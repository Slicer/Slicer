#!/usr/bin/env python

import sys
sys.path.append("./SlicerWizard");

from SlicerWizard import SlicerWizard

if __name__ == "__main__":
  w = SlicerWizard()
  w.execute()
