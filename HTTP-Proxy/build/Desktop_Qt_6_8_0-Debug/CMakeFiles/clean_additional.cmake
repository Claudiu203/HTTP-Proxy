# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/HTTP-Proxy_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/HTTP-Proxy_autogen.dir/ParseCache.txt"
  "HTTP-Proxy_autogen"
  )
endif()
