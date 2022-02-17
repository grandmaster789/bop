set(UNITTEST "unittest")

add_executable(${UNITTEST}
	"main.cpp"
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(${UNITTEST} PRIVATE 
	GTest::gmock 
	GTest::gtest 
	GTest::gmock_main 
	GTest::gtest_main
)
