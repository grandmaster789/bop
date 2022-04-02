set(UNITTEST "unittest")

add_library(catch_main "catch_main.cpp")
target_link_libraries(catch_main PUBLIC Catch2::Catch2)

add_executable(${UNITTEST}
	"test.cpp"
)

find_package(Catch2 REQUIRED)

target_link_libraries(${UNITTEST} PRIVATE 
	catch_main
)

include(CTest)
include(Catch)
catch_discover_tests(${UNITTEST})