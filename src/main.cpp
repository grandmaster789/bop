#include <iostream>
#include <coroutine>
#include <stdexcept>
#include <thread>

#include "util/traits.h"
#include "util/concepts.h"

#include "job/job.h"
#include "job/job_queue.h"
#include "job/job_system.h"

int main() {
	std::cout << "Starting application\n";

	bop::job::JobSystem js; // creating a local instance will initialize the static class, optionally with some settings
	bop::schedule([] { std::cout << "ping\n"; });
	bop::schedule([] { bop::shutdown(); });
	bop::wait_for_shutdown();

	std::cout << "Completed application\n";
}