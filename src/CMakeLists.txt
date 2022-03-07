set(BOP "bop")

add_executable(${BOP}
	"main.cpp"
	"util/typelist.h" 
	"util/valuelist.h" 
	"util/traits.h"
	"util/signature.h"
 "util/concepts.h")

target_link_libraries(${BOP} PRIVATE 
)