set(BOP_LIB "bop")
set(BOP_MAIN "main")

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(${BOP_MAIN} 
	"main.cpp"
)

add_library(${BOP_LIB}
	"job/job.h"
	"job/job.cpp"	
	"job/job_queue.h"
	"job/job_system.h"
	"job/job_system.cpp"
	"job/job_queue.cpp" 
	"job/job_trace.h" 
	"job/job_trace.cpp" 
	"job/co_generator.h" 
	"job/co_job.h" 
	"job/co_job_promise.h"
	
	"task/task_system.h" 
	"task/task.h" 
	"task/task_system.cpp"

	"util/cacheline.h"
	"util/curry.h" 
	"util/concepts.h"	
	"util/traits.h"
	"util/spinlock.h" 
	"util/spinlock.cpp" 
	"util/manual_lifetime.h" 
	"util/platform.h"
	"util/overloaded.h"
	"util/typelist.h"
	"util/function.h"
)

 target_link_libraries(${BOP_MAIN} PRIVATE ${BOP_LIB})

target_link_libraries(${BOP_LIB} PRIVATE 
	nlohmann_json 
	nlohmann_json::nlohmann_json
)