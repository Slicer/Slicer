
#set(ENV{PATH} "${CMAKE_BINARY_DIR}/python-build/lib")
set(ENV{PATH} "@tcl_build@/bin")

execute_process(
  COMMAND sh configure --with-tcl=@tcl_build@/lib --with-tk=@tcl_build@/lib --prefix=@tcl_build@)
)
  
#message("LD_LIBRARY_PATH:=ENV{LD_LIBRARY_PATH}")
#set(ENV{PYTHONHOME} ${python_home})
