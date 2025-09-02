
cmake_minimum_required(VERSION 3.16)

project("ic4-ctrl")

find_package( ic4 REQUIRED )

if (NOT DISABLE_FETCHCONTENT_PACKAGES)
	include(FetchContent)

	FetchContent_Declare(
		CLI11
		GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
		GIT_TAG v2.5.0
	)

	FetchContent_MakeAvailable(CLI11)

	FetchContent_Declare(
		fmt
		GIT_REPOSITORY https://github.com/fmtlib/fmt.git
		GIT_TAG 11.0.2
	)

	FetchContent_MakeAvailable(fmt)

	FetchContent_Declare(
		nlohmann-json
		GIT_REPOSITORY https://github.com/nlohmann/json.git
		GIT_TAG v3.12.0
	)

	FetchContent_MakeAvailable(nlohmann-json)
else()
	find_package( CLI11 REQUIRED )
	find_package( fmt REQUIRED )
	find_package( nlohmann_json REQUIRED )
endif()

add_executable( ic4-ctrl
	"src/ic4-ctrl.cpp"
	"src/ic4_enum_to_string.h"
	"src/ic4-ctrl-helper.h"

	"src/print_property.h"
	"src/print_property.cpp"
 "src/helper_json.cpp")

target_link_libraries( ic4-ctrl
PRIVATE
	ic4::core
	CLI11::CLI11
	fmt::fmt
	nlohmann_json::nlohmann_json
)

if (WIN32)
	target_link_libraries( ic4-ctrl
	PRIVATE
		ic4::gui
	)
endif()

set_property(TARGET ic4-ctrl PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(ic4-ctrl)
