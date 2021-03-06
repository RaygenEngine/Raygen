#include "WindowsRegistry.h"

#include "assets/PodEditor.h"
#include "assets/pods/Image.h"
#include "editor/windows/EdMeshGenerator.h"
#include "editor/windows/editors/EdMaterialArchetypeEditor.h"
#include "editor/windows/general/EdAssetListWindow.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "editor/windows/general/EdAttachmentDebuggerWindow.h"
#include "editor/windows/general/EdConsoleWindow.h"
#include "editor/windows/general/EdMiscWindow.h"
#include "editor/windows/general/EdOutlinerWindow.h"
#include "editor/windows/general/EdProfilerWindow.h"
#include "editor/windows/general/EdPropertyEditorWindow.h"


namespace ed {
class ImageEditorTest : public AssetEditorWindowTemplate<Image> {
public:
	ImageEditorTest(PodEntry* inEntry)
		: AssetEditorWindowTemplate(inEntry)
	{
	}

	void ImguiDraw() override
	{
		ImGui::Text(entry->path.c_str());
		if (ImEd::Button("FLIP")) {
			PodEditor<Image> ed(podHandle);
			std::reverse(ed->data.begin(), ed->data.end());
		}
	}
};


void RegisterWindows(ed::ComponentWindows& windowsComponent)
{
	windowsComponent.AddWindowEntry<OutlinerWindow>("Outliner");


	windowsComponent.AddWindowEntry<PropertyEditorWindow>("Property Editor");

	windowsComponent.AddWindowEntry<AssetsWindow>("Asset Browser");
	windowsComponent.AddWindowEntry<AssetListWindow>("Asset List");

	windowsComponent.AddWindowEntry<ConsoleWindow>("Console");

	windowsComponent.AddWindowEntry<AttachmentDebuggerWindow>("Attachment Debugger");


	windowsComponent.AddWindowEntry<ProfilerWindow>("Profiler");

	windowsComponent.AddWindowEntry<ImGuiDemoWindow>("ImGui Demo");


	windowsComponent.AddWindowEntry<PodEntryEditorWindow>("Entry Editor");
	windowsComponent.AddWindowEntry<MeshGenerator>("Mesh Generator");


	windowsComponent.RegisterAssetWindowEditor<ImageEditorTest>();
	windowsComponent.RegisterAssetWindowEditor<ShaderStageEditorWindow>();
	windowsComponent.RegisterAssetWindowEditor<ShaderHeaderEditorWindow>();
	windowsComponent.RegisterAssetWindowEditor<MaterialArchetypeEditorWindow>();
	windowsComponent.RegisterAssetWindowEditor<MaterialInstanceEditorWindow>();
}
} // namespace ed
