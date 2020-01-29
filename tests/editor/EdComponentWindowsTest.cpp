#include "test.h"
#include "editor/EdComponentWindows.h"


namespace ed {
class UniqueWin : public Window {
public:
	UniqueWin(const std::string& name)
		: Window(name)
	{
	}

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


}; // namespace ed
