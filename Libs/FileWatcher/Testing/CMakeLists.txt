set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary/")

add_executable(FileWatcherDemo FileWatcherDemo.cxx)
target_link_libraries(FileWatcherDemo FileWatcher)

add_executable(FileWatcherTest FileWatcherTest.cxx)
target_link_libraries(FileWatcherTest FileWatcher itksys)

add_test(FileWatcherTest1 ${Slicer_BINARY_DIR}/bin/FileWatcherTest/ ${TEMP})
