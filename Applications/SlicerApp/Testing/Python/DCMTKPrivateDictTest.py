import DICOMLib, sys, slicer, os

dcmfile = sys.argv[1]

dcmdump=DICOMLib.DICOMCommand('dcmdump',[dcmfile])
dump=str(dcmdump.start()).replace("\\r","\r").replace("\\n","\n").splitlines()

found_private_tag = False
for line in dump:
  line = line.split(' ')
  if line[0] == '(2001,1003)':
    if line[-1] == "DiffusionBFactor":
      found_private_tag = True
      break

if not found_private_tag:
  raise Exception("Could not find 'DiffusionBFactor' "
                  "private tag reading file '%s' using 'dcmdump' !" % dcmfile)
