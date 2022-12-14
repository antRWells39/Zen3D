#pragma once
#include <ctype.h>

#include "PxPhysicsAPI.h"
#include "Physics.h"

#include "Common/interface/BasicMath.hpp"

using namespace Diligent;


	class PhysicsBody
	{
	public:

		virtual void InitBody() {};

		float3 GetPosition();
		float4x4 GetRotation();
		void SetPosition(float3 pos);
		void SetConstraint(bool x, bool y, bool z);
		void Turn(float x, float y, float z);
		void SetAngularForce(float x, float y, float z);
		void ApplyForce(float x, float y, float z);
		void ApplyLocalForce(float x, float y, float z);
		void SetForce(float x, float y, float z);
		void SetLocalForce(float x, float y, float z);
		void AddLocalForce(float x, float y, float z, float dampen);
		float3 GetForce();
		float GetRotationY();
	protected:
		physx::PxRigidDynamic* body;
		physx::PxRigidStatic* sbody;
		physx::PxShape* shape;
		physx::PxMaterial* material;
		float w, h, d;
	};
