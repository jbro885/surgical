#include "ThreadConstrained.h"

ThreadConstrained::ThreadConstrained(int num_vertices_init) {
	num_vertices = num_vertices_init;
	if (num_vertices < 5)
		cout << "Internal error: ThreadConstrained: too few number of vertices";
	int numInit = (num_vertices-3)/2;
	double noise_factor = 0.0;
	vector<Vector3d> vertices;
	vector<double> angles;
	Vector3d direction;

	vertices.push_back(Vector3d::Zero());
	angles.push_back(0.0);
	//push back unitx so first tangent matches start_frame
	vertices.push_back(Vector3d::UnitX()*DEFAULT_REST_LENGTH);
	angles.push_back(0.0);
	//specify direction
	direction(0) = 1.0;
	direction(1) = 0.0;
	direction(2) = -2.0;
	direction.normalize();
	for (int i=0; i < numInit; i++) {
		Vector3d noise( ((double)(rand()%10000)) / 10000.0, ((double)(rand()%10000)) / 10000.0, ((double)(rand()%10000)) / 10000.0);
		noise *= noise_factor;
		Vector3d next_Vec = vertices.back()+(direction+noise).normalized()*DEFAULT_REST_LENGTH;
		vertices.push_back(next_Vec);
		angles.push_back(0.0);
	}
	//change direction
	direction(0) = 1.0;
	direction(1) = 0.0;
	direction(2) = 2.0;
	direction.normalize();
	for (int i=0; i < numInit; i++)	{
		Vector3d noise( ((double)(rand()%10000)) / 10000.0, ((double)(rand()%10000)) / 10000.0, ((double)(rand()%10000)) / 10000.0);
		noise *= noise_factor;
		Vector3d next_Vec = vertices.back()+(direction+noise).normalized()*DEFAULT_REST_LENGTH;
		vertices.push_back(next_Vec);
		angles.push_back(0.0);
	}
	//push back unitx so last tangent matches end_frame
	for (int i =0; i < (num_vertices - numInit*2 - 2); i++) {
		vertices.push_back(vertices.back()+Vector3d::UnitX()*DEFAULT_REST_LENGTH);
		angles.push_back(0.0);
	}
	angles.resize(vertices.size());

  vector<Matrix3d> rotations;
	rotations.push_back(Matrix3d::Identity());
	rotations.push_back(Matrix3d::Identity());

	Thread* thread = new Thread(vertices, angles, rotations[0], rotations[1]);
	threads.push_back(thread);

	thread->minimize_energy();

	zero_angle.push_back(0.0);
	rotation_difference.push_back(Matrix3d::Identity());
	constrained_vertices_nums.push_back(0);
	constrained_vertices_nums.push_back(num_vertices_init-1);
}

void ThreadConstrained::get_thread_data(vector<Vector3d> &absolute_points) {
	vector<vector<Vector3d> > points(threads.size());   //check
	for (int thread_num=0; thread_num<threads.size(); thread_num++)
		threads[thread_num]->get_thread_data(points[thread_num]);
	mergeMultipleVector(absolute_points, points);
}

