set(BOP "bop")

add_executable(${BOP}
	"main.cpp"
 "tasks/task.h" "primitives/cancellation.h" )

target_link_libraries(${BOP} PRIVATE 
)