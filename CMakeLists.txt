cmake_minimum_required(VERSION 3.26)

# set the project name
project(libProcessing VERSION 1.0)

include(CTest)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(external/freetype)
add_subdirectory(external/libglfw)
add_subdirectory(external/fmtlib)
add_subdirectory(external/libtess2)
add_subdirectory(external/glm)
add_subdirectory(external/json)

add_library(Processing
  src/processing.cc
  src/processing_math.cc
  src/processing_utils.cc
  src/processing_pfont.cc
  src/processing_pimage.cc
  src/processing_pmaterial.cc
  src/processing_pshape_svg.cc
  src/processing_pshape.cc
  src/processing_color.cc
  src/processing_opengl.cc
  src/processing_opengl_shader.cc
  src/processing_opengl_framebuffer.cc
  src/processing_opengl_texture.cc
  src/processing_pgraphics.cc
  src/processing_pshader.cc
  src/processing_xml.cc
  src/processing_json.cc
  external/glad/src/glad.c
)

install(TARGETS Processing DESTINATION lib)

install(FILES
  include/processing.h
  include/processing_properties.h
  include/processing_math.h
  include/processing_utils.h
  include/processing_debug.h
  include/processing_pfont.h
  include/processing_pimage.h
  include/processing_pmaterial.h
  include/processing_pshape.h
  include/processing_pshape_svg.h
  include/processing_color.h
  include/processing_opengl.h
  include/processing_opengl_shader.h
  include/processing_opengl_framebuffer.h
  include/processing_opengl_texture.h
  include/processing_pgraphics.h
  include/processing_profile.h
  include/processing_pshader.h
  include/processing_pshader_builder.h
  include/processing_java_compatability.h
  include/processing_time.h
  include/processing_enum.h
  include/processing_xml.h
  include/processing_json.h
  DESTINATION include)

target_include_directories(Processing
  PUBLIC
  include
  external/stb
  external/glad/include
  external/freetype/include
  external/libglfw/include
  external/libtess2/Include
  external/fmtlib/include
  external/glm/include
  external/json/include
  ${CURL_INCLUDE_DIRS}
  ${LIBXML2_INCLUDE_DIR}
  )

# Link the libraries
target_link_libraries(Processing
    PUBLIC
    tess2
    glfw
    freetype
    glm
    ${CURL_LIBRARIES}
    LibXml2::LibXml2
    fmt
    )

# Define the array of filenames
set(skip_examples
  "examples/Basics/Shape/GetChild/GetChild.cc"                                         # No disable style
  "examples/Demos/Graphics/DepthSort/DepthSort.cc"                                     # No depth sort
  "examples/Demos/Graphics/LowLevelGLVboInterleaved/LowLevelGLVboInterleaved.cc"       # Won't support
  "examples/Demos/Graphics/LowLevelGLVboSeparate/LowLevelGLVboSeparate.cc"             # Won't support
  "examples/Demos/Tests/MultipleWindows/Arcball.cc"                                    # No multiple windows
  "examples/Demos/Tests/MultipleWindows/MultipleWindows.cc"                            # No multiple windows
  "examples/Demos/Tests/ResizeTest/ResizeTest.cc"                                      # No resize
  "examples/Demos/Tests/SpecsTest/SpecsTest.cc"                                        # Won't support
  "examples/Topics/Shaders/GlossyFishEye/GlossyFishEye.cc"                             # Not sure
  "examples/Topics/Shaders/SepBlur/SepBlur.cc"                                         # Not sure
 )

function(ADD_EXAMPLE file_path)

  # Check if the filename is present in the array
  list(FIND skip_examples ${file_path} index)
  if (NOT index EQUAL -1 )
    message("Skipping '${file_path}' becuase it doesn't work yet.")
    return()
  endif()

  get_filename_component(dir_path ${file_path} DIRECTORY)
  get_filename_component(base_name ${file_path} NAME_WE)
  get_filename_component(extension ${file_path} EXT)

  while (TARGET ${base_name})
    string(MD5 hash ${file_path})
    string(SUBSTRING ${hash} 0 4 suffix)
    message("The target ${base_name} already exists adding hash suffix ${suffix}.")
    set(base_name "${base_name}_${suffix}")
  endwhile()

  # Define the regular expression pattern to search for
  set(regex_pattern "void.*setup.*[(].*[)]")
  # Read the file and search using the regular expression pattern
  file(STRINGS ${file_path} file_content REGEX ${regex_pattern})

  if(file_content)
    add_executable(${base_name} ${file_path})
    target_link_libraries(${base_name}  PRIVATE Processing)
    target_compile_options(${base_name} PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/include/processing.h)
  else()
    add_executable(${base_name} sketch.cc)
    target_link_libraries(${base_name}  PRIVATE Processing)
    target_compile_options(${base_name} PRIVATE -DFILE_TO_INCLUDE=\"${file_path}\" -include ${CMAKE_CURRENT_SOURCE_DIR}/include/processing.h)
  endif()

  # If there's a data directory copy it to the target
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir_path}/data)
    add_custom_command(TARGET ${base_name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${base_name}>/data)

    add_custom_command(TARGET ${base_name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/${dir_path}/data/*
      $<TARGET_FILE_DIR:${base_name}>/data
      )
  endif()

  add_test(NAME ${base_name} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/pro_test.sh ./${base_name} ${CMAKE_CURRENT_SOURCE_DIR}/refs )

endfunction()

# Add all the examples targets
file(GLOB_RECURSE files_to_process CONFIGURE_DEPENDS "./examples/*.cc")
foreach(file_path ${files_to_process})
  file(RELATIVE_PATH file_relative "${CMAKE_CURRENT_SOURCE_DIR}" "${file_path}")
  add_example( ${file_relative} )
endforeach()
