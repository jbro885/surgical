#ifndef _gl_thread_h
#define _gl_thread_h

#include "thread_discrete.h"

#ifdef MAC
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <GL/gle.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/gle.h>
#endif

#include <string.h>


using namespace std;

//CONTOUR STUFF
#define SCALE 1.0
#define CONTOUR(x,y) {                          \
    double ax, ay, alen;                        \
    contour[i][0] = SCALE * (x);                \
    contour[i][1] = SCALE * (y);                \
    if (i!=0) {                                 \
      ax = contour[i][0] - contour[i-1][0];     \
      ay = contour[i][1] - contour[i-1][1];     \
      alen = 1.0 / sqrt (ax*ax + ay*ay);        \
      ax *= alen;   ay *= alen;                 \
      contour_norms [i-1][0] = ay;              \
      contour_norms [i-1][1] = -ax;             \
    }                                           \
    i++;                                        \
  }

#define NUM_PTS_CONTOUR (25)

class GLThread {
 public:
  GLThread();
  GLThread(Thread* _thread);
  //void GetConfiguration(int* size, double* pts_cpy[][], double* twist_cpy[]);
  void DrawThread();
  void DrawAxes();
  void DrawName();
  void minimize_energy();
  void updateThreadPoints();
  void ApplyUserInput(float move_end[], float tangent_end[], float tangent_rotation_end[]);
  void ApplyUserInput(float move_end[], float tangent_end[], float tangent_rotation_end[], float move_start[], float tangent_start[], float tangent_rotation_start[]);
  void InitContour();
  void set_end_constraint(Vector3d pos, Matrix3d rot);

  Thread* getThread() { return _thread; }
  void setThread(Thread* thread) {
    delete _thread;
    _thread = thread;
#ifdef ISOTROPIC
    _thread->set_coeffs_normalized(to_set_bend, to_set_twist, to_set_grav);
#else
    _thread->set_coeffs_normalized(to_set_B, to_set_twist, to_set_grav);
#endif
  }

  void setThreadCoeffs()
  {
#ifdef ISOTROPIC
    _thread->set_coeffs_normalized(to_set_bend, to_set_twist, to_set_grav);
#else
    _thread->set_coeffs_normalized(to_set_B, to_set_twist, to_set_grav);
#endif
  }

  void setName(const char* name)
  {
    strcpy(_display_name, name);
  }

  void printThreadData();

  //Vector3d getEndPosition() { return positions[1]; }
  const Vector3d& getEndPosition() { return positions[1]; }
  const Vector3d& getStartPosition() { return positions[0]; }
  Vector3d getEndTangent()  { return tangents[1]; }
  Matrix3d getEndRotation() { return rotations[1]; }
  Matrix3d getStartRotation() { return rotations[0]; }


#ifdef ISOTROPIC
  double to_set_bend;
#else
  Matrix2d to_set_B;
#endif
  double to_set_twist;
  double to_set_grav;



// protected:
  Thread* _thread;
  char _display_name[256];

  Vector3d positions[2];
  Vector3d tangents[2];
  Matrix3d rotations[3];

  vector<Vector3d> points;
  vector<double> twist_angles;

  double contour[NUM_PTS_CONTOUR][2];
  double contour_norms[NUM_PTS_CONTOUR][2];

  Vector3d display_start_pos;

};

#endif
