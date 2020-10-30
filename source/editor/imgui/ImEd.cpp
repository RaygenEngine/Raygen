#include "ImEd.h"

#include "assets/PodEntry.h"
#include "assets/AssetRegistry.h"
#include "editor/EdMenu.h"
#include "reflection/PodTools.h"
#include "assets/PodIncludes.h"
#include "universe/ComponentsDb.h"

namespace ImEd {
int InputTextCallback(ImGuiInputTextCallbackData* data);

struct InputTextCallback_UserData {
	std::string* Str;
	ImGuiInputTextCallback ChainCallback;
	void* ChainCallbackUserData;
};


bool Button(const char* label, const ImVec2& size)
{
	bool result;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
	result = ImGui::Button(label, size);
	ImGui::PopStyleVar();
	return result;
}

void SetNextItemPerc(float perc)
{
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * perc);
}

bool BeginMenuBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 7.f)); // On edit update imextras.h c_MenuPaddingY
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 7.f));
	if (!ImGui::BeginMenuBar()) {
		ImGui::PopStyleVar(2);
		return false;
	}
	ImVec2 scaledSize = ImVec2(800, 25);
	auto cursorPos = ImGui::GetCursorPos();

	if (ImGui::InvisibleButton("##MainMenuBarItemButton", scaledSize, ImGuiButtonFlags_PressedOnClick)) {
		LOG_REPORT("Clicked");
	}

	ImGui::SetCursorPos(cursorPos);
	return true;
}

bool BeginMenu(const char* label, bool enabled)
{
	bool open = ImGui::BeginMenu(label, enabled);
	if (open) {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 3.f));
		ImGui::Spacing();
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f, 7.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.f, 7.f));
		return true;
	}
	return false;
}

void EndMenuBar()
{
	ImGui::EndMenuBar();
	ImGui::PopStyleVar(2);
}


void EndMenu()
{
	ImGui::PopStyleVar(2);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, -3.f));
	ImGui::Spacing();
	ImGui::PopStyleVar();
	ImGui::EndMenu();
}

void HSpace(float space)
{
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(space, 0.f));
}


int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back
		// to what we want.
		std::string* str = user_data->Str;
		IM_ASSERT(data->Buf == str->c_str());
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
	}
	else if (user_data->ChainCallback) {
		// Forward to user callback, if any
		data->UserData = user_data->ChainCallbackUserData;
		return user_data->ChainCallback(data);
	}
	return 0;
}

bool InputTextSized(const char* label, std::string* str, ImVec2 size, ImGuiInputTextFlags flags,
	ImGuiInputTextCallback callback, void* user_data)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;

	return ImGui::InputTextEx(label, NULL, const_cast<char*>(str->c_str()), static_cast<int>(str->capacity()) + 1, size,
		flags, InputTextCallback, &cb_user_data);
}

bool IsItemDoubleClicked(ImGuiMouseButton button)
{
	return ImGui::IsItemClicked(button) && ImGui::IsMouseDoubleClicked(button);
}

void CreateTypedPodDrag(PodEntry* entry, ImGuiDragDropFlags flags)
{
	if (ImGui::BeginDragDropSource(flags)) {
		ImGui::PushFont(ImguiImpl::s_AssetIconFont);
		ImGui::TextUnformatted(U8(entry->GetClass()->GetIcon()));
		ImGui::PopFont();
		ImGui::TextUnformatted(std::string(entry->GetName()).c_str());

		std::string payloadTag = "POD_UID_" + std::to_string(entry->type.hash());
		ImGui::SetDragDropPayload(payloadTag.c_str(), &entry, sizeof(PodEntry*));
		ImGui::EndDragDropSource();
	}
}

void CreateTypedPodDrag(BasePodHandle handle, ImGuiDragDropFlags flags)
{
	auto entry = AssetRegistry::GetEntry(handle);
	CreateTypedPodDrag(entry, flags);
}

int AssetNameFilter::FilterImGuiLetters(ImGuiInputTextCallbackData* data)
{
	// CHECK: Define and use in asset manager somewhere
	if (data->EventChar < 256 && !strchr("./<>:\\|?*~#", (char)data->EventChar)) {
		return 0;
	}
	return 1;
}


