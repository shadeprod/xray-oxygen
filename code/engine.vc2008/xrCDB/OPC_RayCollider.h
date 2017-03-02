///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a ray collider.
 *	\file		OPC_RayCollider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_RAYCOLLIDER_H__
#define __OPC_RAYCOLLIDER_H__

	class OPCODE_API RayCollider : public Collider
	{
		public:
		// Constructor / Destructor
											RayCollider();
		virtual								~RayCollider();
		// Generic collision query

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Generic stabbing query for generic OPCODE models. After the call, access the results:
		 *	- with GetContactStatus()
		 *	- in the user-provided destination array
		 *
		 *	\param		world_ray		[in] stabbing ray in world space
		 *	\param		model			[in] Opcode model to collide with
		 *	\param		world			[in] model's world matrix, or null
		 *	\param		cache			[in] a possibly cached face index, or null
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							bool			Collide(const Ray& world_ray, OPCODE_Model* model, const Matrix4x4* world=null, udword* cache=null);

		// Collision queries
							bool			Collide(const Ray& world_ray, const AABBCollisionTree* tree,		const Matrix4x4* world=null, udword* cache=null);
							bool			Collide(const Ray& world_ray, const AABBNoLeafTree* tree,			const Matrix4x4* world=null, udword* cache=null);
							bool			Collide(const Ray& world_ray, const AABBQuantizedTree* tree,		const Matrix4x4* world=null, udword* cache=null);
							bool			Collide(const Ray& world_ray, const AABBQuantizedNoLeafTree* tree,	const Matrix4x4* world=null, udword* cache=null);
							bool			Collide(const Ray& world_ray, const AABBTree* tree, Container& box_indices);
		// Settings

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Settings: enable or disable "closest hit" mode.
		 *	\param		flag		[in] true to report closest hit only
		 *	\see		SetCulling(bool flag)
		 *	\see		SetMaxDist(float maxdist)
		 *	\see		SetDestination(StabbedFaces* sf)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			SetClosestHit(bool flag)				{ mClosestHit	= flag;		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Settings: enable or disable backface culling.
		 *	\param		flag		[in] true to enable backface culling
		 *	\see		SetClosestHit(bool flag)
		 *	\see		SetMaxDist(float maxdist)
		 *	\see		SetDestination(StabbedFaces* sf)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			SetCulling(bool flag)					{ mCulling		= flag;		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Settings: sets the higher distance bound.
		 *	\param		maxdist		[in] higher distance bound. Default = maximal value, for ray queries (else segment)
		 *	\see		SetClosestHit(bool flag)
		 *	\see		SetCulling(bool flag)
		 *	\see		SetDestination(StabbedFaces* sf)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			SetMaxDist(float maxdist=MAX_FLOAT)		{ mMaxDist		= maxdist;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Settings: sets the destination array for stabbed faces.
		 *	\param		cf			[in] destination array, filled during queries
		 *	\see		SetClosestHit(bool flag)
		 *	\see		SetCulling(bool flag)
		 *	\see		SetMaxDist(float maxdist)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			SetDestination(CollisionFaces* cf)		{ mStabbedFaces	= cf;		}

		// Stats
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stats: gets the number of Ray-BV overlap tests after a collision query.
		 *	\see		GetNbRayPrimTests()
		 *	\see		GetNbIntersections()
		 *	\return		the number of Ray-BV tests performed during last query
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				udword			GetNbRayBVTests()				const	{ return mNbRayBVTests;		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stats: gets the number of Ray-Triangle overlap tests after a collision query.
		 *	\see		GetNbRayBVTests()
		 *	\see		GetNbIntersections()
		 *	\return		the number of Ray-Triangle tests performed during last query
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				udword			GetNbRayPrimTests()				const	{ return mNbRayPrimTests;	}

		// In-out test
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stats: gets the number of intersection found after a collision query. Can be used for in/out tests.
		 *	\see		GetNbRayBVTests()
		 *	\see		GetNbRayPrimTests()
		 *	\return		the number of valid intersections during last query
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				udword			GetNbIntersections()			const	{ return mNbIntersections;	}

#ifdef OPC_USE_CALLBACKS
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Callback control: a method to setup object callback. Must provide triangle-vertices for a given triangle index.
		 *	\param		callback	[in] user-defined callback
		 *	\param		data		[in] user-defined data
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			SetCallback(OPC_CALLBACK callback, udword data)			{ mObjCallback = callback;	mUserData = data;	}
#else
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Pointers control: a method to setup object pointers. Must provide access to faces and vertices for a given object.
		 *	\param		faces	[in] pointer to faces
		 *	\param		verts	[in] pointer to vertices
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_				void			SetPointers(const IndexedTriangle* faces, const Point* verts)	{ mFaces = faces;	mVerts = verts;				}
#endif
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
		 *	\return		null if everything is ok, else a string describing the problem
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Collider)	const char*		ValidateSettings();

		protected:
		// Ray in local space
							Point			mOrigin;			//!< Ray origin
							Point			mDir;				//!< Ray direction (normalized)
							Point			mFDir;				//!< fabsf(mDir)
							Point			mData, mData2;
		// Stabbed faces
							CollisionFace	mStabbedFace;		//!< Current stabbed face
							CollisionFaces*	mStabbedFaces;		//!< List of stabbed faces
#ifdef OPC_USE_CALLBACKS
		// User callback
							udword			mUserData;			//!< User-defined data sent to callback
							OPC_CALLBACK	mObjCallback;		//!< Object callback
#else
		// User pointers
				const	IndexedTriangle*	mFaces;				//!< User-defined faces
				const		Point*			mVerts;				//!< User-defined vertices
#endif
		// Stats
							udword			mNbRayBVTests;		//!< Number of Ray-BV tests
							udword			mNbRayPrimTests;	//!< Number of Ray-Primitive tests
		// In-out test
							udword			mNbIntersections;	//!< Number of valid intersections
		// Dequantization coeffs
							Point			mCenterCoeff;
							Point			mExtentsCoeff;
		// Settings
							float			mMaxDist;			//!< Valid segment on the ray
							bool			mClosestHit;		//!< Report closest hit only
							bool			mCulling;			//!< Stab culled faces or not
		// Internal methods
							void			_Stab(const AABBCollisionNode* node);
							void			_Stab(const AABBNoLeafNode* node);
							void			_Stab(const AABBQuantizedNode* node);
							void			_Stab(const AABBQuantizedNoLeafNode* node);
							void			_Stab(const AABBTreeNode* node, Container& box_indices);
							void			_UnboundedStab(const AABBCollisionNode* node);
							void			_UnboundedStab(const AABBNoLeafNode* node);
							void			_UnboundedStab(const AABBQuantizedNode* node);
							void			_UnboundedStab(const AABBQuantizedNoLeafNode* node);
							void			_UnboundedStab(const AABBTreeNode* node, Container& box_indices);
			// Overlap tests
		inline_				BOOL			RayAABBOverlap(const Point& center, const Point& extents);
		inline_				BOOL			SegmentAABBOverlap(const Point& center, const Point& extents);
		inline_				BOOL			RayTriOverlap(const Point& vert0, const Point& vert1, const Point& vert2);
			// Init methods
							BOOL			InitQuery(const Ray& world_ray, const Matrix4x4* world=null, udword* faceid=null);
	};

#endif // __OPC_RAYCOLLIDER_H__
