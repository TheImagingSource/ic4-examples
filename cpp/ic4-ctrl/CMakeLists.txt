
cmake_minimum_required(VERSION 3.8)

project("ic4-ctrl")

find_package( ic4 
	REQUIRED
)
find_package( CLI11 REQUIRED )
find_package( fmt REQUIRED )

add_executable( ic4-ctrl 
	"src/ic4-ctrl.cpp"
	"src/ic4_enum_to_string.h"
)

target_link_libraries( ic4-ctrl
PRIVATE
	ic4::core 
	CLI11::CLI11
	fmt::fmt
)

set_property(TARGET ic4-ctrl PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(ic4-ctrl)