void HelpTooltip(const char* tooltip)
{
	ImEd::HSpace(1.f);
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 0.95f));
	ImGui::TextUnformatted("\xc2\xb0"); // help symbol: aka U+00b0
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
void HelpTooltipInline(const char* tooltip)
{
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void CollapsingHeaderHelpTooltip(const char* tooltip)
{
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 12.f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.6f, 0.95f));
	ImGui::TextUnformatted("?");
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool TransformRun(TransformCache& tr, bool showScale, bool* lockedScale, bool* localMode, bool* displayMatrix,
	bool* lookAt, glm::vec3* lookAtPos)
{
	bool validParams = ((lookAt == nullptr) && (lookAtPos == nullptr)) || (lookAt != nullptr && lookAtPos != nullptr);

	CLOG_ABORT(!validParams,
		"Invalid parameters for TransformManipulator function. lookAt & lookAtPos should both be valid or "
		"nullptr.");


	bool changed = false;

	const auto UpdateLookAtReference = [&]() {
		if (lookAtPos) {
			(*lookAtPos) = tr.position + tr.front();
		}
	};

	if (ImGui::DragFloat3("Position", glm::value_ptr(tr.position), 0.02f)) {
		changed = true;
	}
	if (ImGui::BeginPopupContextItem("PositionPopup")) {
		if (ImGui::MenuItem("Reset##1")) {
			tr.position = {};
			changed = true;
		}
		if (ImGui::MenuItem("Identity##1")) {
			tr.transform = glm::identity<glm::mat4>();
			tr.position = {};
			tr.orientation = glm::identity<glm::quat>();
			tr.scale = glm::vec3(1.f);
			changed = true;
		}
		ImGui::EndPopup();
	}

	if (!lookAt || !lookAtPos || *lookAt == false) {
		glm::vec3 eulerPyr = tr.pyr();
		if (ImGui::DragFloat3("Rotation", glm::value_ptr(eulerPyr), 0.2f)) {
			if (ImGui::IsAnyMouseDown()) {
				// On user drag use quat diff, prevents gimbal locks while dragging
				auto deltaAxis = eulerPyr - tr.pyr();
				tr.orientation = glm::quat(glm::radians(deltaAxis)) * tr.orientation;
				changed = true;
			}
			else {
				// On user type set pyr directly, prevents the axis from flickering
				tr.orientation = glm::quat(glm::radians(eulerPyr));
				changed = true;
			}
		}
	}
	else {
		ImGui::DragFloat3("Look At", glm::value_ptr(*lookAtPos), 0.1f);
		ImEd::HelpTooltipInline(
			"Look At will lock lookat position of the transform for as long as it is active. This way you can "
			"adjust the position while updating the orientation to look at a fixed point.");
		tr.orientation = math::findLookAt(tr.position, *lookAtPos);
		changed = true;
	}

	if (ImGui::BeginPopupContextItem("RotatePopup")) {
		if (ImGui::MenuItem("Reset##2")) {
			tr.orientation = glm::identity<glm::quat>();
			changed = true;
		}
		if (lookAt && ImGui::MenuItem("Look At", nullptr, *lookAt)) {
			*lookAt = !*lookAt;
			if (*lookAt) {
				UpdateLookAtReference();
			}
		}
		ImGui::EndPopup();
	}

	if (showScale) {
		if (!*lockedScale) {
			if (ImGui::DragFloat3("Scale", glm::value_ptr(tr.scale), 0.02f)) {
				changed = true;
			}
		}
		else {
			glm::vec3 newScale = tr.scale;
			if (ImGui::DragFloat3("Locked Scale", glm::value_ptr(newScale), 0.02f)) {
				changed = true;

				glm::vec3 initialScale = tr.scale;

				float ratio = 1.f;
				if (!math::equals(newScale.x, initialScale.x)) {
					ratio = newScale.x / initialScale.x;
				}
				else if (!math::equals(newScale.y, initialScale.y)) {
					ratio = newScale.y / initialScale.y;
				}
				else if (!math::equals(newScale.z, initialScale.z)) {
					ratio = newScale.z / initialScale.z;
				}

				ratio += 0.00001f;
				tr.scale = initialScale * ratio;
			}
		}

		if (ImGui::BeginPopupContextItem("ScalePopup")) {
			if (ImGui::MenuItem("Reset##3")) {
				changed = true;
				tr.scale = glm::vec3(1.f);
			}
			if (lockedScale && ImGui::MenuItem("Lock", nullptr, lockedScale)) {
				*lockedScale = !*lockedScale;
			}
			ImGui::EndPopup();
		}
	}


	if (changed) {
		tr.Compose();
	}

	if (localMode) {
		ImGui::Checkbox("Local Mode", localMode);
		ImEd::HelpTooltipInline("Toggles local/global space for TRS and transform matrix editing.");
		ImGui::SameLine(0.f, 16.f);
	}


	if (displayMatrix) {
		ImGui::Checkbox("Display Matrix", displayMatrix);
		ImEd::HelpTooltipInline("Toggles visiblity and editing of matricies as a row major table.");
		if (*displayMatrix) {
			glm::mat4 matrix = tr.transform;

			// to row major
			auto rowMajor = glm::transpose(matrix);

			bool edited = false;

			edited |= ImGui::DragFloat4("mat.row[0]", glm::value_ptr(rowMajor[0]), 0.01f);
			edited |= ImGui::DragFloat4("mat.row[1]", glm::value_ptr(rowMajor[1]), 0.01f);
			edited |= ImGui::DragFloat4("mat.row[2]", glm::value_ptr(rowMajor[2]), 0.01f);
			edited |= ImGui::DragFloat4("mat.row[3]", glm::value_ptr(rowMajor[3]), 0.01f);
			if (edited) {
				tr.transform = glm::transpose(rowMajor);
				tr.Decompose();
				changed = true;
			}
		}
	}
	return changed;
}

