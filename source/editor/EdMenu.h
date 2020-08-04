#pragma once


namespace ed {
struct Menu;


struct Menu {


	struct MenuOption {
		std::string name;
		std::function<void()> func;
		std::function<bool()> isSelectedFunc;
		UniquePtr<Menu> menu;
	};

	// PERF: Heap allocation galore
	const char* name{ nullptr };

	std::vector<UniquePtr<MenuOption>> options;
	std::unordered_map<std::string, MenuOption*> categories;

	void AddSeperator() { options.emplace_back(); }

	MenuOption& AddEntry(
		const char* inName, std::function<void()>&& funcPtr, std::function<bool()>&& isSelectedBind = {});


	MenuOption& AddEntry(MenuOption&& option);

	MenuOption& AddUnderCategory(const std::string& category, const char* optionName, std::function<void()>&& funcPtr,
		std::function<bool()>&& isSelectedBind = {});

	// categroy can be nullptr to directly add
	MenuOption& AddOptionalCategory(const char* category, const char* optionName, std::function<void()>&& funcPtr,
		std::function<bool()>&& isSelectedBind = {});

	Menu& AddSubMenu(const std::string& menuName);


	void DrawOptions();

	void Draw();

	Menu(const char* inName)
		: name(inName)
	{
	}

	Menu() = default;


	void Clear()
	{
		options.clear();
		categories.clear();
	}
};
} // namespace ed
