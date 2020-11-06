#include "PodImporter.h"

#include "assets/PodEntry.h"


void PodImporterBase::Reimport(PodEntry* intoEntry, const uri::Uri& uri)
{
	LOG_ERROR("Attempting to reimport asset: {} of type {}. No reimport has been implemented for this importer.",
		intoEntry->path, intoEntry->GetClass()->GetName());
}
