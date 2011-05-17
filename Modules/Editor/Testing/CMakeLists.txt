# --------------------------------------------------------------------------
# Testing

ADD_TEST(Slicer3EditorModelMakerTest ${Slicer_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Editor/Testing/TestEditorModelMaker.tcl ${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData/helixMask.nrrd)

ADD_TEST(Slicer3EditorPaintTest ${Slicer_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Editor/Testing/TestEditorPaint.tcl ${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData/fixed.nrrd)

ADD_TEST(Slicer3EditorStructureTest ${Slicer_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Editor/Testing/TestEditorStructures.tcl ${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData/fixed.nrrd)
