import helpers

import argparse, sys, shutil, os, slicer

from SegmentStatistics import SegmentStatisticsLogic

def getMultivolumeFrame(mv, ref, frameNumber=0):
  mvi = mv.GetImageData()

  e0 = vtk.vtkImageExtractComponents()
  e0.SetInputData(mvi)
  e0.SetComponents(frameNumber)
  e0.Update()

  # clone reference
  frame = slicer.modules.volumes.logic().CloneVolume(slicer.mrmlScene, ref,'frame')
  frame.SetAndObserveImageData(e0.GetOutput())
  return frame

def main(argv):
  if True:
    parser = argparse.ArgumentParser(description="mpReview preprocessor")
    parser.add_argument("-m", "--multivolume", dest="multivolume", required=True, help="Input multivolume")
    parser.add_argument("-l", "--label", dest="label", required=True, help="Input label volume")
    parser.add_argument("-o", "--csv", dest="csv", required=True, help="Name of the output csv file")
    parser.add_argument("-f","--frame",dest="frame",default=0,type=int,required=False,help="Frame to extract statistics")

    args = parser.parse_args(argv)
    print args.frame

    (_,mv) = slicer.util.loadVolume(args.multivolume,returnNode=True)
    (_,label) = slicer.util.loadLabelVolume(args.label,returnNode=True)

    from LabelStatistics import LabelStatisticsLogic
    lsl = LabelStatisticsLogic(getMultivolumeFrame(mv,label,args.frame),label)

    csv = lsl.statsAsCSV()

    with open(args.csv,"w") as f:
      f.write(csv)

  sys.exit()

if __name__ == "__main__":
  print("Starting module")
  main(sys.argv[1:])
