#include "pch.h"
#include "OptixProgram.h"


namespace Renderer::Optix
{
	OptixProgram::OptixProgram(OptixRendererBase* renderer)
		: OptixAsset(renderer)
	{
	}

	bool OptixProgram::Load(Assets::StringFile* sourceFile, const std::string& programName)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(sourceFile->GetLabel());

		m_name = programName;

		const std::string ptxString = sourceFile->GetFileData();

		m_handle = GetOptixContext()->createProgramFromPTXString(ptxString, m_name);

		return true;
	}
}
