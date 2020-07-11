#include "pch.h"
#include "AnimatedGeometryNode.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuSkinnedMesh.h"

#include "engine/Input.h"
#include <glm/gtx/matrix_decompose.hpp>

AnimatedGeometryNode::AnimatedGeometryNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneAnimatedGeometry>();
}

void AnimatedGeometryNode::Update(float deltaSeconds)
{
	if (!m_animation.IsDefault() && !m_skinnedMesh.IsDefault(), Input.IsJustPressed(Key::P)) {
		UpdateAnimation(0.1f);
		SetDirty(DF::Joints);
	}

	if (!m_animation.IsDefault() && !m_skinnedMesh.IsDefault(), Input.IsDown(Key::L)) {
		UpdateAnimation(deltaSeconds);
		SetDirty(DF::Joints);
	}


	if (Input.IsDown(Key::O)) {
		m_animationTime = 0;
	}
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
	}

	if (dirtyFlags[DF::SRT]) {
		Enqueue([trans = GetNodeTransformWCS()](SceneAnimatedGeometry& geom) { geom.transform = trans; });
	}

	if (dirtyFlags[DF::Joints] && !dirtyFlags[DF::ModelChange]) {
		Enqueue([joints = m_joints](SceneAnimatedGeometry& geom) { geom.jointMatrices = joints; });
	}
}

void AnimatedGeometryNode::UpdateAnimation(float deltaTime)
{
	auto pod = m_skinnedMesh.Lock();

	auto animationTransform = TickSamplers(deltaTime);

	std::vector<glm::mat4> globalJointMatrix;
	globalJointMatrix.resize(pod->joints.size(), glm::identity<glm::mat4>());


	for (int32 i = 0; auto& joint : pod->joints) {
		if (joint.parentJoint == UINT32_MAX)
			[[unlikely]]
			{
				globalJointMatrix[i] = animationTransform[i];
				++i;
				continue;
			}


		globalJointMatrix[i] = globalJointMatrix[joint.parentJoint] * animationTransform[i];
		++i;
	}


	for (size_t i = 0; i < globalJointMatrix.size(); i++) {
		globalJointMatrix[i] = globalJointMatrix[i] * pod->joints[i].inverseBindMatrix;
	}

	// PERF: Use joints everywhere
	m_joints = globalJointMatrix;
}

std::vector<glm::mat4> AnimatedGeometryNode::TickSamplers(float deltaTime)
{
	m_animationTime += deltaTime;


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


	for (auto& channel : anim->channels) {
		const AnimationSampler& sampler = anim->samplers[channel.samplerIndex];
		int32 jointIndex = channel.targetNode - 2; // WIP:


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
