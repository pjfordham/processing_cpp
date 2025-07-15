cmake_minimum_required(VERSION 3.26)

# set the project name
project(libProcessing VERSION 1.0)

include(FetchContent)
include(CTest)

enable_testing()

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.9
)
FetchContent_MakeAvailable(glfw)

set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        10.2.0
)
FetchContent_MakeAvailable(fmt)

set(FT_WITH_ZLIB OFF CACHE BOOL "" FORCE)
set(FT_WITH_BZIP2 OFF CACHE BOOL "" FORCE)
set(FT_WITH_PNG OFF CACHE BOOL "" FORCE)
set(FT_WITH_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://github.com/freetype/freetype.git
  GIT_TAG        VER-2-13-2
)
FetchContent_MakeAvailable(freetype)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        release-1-0-2
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
  dejavu_fonts
  GIT_REPOSITORY https://github.com/senotrusov/dejavu-fonts-ttf.git
  GIT_TAG        v2.37.3
)
FetchContent_MakeAvailable(dejavu_fonts)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
set(CURL_STATICLIB ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  curl
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG        curl-8_8_0
)
FetchContent_MakeAvailable(curl)

set(LIBXML2_WITH_LZMA OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_ICONV OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_DEBUG OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_ZLIB OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_PROGRAMS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  libxml2
  GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2.git
  GIT_TAG v2.11.4
)
FetchContent_MakeAvailable(libxml2)

FetchContent_Declare(
  libtess2
  GIT_REPOSITORY https://github.com/pjfordham/libtess2.git
  GIT_TAG master
)
FetchContent_MakeAvailable(libtess2)


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
  src/processing_stb_image_impl.cc
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
  external/glad/include
  ${stb_SOURCE_DIR}
  )

# Link the libraries
target_link_libraries(Processing
    PUBLIC
    nlohmann_json::nlohmann_json
    tess2
    glfw
    freetype
    glm::glm
    CURL::libcurl
    LibXml2::LibXml2
    fmt::fmt
    )

# Define the array of filenames
set(skip_examples
  "examples/Demos/Graphics/DepthSort/DepthSort.cc"                                     # No depth sort
  "examples/Demos/Graphics/LowLevelGLVboInterleaved/LowLevelGLVboInterleaved.cc"       # Won't support
  "examples/Demos/Graphics/LowLevelGLVboSeparate/LowLevelGLVboSeparate.cc"             # Won't support
  "examples/Demos/Tests/MultipleWindows/Arcball.cc"                                    # No multiple windows
  "examples/Demos/Tests/MultipleWindows/MultipleWindows.cc"                            # No multiple windows
  "examples/Demos/Tests/ResizeTest/ResizeTest.cc"                                      # No resize
  "examples/Demos/Tests/SpecsTest/SpecsTest.cc"                                        # Won't support
  "examples/Topics/Shaders/SepBlur/SepBlur.cc"                                         # Not sure
 )

function(target_force_include target file)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/FI${file}>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-include${file}>
    )
endfunction()

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
  else()
    add_executable(${base_name} sketch.cc)
    target_compile_options(${base_name} PRIVATE -DFILE_TO_INCLUDE=\"${file_path}\")
  endif()

  target_link_libraries(${base_name} PRIVATE Processing)
  target_force_include(${base_name} "${CMAKE_CURRENT_SOURCE_DIR}/include/processing.h")

  add_custom_command(TARGET ${base_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/data)

  add_custom_command(TARGET ${base_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${dejavu_fonts_SOURCE_DIR}/ttf/DejaVuSans.ttf
    ${CMAKE_BINARY_DIR}/data)

  file(GLOB DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${dir_path}/data/*")

  # If there are data files copy them to the target
  foreach(f ${DATA_FILES})
    add_custom_command(TARGET ${base_name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${f}" ${CMAKE_BINARY_DIR}/data)
  endforeach()

  add_test(NAME ${base_name} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/pro_test.sh ./${base_name} ${CMAKE_CURRENT_SOURCE_DIR}/refs)

endfunction()

# Add all the examples targets
file(GLOB_RECURSE files_to_process CONFIGURE_DEPENDS "./examples/*.cc")
foreach(file_path ${files_to_process})
  file(RELATIVE_PATH file_relative "${CMAKE_CURRENT_SOURCE_DIR}" "${file_path}")
  add_example( ${file_relative} )
endforeach()