void ThreadConstrained::get_thread_data(vector<Vector3d> &absolute_points, vector<double> &absolute_twist_angles) {
	vector<vector<Vector3d> > points(threads.size());
	vector<vector<double> > twist_angles(threads.size());
	for (int thread_num=0; thread_num<threads.size(); thread_num++) {
		threads[thread_num]->get_thread_data(points[thread_num], twist_angles[thread_num]);
		twist_angles[thread_num].back() = 2.0*twist_angles[thread_num][twist_angles[thread_num].size()-2] - twist_angles[thread_num][twist_angles[thread_num].size()-3];
 		vector<double> vd = twist_angles[thread_num];
		/*cout << "twist_angles[" << thread_num << "] :" << vd.size() << endl;
	  for (int i=0; i<(vd.size()+4)/5; i++) {
	    for (int j=0; j<5 && (i*5+j)<vd.size(); j++) {
	      printf("%d:%3.4f\t",i*5+j,vd[i*5+j]);
	    }
	    printf("\n");
	  }*/
 	}
	mergeMultipleVector(absolute_points, points);
	/*	  
	for (int thread_num=0; thread_num<threads.size(); thread_num++) {
	vector<double> vd = twist_angles[thread_num];
	cout << "twist_angles: " << vd.size() << endl;
    for (int i=0; i<(vd.size()+4)/5; i++) {
      for (int j=0; j<5 && (i*5+j)<vd.size(); j++) {
        printf("%d:%3.4f\t",i*5+j,vd[i*5+j]);
      }
      printf("\n");
    }}
    */
  //for (int thread_num=0; thread_num<threads.size(); thread_num++)
    //for (int thread_num_inner=thread_num; thread_num_inner<threads.size(); thread_num_inner++)
		  //mapAdd(twist_angles[thread_num_inner], zero_angle[thread_num_inner]);
	for (int thread_num=0; thread_num<threads.size(); thread_num++) {
  	mapAdd(twist_angles[thread_num], zero_angle[thread_num]);
  	//cout << "zero_angle[" << thread_num << "]: " << zero_angle[thread_num] << endl;
  }
  /*cout << "after mapAdd: " << endl;
  for (int thread_num=0; thread_num<threads.size(); thread_num++) {
		vector<double> vd = twist_angles[thread_num];
		cout << "twist_angles[" << thread_num << "] :" << vd.size() << endl;
	  for (int i=0; i<(vd.size()+4)/5; i++) {
	    for (int j=0; j<5 && (i*5+j)<vd.size(); j++) {
	      printf("%d:%3.4f\t",i*5+j,vd[i*5+j]);
	    }
	    printf("\n");
	  }
 	}*/
	mergeMultipleVector(absolute_twist_angles, twist_angles);
	/*vector<double> vd = absolute_twist_angles;
	cout << "absolute_twist_angles: " << vd.size() << endl;
  for (int i=0; i<(vd.size()+4)/5; i++) {
    for (int j=0; j<5 && (i*5+j)<vd.size(); j++) {
      printf("%d:%3.4f\t",i*5+j,vd[i*5+j]);
    }
    printf("\n");
  }*/
	//cout << absolute_twist_angles[0] << "\t\t" << absolute_twist_angles[7] << "\t\t" << absolute_twist_angles[14] << "\t\t" ;
}

void ThreadConstrained::get_thread_data(vector<Vector3d> &absolute_points, vector<double> &absolute_twist_angles, vector<Matrix3d> &absolute_material_frames) {
	vector<vector<Vector3d> > points(threads.size());
	vector<vector<double> > twist_angles(threads.size());
	vector<vector<Matrix3d> > material_frames(threads.size());
	for (int thread_num=0; thread_num<threads.size(); thread_num++)
		threads[thread_num]->get_thread_data(points[thread_num], twist_angles[thread_num], material_frames[thread_num]);
	mergeMultipleVector(absolute_points, points);
	//for (int thread_num=0; thread_num<threads.size(); thread_num++)
    //for (int thread_num_inner=thread_num; thread_num_inner<threads.size(); thread_num_inner++)
		  //mapAdd(twist_angles[thread_num_inner], zero_angle[thread_num_inner]);
	for (int thread_num=0; thread_num<threads.size(); thread_num++)
  	mapAdd(twist_angles[thread_num], zero_angle[thread_num]);
  	
	mergeMultipleVector(absolute_twist_angles, twist_angles);
	absolute_twist_angles.back() = 2.0*absolute_twist_angles[absolute_twist_angles.size()-2] - absolute_twist_angles[absolute_twist_angles.size()-3];
	mergeMultipleVector(absolute_material_frames, material_frames);
}

// parameters have to be of the right size, i.e. threads.size()+1
void ThreadConstrained::getConstrainedTransforms(vector<Vector3d> &positions, vector<Matrix3d> &rotations, vector<Vector3d> &tangents) {
	int threads_size = threads.size();
	if (positions.size()!=(threads_size+1) || rotations.size()!=(threads_size+1) || tangents.size()!=(threads_size+1))
    cout << "Internal error: getConstrainedTransforms: at least one of the vector parameters is of the wrong size" << endl;
	int thread_num;
	vector<vector<Vector3d> > points(threads_size);	
	for (thread_num=0; thread_num<threads_size; thread_num++) {
		threads[thread_num]->get_thread_data(points[thread_num]);
		positions[thread_num] = (points[thread_num]).front();
		rotations[thread_num] = (threads[thread_num])->start_rot();
		tangents[thread_num] = points[thread_num][1] - points[thread_num][0];
		(tangents[thread_num]).normalize();
	}
	positions[thread_num] = (points[thread_num-1]).back();
	rotations[thread_num] = (threads[thread_num-1])->end_rot();
	tangents[thread_num] = points[thread_num-1][(points[thread_num-1]).size()-1] - points[thread_num-1][(points[thread_num-1]).size()-2];
	(tangents[thread_num]).normalize();
}

