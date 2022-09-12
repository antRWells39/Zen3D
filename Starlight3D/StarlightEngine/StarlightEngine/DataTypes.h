#pragma once
#include "Common/interface/BasicMath.hpp"

using namespace Diligent;
#define MAX_BONES  4




struct Vertex {
	float3 position;



	/// <summary>
	/// The color of the vertex.
	/// </summary>
	float4 color;

	/// <summary>
/// The texture coord used.
/// </summary>
	float3 texture_coord;

	/// <summary>
	/// the 3D normal of the vertex.
	/// </summary>
	float3 normal;

	/// <summary>
	/// the Bi-Normal of the vertex.
	/// </summary>
	float3 bi_normal;

	/// <summary>
	/// The tangent of the vertex.
	/// </summary>
	float3 tangent;

	};
	/// <summary>
	/// A vertex is a single point in a mesh or other vertex constructed entity.
	/// </summary>
	struct VertexActor
	{

		/// <summary>
		/// The position of the vertex in 3D space.
		/// </summary>
		float3 position;



		/// <summary>
		/// The color of the vertex.
		/// </summary>
		float4 color;

		/// <summary>
/// The texture coord used.
/// </summary>
		float3 texture_coord;

		/// <summary>
		/// the 3D normal of the vertex.
		/// </summary>
		float3 normal;

		/// <summary>
		/// the Bi-Normal of the vertex.
		/// </summary>
		float3 bi_normal;

		/// <summary>
		/// The tangent of the vertex.
		/// </summary>
		float3 tangent;

		/// <summary>
		/// The bone index of the vertex.
		/// </summary>
		//int bone_index = 0;

		int m_BoneIDS[MAX_BONES];
		float m_Weights[MAX_BONES];


	};

	/// <summary>
	/// A triangle consists of three indices of verties to form the triangle.
	/// </summary>
	struct Tri {

		uint v0;
		uint v1;
		uint v2;

	};

	struct GeoIndex {

		uint start_tri;
		float4x4 g_Model;
		float4x4 g_ModelInv;

	};