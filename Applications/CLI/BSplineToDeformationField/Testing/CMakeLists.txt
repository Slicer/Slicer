get_filename_component(full_path_without_filename ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component(full_path_without_testing ${full_path_without_filename} PATH)
get_filename_component(module_name ${full_path_without_testing} NAME)
message(WARNING "warning: Module ${module_name} doesn't have any test !")

