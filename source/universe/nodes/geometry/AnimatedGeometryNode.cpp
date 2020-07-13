#include "pch.h"
#include "AnimatedGeometryNode.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuSkinnedMesh.h"
#include "editor/Editor.h"

#include "engine/Input.h"
#include <glm/gtx/matrix_decompose.hpp>

AnimatedGeometryNode::AnimatedGeometryNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneAnimatedGeometry>();
}

void AnimatedGeometryNode::Update(float deltaSeconds)
{
	UpdateAnimation(deltaSeconds);
	SetDirty(DF::Joints);
}

void AnimatedGeometryNode::DirtyUpdate(DirtyFlagset dirtyFlags)
{
	Node::DirtyUpdate(dirtyFlags);

	if (dirtyFlags[DF::ModelChange]) {
		CalculateWorldAABB();

		Enqueue([model = m_skinnedMesh, anim = m_animation, size = m_skinnedMesh.Lock()->joints.size()](
					SceneAnimatedGeometry& geom) {
			geom.modelPod = model;
			geom.model = vl::GpuAssetManager->GetGpuHandle(model);
			geom.isDirtyResize = { true, true, true };
			geom.jointMatrices.resize(size);
		});

		m_joints.resize(m_skinnedMesh.Lock()->joints.size());
		m_animation = PodHandle<Animation>{};
	}

	if (dirtyFlags[DF::AnimationChange]) {
		if (!m_animation.IsDefault()) {
			if (m_joints.size() != m_animation.Lock()->jointCount) {
				LOG_WARN("Incompatible skin with animation.");
				m_animation = PodHandle<Animation>{};
			}
			else {
				m_animationTime = 0;
			}
			UpdateAnimation(0);
			Enqueue([joints = m_joints](SceneAnimatedGeometry& geom) { geom.jointMatrices = joints; });
		}
	}

	if (dirtyFlags[DF::SRT]) {
		Enqueue([trans = GetNodeTransformWCS()](SceneAnimatedGeometry& geom) { geom.transform = trans; });
	}

	// HACK: use dirty srt (node is currently selected always updates srt due to a bug)
	if (!Editor::ShouldUpdateWorld() && dirtyFlags[DF::SRT] && m_playInEditor) {
		UpdateAnimation(MainWorld->GetDeltaTime());
		Enqueue([joints = m_joints](SceneAnimatedGeometry& geom) { geom.jointMatrices = joints; });
	}

	if (dirtyFlags[DF::Joints] && !dirtyFlags[DF::ModelChange]) {
		UpdateAnimation(0);
		Enqueue([joints = m_joints](SceneAnimatedGeometry& geom) { geom.jointMatrices = joints; });
	}
}

void AnimatedGeometryNode::UpdateAnimation(float deltaTime)
{
	if (m_animation.IsDefault() || m_skinnedMesh.IsDefault()) {
		return;
	}
	auto pod = m_skinnedMesh.Lock();


	auto animationTransform = TickSamplers(deltaTime);

	std::vector<glm::mat4> globalJointMatrix;
	globalJointMatrix.resize(pod->joints.size(), glm::identity<glm::mat4>());


	globalJointMatrix[0] = animationTransform[0];
	for (size_t i = 1; i < pod->joints.size(); ++i) {
		globalJointMatrix[i] = globalJointMatrix[pod->joints[i].parentJoint] * animationTransform[i];
	}


	for (size_t i = 0; i < globalJointMatrix.size(); i++) {
		globalJointMatrix[i] = globalJointMatrix[i] * pod->joints[i].inverseBindMatrix;
	}

	// PERF: Use joints everywhere
	m_joints = globalJointMatrix;
}

std::vector<glm::mat4> AnimatedGeometryNode::TickSamplers(float deltaTime)
{
	m_animationTime += deltaTime * m_playbackSpeed;
	auto totalAnimTime = m_animation.Lock()->time;
	if (m_animationTime > totalAnimTime) {
		m_animationTime -= totalAnimTime;
	}
	else if (m_animationTime < 0) {
		m_animationTime += totalAnimTime;
	}

	auto mesh = m_skinnedMesh.Lock();

	auto anim = m_animation.Lock();

	std::vector<glm::vec3> transforms(mesh->joints.size());
	std::vector<glm::quat> rotations(mesh->joints.size(), glm::identity<glm::quat>());
	std::vector<glm::vec3> scales(mesh->joints.size(), glm::vec3(1.f, 1.f, 1.f));


	for (int32 i = 0; i < mesh->joints.size(); ++i) {
		glm::vec3 skew;
		glm::vec4 persp;
		glm::vec3 t;
		glm::quat r;
		glm::vec3 s;

		glm::decompose(mesh->joints[i].localTransform, s, r, t, skew, persp);


		transforms[i] = t;
		rotations[i] = r;
		scales[i] = s;
	}

	// TODO: BUG:
	// This logic is incorrect when there is no inputs for a timestrip in a particular sampler. (can happen at the
	// ending of the animation) When this happens what should be sampled is the last keyframe (instead of the initial
	// position)


	for (auto& channel : anim->channels) {
		const AnimationSampler& sampler = anim->samplers[channel.samplerIndex];
		int32 jointIndex = channel.targetJoint; // WIP:


		for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
			// Get the input keyframe values for the current time stamp
			if ((m_animationTime >= sampler.inputs[i]) && (m_animationTime <= sampler.inputs[i + 1])) {

				float a = (m_animationTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				switch (channel.path) {
					case AnimationPath::Translation:
						transforms[jointIndex]
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
		localMatrix[i] = math::transformMat(scales[i], rotations[i], transforms[i]);
	}

	return localMatrix;
}

AnimatedGeometryNode::~AnimatedGeometryNode()
{
	Scene->EnqueueDestroyCmd<SceneAnimatedGeometry>(sceneUid);
}
