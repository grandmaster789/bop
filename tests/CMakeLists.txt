set(UNITTEST "unittest")

add_library(${UNITTEST}
	"test.cpp"
)

find_package(Catch2 CONFIG REQUIRED)

target_link_libraries(${UNITTEST} PRIVATE 
	Catch2::Catch2
)

#include(CTest)
#include(Catch)
#catch_discover_tests(${UNITTEST})