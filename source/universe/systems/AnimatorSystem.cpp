#include "AnimatorSystem.h"

#include "assets/pods/Animation.h"
#include "assets/pods/SkinnedMesh.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/scene/SceneGeometry.h"
#include "universe/components/SkinnedMeshComponent.h"

namespace {
std::vector<glm::mat4> TickSamplers(CSkinnedMesh& sm, float deltaSeconds)
{
	sm.animationTime += deltaSeconds * sm.playbackSpeed;
	auto totalAnimTime = sm.animation.Lock()->time;
	if (sm.animationTime > totalAnimTime) {
		sm.animationTime -= totalAnimTime;
	}
	else if (sm.animationTime < 0) {
		sm.animationTime += totalAnimTime;
	}

	auto mesh = sm.skinnedMesh.Lock();

	auto anim = sm.animation.Lock();

	std::vector<glm::vec3> translations(mesh->joints.size());
	std::vector<glm::quat> rotations(mesh->joints.size(), glm::identity<glm::quat>());
	std::vector<glm::vec3> scales(mesh->joints.size(), glm::vec3(1.f, 1.f, 1.f));


	for (int32 i = 0; i < mesh->joints.size(); ++i) {
		translations[i] = mesh->joints[i].translation;
		rotations[i] = mesh->joints[i].rotation;
		scales[i] = mesh->joints[i].scale;
	}

	// CHECK: BUG:
	// This logic is incorrect when there is no inputs for a timestrip in a particular sampler. (can happen at the
	// ending of the animation) When this happens what should be sampled is the last keyframe (instead of the initial
	// position)

	// Gltf Spec:
	//	The inputs of each sampler are relative to t=0, defined as the beginning of the parent animations entry. Before
	// and after the provided input range, output should be "clamped" to the nearest end of the input range. For
	// example, if the earliest sampler input for an animation is t=10, a client implementation should begin playback of
	// that animation at t=0 with output clamped to the first output value. Samplers within a given animation are not
	// required to have the same inputs.

	for (auto& channel : anim->channels) {
		const AnimationSampler& sampler = anim->samplers[channel.samplerIndex];
		int32 jointIndex = channel.targetJoint;


		for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
			// Get the input keyframe values for the current time stamp
			if ((sm.animationTime >= sampler.inputs[i]) && (sm.animationTime <= sampler.inputs[i + 1])) {

				float a = (sm.animationTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				switch (channel.path) {
					case AnimationPath::Translation:
						translations[jointIndex]
							= glm::mix(sampler.GetOutputAsVec3(i), sampler.GetOutputAsVec3(i + 1), a);

						break;
					case AnimationPath::Rotation:
						glm::quat q1 = sampler.GetOutputAsQuat(i);
						glm::quat q2 = sampler.GetOutputAsQuat(i + 1);
						rotations[jointIndex] = glm::normalize(glm::slerp(q1, q2, a));
						break;
					case AnimationPath::Scale:
						scales[jointIndex] = glm::mix(sampler.GetOutputAsVec3(i), sampler.GetOutputAsVec3(i + 1), a);

						break;
					case AnimationPath::Weights: break;
				}
			}
		}
	}
	std::vector<glm::mat4> localMatrix(mesh->joints.size(), glm::identity<glm::mat4>());


	for (size_t i = 0; i < localMatrix.size(); i++) {
		localMatrix[i] = math::transformMat(scales[i], rotations[i], translations[i]);
	}

	return localMatrix;
}


void UpdateComponent(CSkinnedMesh& sm, float deltaSeconds)
{
	if (sm.animation.IsDefault() || sm.skinnedMesh.IsDefault()) {
		return;
	}

	auto pod = sm.skinnedMesh.Lock();

	auto animationTransform = TickSamplers(sm, deltaSeconds);

	sm.joints[0] = animationTransform[0];
	for (size_t i = 1; i < pod->joints.size(); ++i) {
		// Parent here is guaranteed to be evaluated because the joints array is sorted properly in top down tree form
		// during importing
		sm.joints[i] = sm.joints[pod->joints[i].parentJoint] * animationTransform[i];
	}

	// Seperate loop required here because the above loop has dependency on itself
	for (size_t i = 0; i < sm.joints.size(); i++) {
		sm.joints[i] = sm.joints[i] * pod->joints[i].inverseBindMatrix;
	}


	//
}
} // namespace

void AnimatorSystem::UpdateAnimations(entt::registry& reg, float deltaSeconds)
{
	PROFILE_SCOPE(World);

	auto view = reg.view<CSkinnedMesh, CSkinnedMesh::Dirty>();
	for (auto& [ent, sm] : view.each()) {
		auto size = sm.skinnedMesh.Lock()->joints.size();
		if (size != sm.joints.size()) {
			sm.joints.resize(sm.skinnedMesh.Lock()->joints.size());
		}
	}

	auto view2 = reg.view<CSkinnedMesh>();
	for (auto& [ent, sm] : view2.each()) {
		UpdateComponent(sm, deltaSeconds);
	}
}

void AnimatorSystem::UploadAnimationsToScene(entt::registry& reg, Scene& scene)
{
	auto view = reg.view<CSkinnedMesh>();
	for (auto& [ent, sm] : view.each()) {

		scene.EnqueueCmd<CSkinnedMesh::RenderSceneType>(
			sm.sceneUid, [joints = sm.joints](SceneAnimatedGeometry& geom) { geom.jointMatrices = joints; });
	}
}
