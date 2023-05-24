cmake_minimum_required(VERSION 3.26)

# set the project name
project(libProcessing VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(Processing
  processing.h
  processing.cc

  PerlinNoise.cc
  PerlinNoise.h

  processing_math.h
  processing_math.cc

  processing_pfont.h
  processing_pfont.cc

  processing_pimage.h
  processing_pimage.cc

  processing_pshape.h
  processing_pshape.cc

  processing_color.h
  processing_color.cc

  processing_opengl.h
  processing_opengl.cc

  processing_pgraphics.h
  processing_pgraphics.cc

  processing_pshader.h
  processing_pshader.cc

  processing_java_compatability.h
  processing_texture_manager.h
  processing_time.h
  processing_enum.h )

install(TARGETS Processing DESTINATION lib)

install(FILES
  processing.h
  processing_math.h
  processing_pfont.h
  processing_pimage.h
  processing_pshape.h
  processing_color.h
  processing_opengl.h
  processing_pgraphics.h
  processing_pshader.h
  processing_java_compatability.h
  processing_texture_manager.h
  processing_time.h
  processing_enum.h
  DESTINATION include)

target_link_libraries(Processing  PUBLIC -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lfreetype -lGLEW -lGL -lcurl -lfmt)


function(ADD_EXAMPLE file_path)
  # Get directory path
  get_filename_component(dir_path ${file_path} DIRECTORY)

  # Get base name without extension
  get_filename_component(base_name ${file_path} NAME_WE)

  # Get file extension
  get_filename_component(extension ${file_path} EXT)

  # Print the results
  # message("Directory path: ${dir_path}")
  # message("Base name: ${base_name}")
  # message("Extension: ${extension}")

  if (TARGET ${base_name})
    string(RANDOM LENGTH 4 ALPHABET "abcdefghijklmnopqrstuvwxyz0123456789" RESULT_VARIABLE random_string)
    message("Random string: ${random_string}")

    # The target already exists
    message("The target ${base_name} already exists adding random suffix.")
    set(base_name "${base_name}_${random_string}")
  endif()

  add_executable(${base_name} ${file_path})
  target_link_libraries(${base_name}  PRIVATE Processing)
  target_compile_options(${base_name} PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/processing.h)

endfunction()

# Find files using the file(GLOB_RECURSE) command
file(GLOB_RECURSE files_to_process CONFIGURE_DEPENDS "./examples/*.cc")

# Iterate over the files
foreach(file_path ${files_to_process})
  add_example( ${file_path} )
endforeach()


#add_executable(TextureQuad TextureQuad.cc)
#target_link_libraries(TextureQuad PRIVATE Processing)
