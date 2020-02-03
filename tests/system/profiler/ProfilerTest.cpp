#include "Test.h"
#include "system/Logger.h"
#include "system/profiler/ProfileScope.h"

TEST("Profiler")
{
	using namespace std::literals;

	Profiler::s_isProfiling = true;
	Profiler::EndFrame();
	Profiler::Z_ClearRegistrations();

	for (int32 i = 0; i < 5; ++i) {
		PROFILE_SCOPE(Test);
		std::this_thread::sleep_for(1ms);
	}

	// This does is not the best but the only way to properly pass all cases
	if constexpr (IsEnabled(ProfilerSetup::Test)) {
		REQ(Profiler::GetAll().size() > 0);

		auto vec = Profiler::GetModule(ProfilerSetup::Test);
		REQ(vec != nullptr);

		REQ(vec->size() == 1);


		REQ(vec->at(0)->hits == 5);
		REQ(vec->at(0)->sumDuration >= 4ms);

		Profiler::EndFrame();

		REQ(vec->at(0)->hits == 0);
		REQ(vec->at(0)->sumDuration == 0s);
	}
	else {
		REQ(Profiler::GetAll().size() == 0);

		auto vec = Profiler::GetModule(ProfilerSetup::Test);
		REQ(vec == nullptr);
	}
}
