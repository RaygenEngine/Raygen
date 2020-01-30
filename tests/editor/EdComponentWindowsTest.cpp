#include "test.h"
#include "editor/EdComponentWindows.h"


namespace ed {
int32 g_uniqueDraws = 0;
class UniqueWin : public Window {
public:
	UniqueWin(const std::string& name)
		: Window(name)
	{
	}

	void OnDraw() override { g_uniqueDraws++; }

	~UniqueWin() override = default;
};

TEST("Unique Windows")
{
	ComponentWindows comp;
	comp.AddWindowEntry<UniqueWin>("Test Win");
	mti::Hash hash = mti::GetHash<UniqueWin>();

	SECT("can open")
	{
		comp.OpenUnique<UniqueWin>();
		REQ(comp.IsUniqueOpen(mti::GetHash<UniqueWin>()));

		SECT("then close")
		{
			comp.CloseUnique<UniqueWin>();
			REQ(!comp.IsUniqueOpen<UniqueWin>());
		}
	}

	SECT("can toggle")
	{
		comp.ToggleUnique<UniqueWin>();
		REQ(comp.IsUniqueOpen(hash));
		comp.ToggleUnique<UniqueWin>();
		REQ(!comp.IsUniqueOpen(hash));
	}
}


int32 g_OpenClose = 0;
ComponentWindows* g_ComponoentWindow = nullptr;

class UniqueWinOpen : public Window {
public:
	UniqueWinOpen(const std::string& name)
		: Window(name)
	{
	}

	void OnDraw() override
	{
		if (g_OpenClose == 1) {
			g_ComponoentWindow->OpenUnique<UniqueWin>();
		}
		else if (g_OpenClose == 2) {
			g_ComponoentWindow->CloseUnique<UniqueWin>();
		}
		else if (g_OpenClose == 3) { // Self remove
			g_ComponoentWindow->CloseUnique<UniqueWinOpen>();
		}
	}

	~UniqueWinOpen() override = default;
};

TEST("Adding / Removing windows while iterating")
{
	ComponentWindows comp;
	g_ComponoentWindow = &comp;
	g_OpenClose = 0;
	g_uniqueDraws = 0;
	comp.AddWindowEntry<UniqueWin>("Unique Win");
	comp.AddWindowEntry<UniqueWinOpen>("Unique Win Open Close");

	comp.OpenUnique<UniqueWinOpen>();

	SECT("can self remove last")
	{
		g_OpenClose = 3;
		comp.ZTest_Draw(); // If crashing here the removal of oneself is wrong
		REQ(comp.m_uniqueWindows.size() == 0);
	}

	SECT("can add while iterating")
	{
		g_OpenClose = 1;
		comp.ZTest_Draw();
		REQ(g_uniqueDraws == 1); // Verify we iterated the "added" window

		SECT("and then remove")
		{
			g_OpenClose = 2;
			comp.ZTest_Draw();
			// Removing another window while iterating produces "undefined" on whether the other draw will run this
			// frame, but remains a valid removal and should not skip other entries.
			REQ(comp.m_uniqueWindows.size() == 1);
		}
	}
}


}; // namespace ed
