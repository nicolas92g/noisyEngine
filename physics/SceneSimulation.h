#pragma once

#include <PxPhysicsAPI.h>

namespace ns {
	class SceneSimulation
	{
	public:
		SceneSimulation();
	protected:

		static physx::PxFoundation* foundation;
		physx::PxPhysics* physics_;
		physx::PxScene* scene_;


	protected:
		static physx::PxDefaultErrorCallback defaultErrorCallback;
		static physx::PxDefaultAllocator defaultAllocatorCallback;
		static constexpr physx::PxU32 physXVersion = PX_PHYSICS_VERSION;
	};
}

