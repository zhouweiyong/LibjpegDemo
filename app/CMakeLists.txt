

cmake_minimum_required(VERSION 3.4.1)


add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp )

set(distribution_dir ${CMAKE_SOURCE_DIR}/libs/)

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library(android-lib android)

add_library(libjpeg-lib STATIC IMPORTED)
set_target_properties(libjpeg-lib  PROPERTIES IMPORTED_LOCATION ${distribution_dir}/${ANDROID_ABI}/libjpeg.a)

include_directories(src/main/cpp/include/)

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${android-lib}
                       jnigraphics
                       libjpeg-lib)