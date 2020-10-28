#include "test.h"

#include "universe/World.h"
#include "universe/ComponentsDb.h"

struct CTestComponent {

	CTestComponent(bool* ptr = nullptr)
		: hasEndedPtr(ptr)
	{
	}


	REFLECTED_COMP(CTestComponent) {}

	bool hasBegun{ false };
	bool hasEnded{ false };
	bool hasTicked{ false };
	bool* hasEndedPtr{ nullptr };

	void BeginPlay() { hasBegun = true; }

	void EndPlay()
	{
		hasEnded = true;
		if (hasEndedPtr) {
			*hasEndedPtr = true;
		}
	}

	void Tick(float deltaSeconds) { hasTicked = true; }
};


TEST("Basic Entity Begin/End Play")
{
	World world;

	auto entity = world.CreateEntity("Test entity");
	auto& comp = entity.Add<CTestComponent>();

	world.BeginPlay();
	REQ(comp.hasBegun);


	world.UpdateWorld(nullptr);
	REQ(comp.hasTicked);

	world.EndPlay();
	REQ(comp.hasEnded);
}


TEST("Live World Entity Begin/End Play")
{
	World world;
	auto entity = world.CreateEntity("Test entity");

	bool didEnd = false;

	world.BeginPlay();

	world.UpdateWorld(nullptr);
	auto& comp = entity.Add<CTestComponent>(&didEnd);
	REQ(comp.hasBegun);
	REQ(!comp.hasTicked);

	entity.SafeRemove<CTestComponent>();
	REQ(didEnd);

	world.EndPlay();
}
