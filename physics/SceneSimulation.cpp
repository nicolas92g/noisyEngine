#include "SceneSimulation.h"

#include <thread>

physx::PxDefaultErrorCallback ns::SceneSimulation::defaultErrorCallback;
physx::PxDefaultAllocator ns::SceneSimulation::defaultAllocatorCallback;

physx::PxFoundation* ns::SceneSimulation::foundation;

ns::SceneSimulation::SceneSimulation()
{
	using namespace physx;

	//check that the static foundation object was created else create it
	if (!foundation) {
		foundation = PxCreateFoundation(physXVersion, defaultAllocatorCallback, defaultErrorCallback);
		_STL_ASSERT(foundation, "failed to create PxFoundation");
	}

	//create the custom physics object
	physics_ = PxCreatePhysics(physXVersion, *foundation, PxTolerancesScale());
	_STL_ASSERT(physics_, "failed to create PxPhysics !");

	//create the scene description
	PxSceneDesc sceneDesc(physics_->getTolerancesScale());
	sceneDesc.gravity = { 0.f, -9.81f, 0.f };
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());
	_STL_ASSERT(sceneDesc.cpuDispatcher, "failed to create the CpuDispatcher");

	sceneDesc.filterShader = &PxDefaultSimulationFilterShader;

	scene_ = physics_->createScene(sceneDesc);

}
