cmake_minimum_required (VERSION 3.0)

set (name example)
project (${name})

if (WIN32)
  add_definitions (-D_CRT_SECURE_NO_WARNINGS)
else (WIN32)
  add_definitions (-std=c++14)
endif (WIN32)

# get all *.cpp in current folder
aux_source_directory(. src_file)

# make a project for each examples
foreach (src ${src_file})
  message ("file ${src} found")

  # project name
  string (REGEX REPLACE ".*/(.*)/*.cpp$" "\\1" pro_name ${src})

  # replace "_" to "-"
  string (REPLACE "_" "-" pro_name ${pro_name})
  message ("project ${pro_name} added")

  # src files(*.cpp files)
  set (file ${src})

  add_executable(${pro_name} ${file})
endforeach (src)
