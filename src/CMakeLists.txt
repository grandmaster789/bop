set(BOP "bop")

find_package(nlohmann_json CONFIG REQUIRED)


add_executable(${BOP}
	"main.cpp"
	"job/job.h"
	"job/job.cpp"
	"job/job_queue.h"
	"job/job_system.h"
	"job/job_system.cpp"

	"util/concepts.h"
	"util/thread_types.h"
	"util/traits.h"
	"util/simple_function.h"
	"util/simple_function.cpp"	
	"util/tag.h"
)

target_link_libraries(${BOP} PRIVATE 
	nlohmann_json 
	nlohmann_json::nlohmann_json
)