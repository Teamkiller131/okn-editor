# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\okn-editor-app_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\okn-editor-app_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\okn-editor_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\okn-editor_autogen.dir\\ParseCache.txt"
  "okn-editor-app_autogen"
  "okn-editor_autogen"
  )
endif()
