set(BOP_LIB "bop")
set(BOP_MAIN "main")

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(${BOP_MAIN} 
	"main.cpp"
 "job/co_generator.h")

add_library(${BOP_LIB}
	"job/job.h"
	"job/job.cpp"
	"job/job_queue.h"
	"job/job_system.h"
	"job/job_system.cpp"
	"job/job_queue.cpp" 
	"job/job_trace.h" 
	"job/job_trace.cpp" 
	"job/co_job.h" 
	"job/co_job_promise.h" 
	"job/co_job_promise.cpp"
	
	"util/curry.h" 
	"util/concepts.h"	
	"util/traits.h"
  "job/co_generator.h")

 target_link_libraries(${BOP_MAIN} PRIVATE ${BOP_LIB})

target_link_libraries(${BOP_LIB} PRIVATE 
	nlohmann_json 
	nlohmann_json::nlohmann_json
)