void ThreadConstrained::getConstrainedVerticesNums(vector<int> &vertices_nums) {
	vertices_nums = constrained_vertices_nums;
}

void ThreadConstrained::getConstrainedVertices(vector<Vector3d> &constrained_vertices) {
	vector<Vector3d> vertices;
	this->get_thread_data(vertices);
	constrained_vertices.clear();
	for (int i=0; i<constrained_vertices_nums.size(); i++) {
		constrained_vertices.push_back(vertices[constrained_vertices_nums[i]]);
	}
}

void ThreadConstrained::getFreeVerticesNums(vector<int> &free_vertices_nums) {
	free_vertices_nums.resize(num_vertices);
	for (int i=0; i<free_vertices_nums.size(); i++)
		free_vertices_nums[i] = i;
  vector<int> ind_vect(num_vertices);
  invalidateConstraintsNums(ind_vect, constrained_vertices_nums);
	for(int i=num_vertices-1; i>=0; i--)
		if (ind_vect[i]!=-1)
			free_vertices_nums.erase(free_vertices_nums.begin()+i);
}

void ThreadConstrained::getFreeVertices(vector<Vector3d> &free_vertices) {
  this->get_thread_data(free_vertices);
  vector<int> ind_vect(num_vertices);
  invalidateConstraintsNums(ind_vect, constrained_vertices_nums);
	for(int i=num_vertices-1; i>=0; i--)
		if (ind_vect[i]!=-1)
			free_vertices.erase(free_vertices.begin()+i);
}

void ThreadConstrained::getOperableVertices(vector<Vector3d> &operable_vertices, vector<int> &operable_vertices_nums, vector<bool> &constrained_or_free) {
	this->get_thread_data(operable_vertices);
	operable_vertices_nums.resize(num_vertices);
	constrained_or_free.resize(num_vertices);
	for(int i=0; i<num_vertices; i++) {
		operable_vertices_nums[i] = i;
		constrained_or_free[i] = false;
	}
	for(int i=0; i<constrained_vertices_nums.size(); i++)
		constrained_or_free[constrained_vertices_nums[i]] = true;
	vector<int> ind_vect(num_vertices);
	invalidateAroundConstraintsNums(ind_vect, constrained_vertices_nums);
	vector<int> vi = ind_vect;
	for(int i=num_vertices-1; i>=0; i--) {
		if (ind_vect[i]==-1) {
			operable_vertices.erase(operable_vertices.begin()+i);
			operable_vertices_nums.erase(operable_vertices_nums.begin()+i);
			constrained_or_free.erase(constrained_or_free.begin()+i);
		}
	}
}

Vector3d ThreadConstrained::start_pos() {
	return (threads.front())->start_pos();
}

Vector3d ThreadConstrained::end_pos() {
	return (threads.back())->end_pos();
}

Matrix3d ThreadConstrained::start_rot() {
	return (threads.front())->start_rot();
}

Matrix3d ThreadConstrained::end_rot() {
	return (threads.back())->end_rot();
}

void ThreadConstrained::set_coeffs_normalized(double bend_coeff, double twist_coeff, double grav_coeff) {
	for (int thread_num=0; thread_num<threads.size(); thread_num++)
		threads[thread_num]->set_coeffs_normalized(bend_coeff, twist_coeff, grav_coeff);
}

void ThreadConstrained::set_coeffs_normalized(const Matrix2d& bend_coeff, double twist_coeff, double grav_coeff) {
	for (int thread_num=0; thread_num<threads.size(); thread_num++)
		threads[thread_num]->set_coeffs_normalized(bend_coeff, twist_coeff, grav_coeff);
}

