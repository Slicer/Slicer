# --------------------------------------------------------------------------
# Testing

ADD_TEST(Slicer3EditorModelMakerTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Modules/Editor/Testing/TestEditorModelMaker.tcl ${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/TestData/helixMask.nrrd)
ADD_TEST(Slicer3EditorPainTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Modules/Editor/Testing/TestEditorPaint.tcl ${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/TestData/fixed.nrrd)
