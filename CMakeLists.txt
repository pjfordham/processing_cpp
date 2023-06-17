cmake_minimum_required(VERSION 3.26)

# set the project name
project(libProcessing VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED COMPONENTS main image ttf)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt REQUIRED)

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

target_include_directories(Processing
  PUBLIC
  ${SDL2_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${FMT_INCLUDE_DIRS}
  )

# Link the libraries
target_link_libraries(Processing
    PUBLIC
    ${SDL2_LIBRARIES}
    -lSDL2_image
    -lSDL2_ttf
    ${FREETYPE_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CURL_LIBRARIES}
    fmt::fmt
    )

# Define the array of filenames
set(skip_examples
  "examples/Basics/Lights/Mixture/Mixture.cc"
  "examples/Basics/Lights/MixtureGrid/MixtureGrid.cc"
  "examples/Basics/Lights/Reflection/Reflection.cc"
  "examples/Basics/Lights/Spot/Spot.cc"
  "examples/Basics/Shape/DisableStyle/DisableStyle.cc"
  "examples/Basics/Shape/GetChild/GetChild.cc"
  "examples/Basics/Shape/LoadDisplayOBJ/LoadDisplayOBJ.cc"
  "examples/Basics/Shape/LoadDisplaySVG/LoadDisplaySVG.cc"
  "examples/Basics/Shape/ScaleShape/ScaleShape.cc"
  "examples/Basics/Shape/ShapeVertices/ShapeVertices.cc"
  "examples/Demos/Graphics/DepthSort/DepthSort.cc"
  "examples/Demos/Graphics/LowLevelGLVboInterleaved/LowLevelGLVboInterleaved.cc"
  "examples/Demos/Graphics/LowLevelGLVboSeparate/LowLevelGLVboSeparate.cc"
  "examples/Demos/Graphics/MeshTweening/MeshTweening.cc"
  "examples/Demos/Graphics/Particles/Particle.cc"
  "examples/Demos/Graphics/Particles/ParticleSystem.cc"
  "examples/Demos/Graphics/Particles/Particles.cc"
  "examples/Demos/Graphics/Planets/Perlin.cc"
  "examples/Demos/Graphics/Planets/Planets.cc"
  "examples/Demos/Graphics/Ribbons/Ribbons.cc"
  "examples/Demos/Graphics/Trefoil/Surface.cc"
  "examples/Demos/Graphics/Trefoil/Trefoil.cc"
  "examples/Demos/Graphics/Wiggling/Wiggling.cc"
  "examples/Demos/Graphics/Yellowtail/Gesture.cc"
  "examples/Demos/Graphics/Yellowtail/Vec3f.cc"
  "examples/Demos/Graphics/Yellowtail/Yellowtail.cc"
  "examples/Demos/Performance/CubicGridRetained/CubicGridRetained.cc"
  "examples/Demos/Performance/DynamicParticlesImmediate/DynamicParticlesImmediate.cc"
  "examples/Demos/Performance/DynamicParticlesRetained/DynamicParticlesRetained.cc"
  "examples/Demos/Performance/StaticParticlesImmediate/StaticParticlesImmediate.cc"
  "examples/Demos/Performance/StaticParticlesRetained/StaticParticlesRetained.cc"
  "examples/Demos/Tests/MultipleWindows/Arcball.cc"
  "examples/Demos/Tests/MultipleWindows/MultipleWindows.cc"
  "examples/Demos/Tests/OffscreenTest/OffscreenTest.cc"
  "examples/Demos/Tests/ResizeTest/ResizeTest.cc"
  "examples/Demos/Tests/SpecsTest/SpecsTest.cc"
  "examples/Topics/Advanced Data/CountingStrings/CountingStrings.cc"
  "examples/Topics/Advanced Data/HashMapClass/HashMapClass.cc"
  "examples/Topics/Advanced Data/HashMapClass/Word.cc"
  "examples/Topics/Advanced Data/IntListLottery/IntListLottery.cc"
  "examples/Topics/Advanced Data/LoadSaveJSON/Bubble.cc"
  "examples/Topics/Advanced Data/LoadSaveJSON/LoadSaveJSON.cc"
  "examples/Topics/Advanced Data/LoadSaveTable/Bubble.cc"
  "examples/Topics/Advanced Data/LoadSaveTable/LoadSaveTable.cc"
  "examples/Topics/Advanced Data/LoadSaveXML/Bubble.cc"
  "examples/Topics/Advanced Data/LoadSaveXML/LoadSaveXML.cc"
  "examples/Topics/Advanced Data/Regex/Regex.cc"
  "examples/Topics/Advanced Data/Threads/Threads.cc"
  "examples/Topics/Advanced Data/XMLYahooWeather/XMLYahooWeather.cc"
  "examples/Topics/Cellular Automata/Spore1/Spore1.cc"
  "examples/Topics/Cellular Automata/Spore2/Spore2.cc"
  "examples/Topics/Create Shapes/BeginEndContour/BeginEndContour.cc"
  "examples/Topics/Create Shapes/GroupPShape/GroupPShape.cc"
  "examples/Topics/Create Shapes/ParticleSystemPShape/Particle.cc"
  "examples/Topics/Create Shapes/ParticleSystemPShape/ParticleSystem.cc"
  "examples/Topics/Create Shapes/ParticleSystemPShape/ParticleSystemPShape.cc"
  "examples/Topics/Create Shapes/PolygonPShapeOOP3/Polygon.cc"
  "examples/Topics/Create Shapes/PolygonPShapeOOP3/PolygonPShapeOOP3.cc"
  "examples/Topics/Create Shapes/WigglePShape/WigglePShape.cc"
  "examples/Topics/Create Shapes/WigglePShape/Wiggler.cc"
  "examples/Topics/Curves/ArcLengthParametrization/ArcLengthParametrization.cc"
  "examples/Topics/Curves/ArcLengthParametrization/BezierCurve.cc"
  "examples/Topics/File IO/DirectoryList/DirectoryList.cc"
  "examples/Topics/File IO/SaveFile1/SaveFile1.cc"
  "examples/Topics/File IO/SaveFile2/SaveFile2.cc"
  "examples/Topics/File IO/TileImages/TileImages.cc"
  "examples/Topics/Image Processing/Blending/Blending.cc"
  "examples/Topics/Image Processing/EdgeDetection/EdgeDetection.cc"
  "examples/Topics/Image Processing/Explode/Explode.cc"
  "examples/Topics/Image Processing/Extrusion/Extrusion.cc"
  "examples/Topics/Image Processing/Histogram/Histogram.cc"
  "examples/Topics/Image Processing/LinearImage/LinearImage.cc"
  "examples/Topics/Image Processing/PixelArray/PixelArray.cc"
  "examples/Topics/Interaction/Tickle/Tickle.cc"
  "examples/Topics/Shaders/BlurFilter/BlurFilter.cc"
  "examples/Topics/Shaders/Conway/Conway.cc"
  "examples/Topics/Shaders/CustomBlend/CustomBlend.cc"
  "examples/Topics/Shaders/Deform/Deform.cc"
  "examples/Topics/Shaders/DomeProjection/CubeMapUtils.cc"
  "examples/Topics/Shaders/DomeProjection/DomeProjection.cc"
  "examples/Topics/Shaders/EdgeDetect/EdgeDetect.cc"
  "examples/Topics/Shaders/EdgeFilter/EdgeFilter.cc"
  "examples/Topics/Shaders/GlossyFishEye/GlossyFishEye.cc"
  "examples/Topics/Shaders/ImageMask/ImageMask.cc"
  "examples/Topics/Shaders/InfiniteTiles/InfiniteTiles.cc"
  "examples/Topics/Shaders/SepBlur/SepBlur.cc"
  "examples/Topics/Simulate/SoftBody/SoftBody.cc"
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
    target_compile_options(${base_name} PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/processing.h)
  else()
    add_executable(${base_name} sketch.cc)
    target_link_libraries(${base_name}  PRIVATE Processing)
    target_compile_options(${base_name} PRIVATE -DFILE_TO_INCLUDE=\"${file_path}\" -include ${CMAKE_CURRENT_SOURCE_DIR}/processing.h)
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

endfunction()

# Add all the examples targets
file(GLOB_RECURSE files_to_process CONFIGURE_DEPENDS "./examples/*.cc")
foreach(file_path ${files_to_process})
  file(RELATIVE_PATH file_relative "${CMAKE_CURRENT_SOURCE_DIR}" "${file_path}")
  add_example( ${file_relative} )
endforeach()