void ThreadConstrained::minimize_energy() {
	for (int thread_num=0; thread_num<threads.size(); thread_num++)
		threads[thread_num]->minimize_energy();
}

void ThreadConstrained::updateConstraints (vector<Vector3d> poss, vector<Matrix3d> rots) {
	if (poss.size() != rots.size())
		cout << "Internal Error: setConstraints: different number of positions and rotations" << endl;
	if (poss.size() != threads.size()+1)
		cout << "Internal Error: setConstraints: not the right number of transforms for current threads. addConstraint or removeConstraint first." << endl;
	for (int i=0; i<threads.size(); i++) {
		Matrix3d new_end_rot;
		if (i==threads.size()-1) {
			new_end_rot = rots[i+1];
		} else {
			new_end_rot = rotation_difference[i]*rots[i+1];
		}
		zero_angle[i] += angle_mismatch(rots[i], threads[i]->start_rot());
	  threads[i]->set_constraints_nearEnds(poss[i], rots[i], poss[i+1], new_end_rot);
	}
	//cout << "\t\tupdateConstraints is being called" << endl;
	//cout << zero_angle[0] << "\t\t" << zero_angle[1] << endl;
}

void ThreadConstrained::printConstraints () {
	cout << "printing constraints: " << endl;
	cout << threads[0]->start_rot() << endl;
	cout << threads[0]->end_rot() << endl;
	//cout << threads[1]->start_rot() << endl;
	//cout << threads[1]->end_rot() << endl;
}

void ThreadConstrained::addConstraint (int absolute_vertex_num) {
	int thread_num = insertSorted(constrained_vertices_nums, absolute_vertex_num)-1;
	splitThread(thread_num, localVertex(absolute_vertex_num));
}

void ThreadConstrained::removeConstraint (int absolute_vertex_num) {
	int thread_num = removeSorted(constrained_vertices_nums, absolute_vertex_num) - 1;
	mergeThread(thread_num);
}

/*// Returns the number of the vertex that is nearest to pos. The chosen vertex have to be in vertices but not in vertex_exception.
int ThreadConstrained::nearestVertex(Vector3d pos, vector<Vector3d> vertices, vector<int> vertex_exceptions) {
	int vertex_num;
	vector<double> distances;
	for (vertex_num=0; vertex_num<vertices.size(); vertex_num++) {
		distances.push_back((vertices[vertex_num]-pos).squaredNorm());
	}
	invalidateSortedElements(distances, vertex_exceptions);
	for (vertex_num=0; distances[vertex_num]<0; vertex_num++) { }
	int min_vertex_num = vertex_num;
	for (vertex_num=min_vertex_num+1; vertex_num<distances.size(); vertex_num++)
		if (distances[vertex_num]>=0 && (distances[vertex_num] < distances[min_vertex_num]))
			min_vertex_num = vertex_num;
	return min_vertex_num;
}*/

