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

	// dummy tasks to verify that the trace data is correctly displayed by chrome
	for (int i = 0; i < 50; ++i)
		bop::schedule([] {
			using namespace std::chrono_literals;

			for (int i = 5; i > 0; --i) {
				std::cout << "** " << i << "\n";
				std::this_thread::sleep_for(1s);
			 }
		});

	bop::schedule([] { bop::shutdown(); });
	bop::wait_for_shutdown();

	std::cout << "Completed application\n";
}