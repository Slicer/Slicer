# --------------------------------------------------------------------------
# Testing

add_test(Slicer3EditorModelMakerTest ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Editor/Testing/TestEditorModelMaker.tcl ${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData/helixMask.nrrd)

add_test(Slicer3EditorPaintTest ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Editor/Testing/TestEditorPaint.tcl ${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData/fixed.nrrd)

add_test(Slicer3EditorStructureTest ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Editor/Testing/TestEditorStructures.tcl ${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData/fixed.nrrd)