// Splits the thread threads[thread_num] into two threads, which are stored at threads[thread_num] and threads[thread_num+1].  Threads in threads that are stored after thread_num now have a new thread_num which is one unit more than before. The split is done at vertex vertex of thread[thread_num]
void ThreadConstrained::splitThread(int thread_num, int vertex) {
	vector<Vector3d> point0;
	vector<Vector3d> point1;
	vector<Vector3d> points;
	vector<double> twist_angles;
	threads[thread_num]->get_thread_data(points, twist_angles);
	vector<double> twist_angle0(vertex+1, 0.0);
	vector<double> twist_angle1(twist_angles.size()-vertex, 0.0);

	splitVector(point0, point1, points, vertex);

  Matrix3d vertex_end_rot = threads[thread_num]->material_at_ind(vertex-1);
  Matrix3d vertex_start_rot = threads[thread_num]->material_at_ind(vertex);
  Matrix3d difference_matrix = vertex_end_rot*vertex_start_rot.transpose();
	insertAt(rotation_difference, difference_matrix, thread_num);

	/*cout << "twist_angles: " << twist_angles.size() << endl;
  for (int i=0; i<(twist_angles.size()+4)/5; i++) {
    for (int j=0; j<5 && (i*5+j)<twist_angles.size(); j++) {
      printf("%d:%3.4f\t",i*5+j,twist_angles[i*5+j]);
    }
    printf("\n");
  }*/

	Thread* thread0 = new Thread(point0, twist_angle0, (Matrix3d&) (threads[thread_num])->start_rot(), vertex_end_rot);
	Thread* thread1 = new Thread(point1, twist_angle1, vertex_start_rot, (Matrix3d&) (threads[thread_num])->end_rot());
	delete threads[thread_num];
	threads[thread_num] = thread0;
	insertAt(threads, thread1, thread_num+1);
	
	threads[thread_num]->minimize_energy();
	threads[thread_num+1]->minimize_energy();
	threads[thread_num]->get_thread_data(point0, twist_angle0);
	threads[thread_num+1]->get_thread_data(point1, twist_angle1);
	
	//cout << "twist_angle 0 and 1 after creating threads and minimizing energy: " << endl;

	twist_angle0.back() = 2.0*twist_angle0[twist_angle0.size()-2] - twist_angle0[twist_angle0.size()-3];
	twist_angle1.back() = 2.0*twist_angle1[twist_angle1.size()-2] - twist_angle1[twist_angle1.size()-3];
	
	/*cout << "twist_angle0: " << twist_angle0.size() << endl;
  for (int i=0; i<(twist_angle0.size()+4)/5; i++) {
    for (int j=0; j<5 && (i*5+j)<twist_angle0.size(); j++) {
      printf("%d:%3.4f\t",i*5+j,twist_angle0[i*5+j]);
    }
    printf("\n");
  }
	cout << "twist_angle1: " << twist_angle1.size() << endl;
  for (int i=0; i<(twist_angle1.size()+4)/5; i++) {
    for (int j=0; j<5 && (i*5+j)<twist_angle1.size(); j++) {
      printf("%d:%3.4f\t",i*5+j,twist_angle1[i*5+j]);
    }
    printf("\n");
  }*/
  insertAt(zero_angle, zero_angle[thread_num] + twist_angle0.back(), thread_num+1);
	//cout << "zero_angle[" << thread_num << "]: " << zero_angle[thread_num] << endl;
  //cout << "zero_angle[" << thread_num+1 << "]: " << zero_angle[thread_num+1] << endl;
}

// Merges the threads threads[thread_num] and threads[thread_num+1] into one thread, which is stored at threads[thread_num]. Threads in threads that are stored after thread_num+1 now have a new thread_num which is one unit less than before.
void ThreadConstrained::mergeThread(int thread_num) {
	vector<Vector3d> points0;
	vector<Vector3d> points1;
	vector<Vector3d> point;
	vector<double> twist_angles0;
	vector<double> twist_angles1;
	threads[thread_num]->get_thread_data(points0, twist_angles0);
	threads[thread_num+1]->get_thread_data(points1, twist_angles1);
	vector<double> twist_angle(twist_angles0.size()+twist_angles1.size()-1);

	mergeVector(point, points0, points1);
	//mergeVector(twist_angle, twist_angles0, twist_angles1);

	zero_angle.erase(zero_angle.begin()+thread_num+1);
	rotation_difference.erase(rotation_difference.begin()+thread_num);

	Thread* thread = new Thread(point, twist_angle, (Matrix3d&) (threads[thread_num])->start_rot(), (Matrix3d&) (threads[thread_num+1])->end_rot());
	delete threads[thread_num];
	delete threads[thread_num+1];
	threads[thread_num] = thread;
	threads.erase(threads.begin()+thread_num+1);
	threads[thread_num]->minimize_energy();
}

// Returns the thread number that owns the absolute_vertex number. absolute_vertex must not be a constrained vertex
int ThreadConstrained::threadOwner(int absolute_vertex) {
	int thread_num;
	int num_absolute_vertices = 1;
	vector<vector<Vector3d> > points(threads.size());
	for (thread_num=0; thread_num<threads.size(); thread_num++) {
		threads[thread_num]->get_thread_data(points[thread_num]);
		num_absolute_vertices += (points[thread_num]).size() - 1;
		if (absolute_vertex < num_absolute_vertices) { break; }
	}
	if (thread_num==threads.size()) {
		cout << "Internal error: threadOwner: absolute_vertex is greater than total number of vertices" << endl;
		return 0;
	} else {
		return thread_num;
	}
}

