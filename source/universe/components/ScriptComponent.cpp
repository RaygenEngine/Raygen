#include "ScriptComponent.h"

#include "engine/Input.h"

void CScript::BeginPlay()
{
	LOG_REPORT("Begin Play: {}", self->name);
}

void CScript::EndPlay()
{
	LOG_REPORT("End Play");
}

void CScript::Tick(float deltaSeconds)
{
	if (Input.IsDown(Key::Shift)) {
		LOG_REPORT("DeltaTime: {}", deltaSeconds);
	}
}

static_assert(componentdetail::CBeginPlayComp<CScript>);
static_assert(componentdetail::CEndPlayComp<CScript>);
static_assert(componentdetail::CTickableComp<CScript>);
static_assert(componentdetail::CSelfEntityMember<CScript>);


static_assert(componentdetail::CCreateDestoryComp<CScript>);