PodEntry* AcceptGenericPodDrop(std::function<void(BasePodHandle, PodEntry*)> onDropped)
{
	PodEntry* result = nullptr;
	podtools::ForEachPodType([&]<typename PodType>() {
		if (result) {
			return;
		}

		if (onDropped) {
			result = AcceptTypedPodDrop<PodType>(onDropped);
		}
		else {
			result = AcceptTypedPodDrop<PodType>();
		}
	});
	return result;
}


ed::Menu MakeMenu(const ComponentMetaEntry** outEntryPtr)
{
	using namespace ed;
	Menu menu;
	static std::vector<UniquePtr<std::string>> stringBank;

	auto& categories = ComponentsDb::Z_GetCategories();

	auto add = [&, outEntryPtr](const std::string& cat, auto& types) {
		for (auto type : types) {
			if (auto entry = ComponentsDb::GetType(type); entry) {
				auto& cl = *entry->clPtr;
				auto& str = *stringBank.emplace_back(
					std::make_unique<std::string>(fmt::format("{}   {}", U8(cl.GetIcon()), cl.GetName().substr(1))));
				menu.AddOptionalCategory(cat.c_str(), str.c_str(), [=]() {
					//
					(*outEntryPtr) = entry;
				});
			}
		}
	};

	for (auto& [cat, types] : categories) {
		if (!cat.empty()) {
			add(cat, types);
		}
	}

	if (auto it = categories.find(""); it != categories.end()) {
		add(it->first, it->second);
	}

	return menu;
}

bool ButtonNoBorderText(const char* text, ImVec2 extraPad, bool padIsSize)
{
	ImVec2 size;
	if (!padIsSize) {
		size = ImGui::CalcTextSize(text, nullptr, true);
		size += extraPad;
		size += extraPad;
	}
	else {
		size = extraPad;
	}

	return ImGui::Selectable(text, false, 0, size);
}

bool ButtonIcon(const char8_t* icon, ImVec2 size)
{
	char8_t txt[] = u8" ???? ";
	txt[1] = icon[0];
	txt[2] = icon[1];
	txt[3] = icon[2];
	txt[4] = icon[3];
	return ImGui::Selectable(U8(txt), false, 0, size);
}

bool ButtonMediumIcon(const char8_t* icon, ImVec2 size)
{
	ImGui::PushFont(ImguiImpl::s_MediumSizeIconFont);
	const bool result = ImGui::Selectable(U8(icon), false, 0, size);
	ImGui::PopFont();
	return result;
}

const ComponentMetaEntry* ComponentClassMenu()
{
	static const ComponentMetaEntry* entry;
	static ed::Menu menu = MakeMenu(&entry);

	entry = nullptr;
	menu.DrawOptions();
	return entry;
}

} // namespace ImEd
