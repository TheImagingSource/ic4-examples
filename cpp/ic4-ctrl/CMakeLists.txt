
cmake_minimum_required(VERSION 3.8)

project("ic4-ctrl")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

find_package( CLI11 REQUIRED )
find_package( fmt REQUIRED )

add_executable( ic4-ctrl
	"src/ic4-ctrl.cpp"
	"src/ic4_enum_to_string.h"
	"src/ic4-ctrl-helper.h"
)

target_link_libraries( ic4-ctrl
  PRIVATE
  ic4::core
  CLI11::CLI11
  fmt::fmt
)

if (WIN32)

  target_link_libraries( ic4-ctrl
    PRIVATE
	ic4::gui
  )

endif()


set_property(TARGET ic4-ctrl PROPERTY CXX_STANDARD 14)

if (WIN32)

  ic4_copy_runtime_to_target(ic4-ctrl)

endif()