// Returns the local vertex number (i.e. vertex number within a thread), given the absolute vertex number (i.e. vertex number within all vertices).
int ThreadConstrained::localVertex(int absolute_vertex) {
	int thread_num;
	int num_absolute_vertices = 1;
	vector<vector<Vector3d> > points(threads.size());
	for (thread_num=0; thread_num<threads.size(); thread_num++) {
		threads[thread_num]->get_thread_data(points[thread_num]);
		num_absolute_vertices += (points[thread_num]).size() - 1;
		if (absolute_vertex < num_absolute_vertices) { break; }
	}
	if (thread_num==threads.size()) {
		cout << "Internal error: localVertex: absolute_vertex is greater than total number of vertices" << endl;
		return 0;
	} else {
		return absolute_vertex + (points[thread_num]).size() - num_absolute_vertices;
	}
}

// Invalidates (sets to -1) the elements of v at indices around i. Indices i are specified by constraintsNums. Indices in constraintsNums have to be in the range of v.
void invalidateAroundConstraintsNums(vector<int> &v, vector<int> constraintsNums) {
	for (int i=0; i<v.size(); i++)
		v[i] = i;
	for (int i=0; i<constraintsNums.size(); i++) {
		if (constraintsNums[i] == 0) {
			v[constraintsNums[i]+1] = v[constraintsNums[i]+2] = -1;
		} else if (constraintsNums[i] == 1) {
			v[constraintsNums[i]-1] = v[constraintsNums[i]+1] = v[constraintsNums[i]+2] = -1;
		} else if (constraintsNums[i] == v.size()-1) {
			v[constraintsNums[i]-2] = v[constraintsNums[i]-1] = -1;
		} else if (constraintsNums[i] == v.size()-2) {
			v[constraintsNums[i]-2] = v[constraintsNums[i]-1] = v[constraintsNums[i]+1] = -1;
		} else {
			v[constraintsNums[i]-2] = v[constraintsNums[i]-1] = v[constraintsNums[i]+1] = v[constraintsNums[i]+2] = -1;
		}
	}
}

// Invalidates (sets to -1) the elements of v at indices i. Indices i are specified by constraintsNums. Indices in constraintsNums have to be in the range of v.
void invalidateConstraintsNums(vector<int> &v, vector<int> constraintsNums) {
	for (int i=0; i<v.size(); i++)
		v[i] = i;
	for (int i=0; i<constraintsNums.size(); i++) {
		v[constraintsNums[i]] = -1;
	}
}

