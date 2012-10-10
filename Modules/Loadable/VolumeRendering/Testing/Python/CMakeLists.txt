if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)
  
  configure_file(
    slicervolumerenderingtestdatapaths.py.in
    ${Slicer_BINARY_DIR}/bin/Python/slicer/slicervolumerenderingtestdatapaths.py
    @ONLY
    )

  # It is expected for VTK GPU Ray cast to not work on Apple computers
  # Testing without VTK GPU Ray cast technique on Apple
  if(APPLE)
    slicer_add_python_test(
      SCRIPT VolumeRendering_CTAbdomen_Apple.py
      SLICER_ARGS --launcher-no-splash --qt-testing
      )
  else()
    slicer_add_python_test(
      SCRIPT VolumeRendering_CTAbdomen.py
      SLICER_ARGS --launcher-no-splash --qt-testing
      )
  endif()

endif()
