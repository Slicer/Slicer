################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

cmake_minimum_required(VERSION 3.13.4)

function(list_to_string separator input_list output_string_var)
  set(_string "")
  # Get list length
  list(LENGTH input_list list_length)
  # If the list has 0 or 1 element, there is no need to loop over.
  if(list_length LESS 2)
    set(_string  "${input_list}")
  else()
    math(EXPR last_element_index "${list_length} - 1")
    foreach(index RANGE ${last_element_index})
      # Get current item_value
      list(GET input_list ${index} item_value)
      if(NOT item_value STREQUAL "")
        # .. and append non-empty value to output string
        set(_string  "${_string}${item_value}")
        # Append separator if current element is NOT the last one.
        if(NOT index EQUAL last_element_index)
          set(_string  "${_string}${separator}")
        endif()
      endif()
    endforeach()
  endif()
  set(${output_string_var} ${_string} PARENT_SCOPE)
endfunction()


#
# cmake -DTEST_list_to_string_test:BOOL=ON -P ListToString.cmake
#
function(list_to_string_test)

  function(list_to_string_test_check id current_output expected_output)
    if(NOT "${current_output}" STREQUAL "${expected_output}")
      message(FATAL_ERROR "Problem with list_to_string() - See testcase: ${id}\n"
                          "current_output:${current_output}\n"
                          "expected_output:${expected_output}")
    endif()
  endfunction()

  set(id 1)
  set(case${id}_input "")
  set(case${id}_expected_output "")
  list_to_string("^^" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 2)
  set(case${id}_input item1)
  set(case${id}_expected_output "item1")
  list_to_string("^^" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 3)
  set(case${id}_input item1 item2)
  set(case${id}_expected_output "item1^^item2")
  list_to_string("^^" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 4)
  set(case${id}_input item1 item2 item3)
  set(case${id}_expected_output "item1^^item2^^item3")
  list_to_string("^^" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 5)
  set(case${id}_input item1 item2 item3 item4)
  set(case${id}_expected_output "item1^^item2^^item3^^item4")
  list_to_string("^^" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 6)
  set(case${id}_input item1 "" item3 item4)
  set(case${id}_expected_output "item1^^item3^^item4")
  list_to_string("^^" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 7)
  set(case${id}_input item1 ^^item2 item3 item4)
  set(case${id}_expected_output "item1^^^^item2^^item3^^item4")
  list_to_string("^^" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 8)
  set(case${id}_input item1 item2 item3 item4)
  set(case${id}_expected_output "item1item2item3item4")
  list_to_string("" "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  set(id 9)
  set(case${id}_input item1 item2 item3 item4)
  set(case${id}_expected_output "item1 item2 item3 item4")
  list_to_string(" " "${case${id}_input}" case${id}_current_output)
  list_to_string_test_check(${id} "${case${id}_current_output}" "${case${id}_expected_output}")

  message("SUCCESS")
endfunction()
if(TEST_list_to_string_test)
  list_to_string_test()
endif()