/*// Elements v[i-1], v[i] and v[i+1] are removed from v, where the indices i are specified in invalidElements. invalidElements have to be sorted. The difference between adjacent elements in invalidElements have to be of at least 3.
template<typename T>
void removeInvalidSortedElements(vector<T> &v, vector<int> invalidElements) {
	if (invalidElements.size()) {
		vector<int>::reverse_iterator invIt = invalidElements.rbegin();
		int i = invalidElements.size() - 1;
		int vect_size = v.size();
		while (invIt < invalidElements.rend()) {
			if (invalidElements[i]==vect_size-1) {
				v.erase(v.begin()+invalidElements[i]-1, v.begin()+invalidElements[i]+1);
			}	else if (invalidElements[i]==0) {
				v.erase(v.begin()+invalidElements[i], v.begin()+invalidElements[i]+2);
			} else {
				v.erase(v.begin()+invalidElements[i]-1, v.begin()+invalidElements[i]+2);
			}
			invIt++;
			i--;
		}
	}
}

// Elements v[i-1] and v[i+1] are removed from v, where the indices i are specified in invalidElements. invalidElements have to be sorted. The difference between adjacent elements in invalidElements have to be of at least 3.
template<typename T>
void removeAdjacentInvalidSortedElements(vector<T> &v, vector<int> invalidElements) {
	if (invalidElements.size()) {
		vector<int>::reverse_iterator invIt = invalidElements.rbegin();
		int i = invalidElements.size() - 1;
		int vect_size = v.size();
		while (invIt < invalidElements.rend()) {
			if (invalidElements[i]==vect_size-1) {
				v.erase(v.begin()+invalidElements[i]-2, v.begin()+invalidElements[i]);
			}	else if (invalidElements[i]==0) {
				v.erase(v.begin()+invalidElements[i]+1, v.begin()+invalidElements[i]+3);
			} else if (invalidElements[i]==vect_size-2) {			
				v.erase(v.begin()+invalidElements[i]+1);
				v.erase(v.begin()+invalidElements[i]-2, v.begin()+invalidElements[i]);
			}	else if (invalidElements[i]==1) {			
				v.erase(v.begin()+invalidElements[i]+1, v.begin()+invalidElements[i]+3);
				v.erase(v.begin()+invalidElements[i]-1);
			} else {
				v.erase(v.begin()+invalidElements[i]+1, v.begin()+invalidElements[i]+3);
				v.erase(v.begin()+invalidElements[i]-2, v.begin()+invalidElements[i]);
			}
			invIt++;
			i--;
		}
	}
}

// Elements v[i] are invalidated (set to -1), where the indices i are specified in invalidElements. invalidElements have to be sorted.
template<typename T>
void invalidateSortedElements(vector<T> &v, vector<int> invalidElements) {
	if (invalidElements.size()) {
		vector<int>::reverse_iterator invIt = invalidElements.rbegin();
		int i = invalidElements.size() - 1;
		int vect_size = v.size();
		while (invIt < invalidElements.rend()) {
			if (invalidElements[i]==vect_size-1) {
				v[invalidElements[i]-1] = v[invalidElements[i]] = -1;
			}	else if (invalidElements[i]==0) {
				v[invalidElements[i]] = v[invalidElements[i]+1] = -1;
			} else {
				v[invalidElements[i]-1] = v[invalidElements[i]] = v[invalidElements[i]+1] = -1;
			}
			invIt++;
			i--;
		}
	}
}*/

template<typename T>
void mapAdd (vector<T> &v, T num) {
	for (int i=0; i<v.size(); i++)
		v[i] = v[i] + num;
}

// Inserts an element at index index. The elements after index index are moved one position to the right in vector.
template<typename T>
void insertAt (vector<T> &v, T e, int index) {
	v.insert(v.begin()+index, e);
}

// Last element of v1 and first element of v2 are equal to v[index].
template<typename T>
void splitVector (vector<T> &v1, vector<T> &v2, vector<T> v, int index) {
	v1.clear();
	v2.clear();
	v1.insert(v1.begin(), v.begin(), v.begin() + index + 1);
	v2.insert(v2.begin(), v.begin() + index, v.end());
}
// Last element of v1 is discarded since it is assumed to be the same as the first element of v2.
template<typename T>
void mergeVector (vector<T> &v, vector<T> v1, vector<T> v2) {
	v = v1;
	v.pop_back();
	v.insert(v.end(), v2.begin(), v2.end());
}

// Last element of a vector in vectors is discarded since it is assumed to be the same as the first element of the consecutive vector in vectors. The last vector in vectors is the exception.
template<typename T>
void mergeMultipleVector(vector<T> &v, vector<vector<T> > vectors) {
	if (vectors.size()==1) {
		v = vectors[0];
	} else {
		mergeVector(v,vectors[0],vectors[1]);
		for (int i=2; i < vectors.size(); i++)
			mergeVector(v,v,vectors[i]);
	}
}

// Returns the position where the element was inserted.
int insertSorted (vector<int> &v, int e) {
	if (!v.size()) {
		v.push_back(e);
		return 0;
	}
	int last = v.size() - 1;
	v.push_back(v[last]);
	int i;
	for (i=last; i>=0 && v[i]>e; i--)
		v[i] = v[i - 1];
	if (v[i]==e)
		cout << "Internal error: insertSorted: element to insert is already in vector" << endl;
	v[i+1] = e;
	return i+1;
}

//Returns the position where the element was removed. Element to remove has to be in vector.
int removeSorted (vector<int> &v, int e) {
	int i;
	for (i=0; i<v.size() && v[i]!=e; i++) {}
	if (i==v.size())
		cout << "Internal error: remove: element to remove is not in vector" << endl;
	v.erase(v.begin()+i);
	return i;
}

