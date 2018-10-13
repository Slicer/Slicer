#!/usr/bin/env bash

# See https://github.com/codespell-project/codespell#readme

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")

cd $SCRIPT_DIR/../../

if ! command -v codespell &> /dev/null; then
  echo >&2 'error: "codespell" command not found in the PATH !'
  exit 1
fi

codespell -q6 --skip="\
.git,\
*.crt,\
*.svg,\
*.vtp,\
*DICOM-Master.json,\
*SlicerGeneralAnatomy.json,\
./CMakeLists.txt.user,\
./CMake/CTestCustom.cmake.in,\
./License.txt,\
./Resources/*.h,\
./Base/Logic/vtkSlicerApplicationLogicRequests.h,\
./Base/QTCLI/vtkSlicerCLIModuleLogic.cxx,\
./Base/QTCore/Resources/Certs/README,\
./Libs/MRML/Core/vtkMRMLCoreTestingUtilities.cxx,\
./Libs/vtkITK/vtkITKGrowCutSegmentationImageFilter.cxx,\
./Libs/vtkITK/itkConstrainedValueMultiplicationImageFilter.h,\
./Libs/vtkITK/itkLevelTracingImageFilter.h,\
./Modules/CLI/ExtractSkeleton/tilg_iso_3D.cxx,\
./Modules/CLI/ResampleDTIVolume/itkDiffusionTensor3D*,\
./Modules/Loadable/Reformat/Resources/UI/qSlicerReformatModuleWidget.ui\
" \
-L "\
supercede,\
thru,\
serie,\
loosing,\
unselect,\
toolsbox,\
dur,\
als,\
sinc,\
therefor,\
extracter,\
inactivate\
"
