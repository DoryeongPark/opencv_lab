#include<iostream>
#include<vector>
#include<chrono>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>

using namespace std;
using namespace cv;
using namespace chrono;

/*
	Author		:	Doryeong Park
	Date		:	18. 4. 2017	
	Description	:	Instantiates & Manages tracking images   
*/

namespace tracking {

	/*
		Expresses information of one object image bound  
	*/
	class TrackingObject {

		Rect roi;
		long start_time;

		int index = 0;

	public:
		
		TrackingObject() = default;
		bool is_overlapped(const Rect& object);

	};


	/*
		Manager class for all tracking objects
	*/
	class TrackingObjectPool {

		vector<TrackingObject> pool;

		long current_time;

	public:
	
		int get_counts();
		void set_current_time();
		void reflect(const vector<Rect>& objects);

	private:

		bool is_continous(const Rect& rect);
		bool is_new(const Rect& rect);

	};
	
}
