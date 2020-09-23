#include "pch.h"
#include "EdMenu.h"

#include "editor/imgui/ImEd.h"


namespace ed {


ed::Menu::MenuOption& ed::Menu::AddEntry(const char* inName, std::function<void()>&& funcPtr,
	std::function<bool()>&& isSelectedBind, std::function<bool()>&& isVisibleBind)
{

	MenuOption& option = *options.emplace_back(std::make_unique<MenuOption>());
	option.name = inName;
	option.func = std::move(funcPtr);

	if (isSelectedBind) {
		option.isSelectedFunc = std::move(isSelectedBind);
	}

	if (isVisibleBind) {
		option.isVisibleFunc = std::move(isVisibleBind);
	}

	return option;
}

ed::Menu::MenuOption& ed::Menu::AddEntry(MenuOption&& option)
{
	return *options.emplace_back(std::make_unique<MenuOption>(std::move(option)));
}

ed::Menu::MenuOption& ed::Menu::AddUnderCategory(const std::string& category, const char* optionName,
	std::function<void()>&& funcPtr, std::function<bool()>&& isSelectedBind)
{
	return AddSubMenu(category).AddEntry(optionName, std::move(funcPtr), std::move(isSelectedBind));
}

ed::Menu::MenuOption& Menu::AddOptionalCategory(const char* category, const char* optionName,
	std::function<void()>&& funcPtr, std::function<bool()>&& isSelectedBind)
{
	if (!category || category[0] == '\0') {
		return AddEntry(optionName, std::move(funcPtr), std::move(isSelectedBind));
	}
	return AddUnderCategory(category, optionName, std::move(funcPtr), std::move(isSelectedBind));
}

ed::Menu& ed::Menu::AddSubMenu(const std::string& menuName)
{
	auto submenuOptionIt = categories.find(menuName);

	if (submenuOptionIt != categories.end()) {
		return *submenuOptionIt->second->menu;
	}
	//
	auto& submenuOpt = *options.emplace_back(std::make_unique<MenuOption>());
	submenuOpt.menu = std::make_unique<Menu>(menuName.c_str());
	submenuOpt.name = menuName;

	categories.emplace(menuName, &submenuOpt);

	return *submenuOpt.menu;
}

void ed::Menu::DrawOptions(std::optional<glm::vec2> subItemframePadding, std::optional<glm::vec2> subItemItemSpacing)
{
	for (auto& entry : options) {
		if (!entry) {
			ImGui::Separator();
			continue;
		}

		if (entry->isVisibleFunc) {
			if (!entry->isVisibleFunc()) {
				continue;
			}
		}

		if (entry->menu) {
			if (ImGui::BeginMenu(entry->name.c_str())) {
				int32 valuesToPop = 0;
				if (subItemframePadding.has_value()) {
					ImGui::PushStyleVar(
						ImGuiStyleVar_FramePadding, ImVec2(subItemframePadding->x, subItemframePadding->y));
					valuesToPop++;
				}
				if (subItemItemSpacing.has_value()) {
					ImGui::PushStyleVar(
						ImGuiStyleVar_ItemSpacing, ImVec2(subItemItemSpacing->x, subItemItemSpacing->y));
					valuesToPop++;
				}
				entry->menu->DrawOptions();

				ImGui::PopStyleVar(valuesToPop);

				ImGui::EndMenu();
			}
			continue;
		}

		bool selected = entry->isSelectedFunc ? entry->isSelectedFunc() : false;
		if (ImGui::MenuItem(entry->name.c_str(), nullptr, selected)) {
			std::invoke(entry->func);
		}
	}
}

void ed::Menu::Draw()
{
	bool open = ImEd::BeginMenu(name);
	if (open) {
		DrawOptions();
		ImEd::EndMenu();
	}
}


} // namespace ed
