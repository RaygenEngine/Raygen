#include "Test.h"
#include "system/Logger.h"
#include "system/profiler/ProfileScope.h"
#include <thread>

TEST("Profiler")
{
	using namespace std::literals;

	Profiler::BeginProfiling();
	Profiler::Z_ClearRegistrations();

	for (int32 i = 0; i < 5; ++i) {
		PROFILE_SCOPE(World);
		std::this_thread::sleep_for(1ms);
	}


	// TODO:
	// This does is not the best but the only way to properly pass all cases
	if constexpr (IsEnabled(ProfilerSetup::World)) {
		REQ(Profiler::GetAll().size() > 0);

		auto vec = Profiler::GetModule(ProfilerSetup::World);
		REQ(vec != nullptr);

		REQ(vec->size() == 1);


		REQ(vec->at(0)->hits == 5);
		REQ(vec->at(0)->sumDuration >= 4ms);

		Profiler::BeginFrame();

		REQ(vec->at(0)->hits == 0);
		REQ(vec->at(0)->sumDuration == 0s);
	}
	else {
		REQ(Profiler::GetAll().size() == 0);

		auto vec = Profiler::GetModule(ProfilerSetup::World);
		REQ(vec == nullptr);
	}
}
