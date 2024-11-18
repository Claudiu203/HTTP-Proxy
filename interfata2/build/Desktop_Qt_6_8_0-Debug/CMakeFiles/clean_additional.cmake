# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/interfata_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/interfata_autogen.dir/ParseCache.txt"
  "interfata_autogen"
  )
endif()
