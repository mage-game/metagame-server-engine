
include_directories(.)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_library(fastdb ${SRC_FASTDB})
