#ifndef _World_h
#define _World_h

#include <stdlib.h>

#ifdef MAC
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <GL/gle.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/gle.h>
#endif

#include <iostream>
#include <fstream>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <math.h>
#include <vector>
#include <typeinfo>

#include "../utils/drawUtils.h"
#include "../Collisions/intersectionStructs.h"
#include "../Collisions/collisionUtils.h"

using namespace std;
USING_PART_OF_NAMESPACE_EIGEN

enum object_type {NO_OBJECT, THREAD_CONSTRAINED, CAPSULE, CURSOR, END_EFFECTOR, INFINITE_PLANE, TEXTURED_SPHERE};

class EnvObject;

class ThreadConstrained;

class ControlBase;

class World
{
	public:
		World();
		World(const World& rhs);
    World& operator=(const World& rhs);
		~World();

		void addThread(ThreadConstrained* thread);
		void addEnvObj(EnvObject* obj);
		vector<EnvObject*>* getEnvObjs();
		vector<EnvObject*> getEnvObjs(object_type type);
		void getEnvObjs(vector<EnvObject*>& objects, object_type type);
		vector<ThreadConstrained*>* getThreads();
		void initializeThreadsInEnvironment();
		void clearObjs();

		//applies controli to cursori
		void applyControl(const vector<ControlBase*>& controls);
		
		void draw();
		bool capsuleObjectIntersection(int capsule_ind, const Vector3d& start, const Vector3d& end, const double radius, vector<Intersection>& intersections);
		double capsuleObjectRepulsionEnergy(const Vector3d& start, const Vector3d& end, const double radius);
		void capsuleObjectRepulsionEnergyGradient(const Vector3d& start, const Vector3d& end, const double radius, Vector3d& gradient);
		
		void getStates(vector<VectorXd>& states);
		
		void saveToBackup();
		void restoreFromBackup();
	
	private:
		vector<ThreadConstrained*> threads;
		vector<EnvObject*> objs;
};

#endif
