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

	//Function for two classes
	long get_current_time_milliseconds() {

		auto now = system_clock::now();
		auto now_ms = time_point_cast<milliseconds>(now);
		auto epoch = now_ms.time_since_epoch();
		auto value = duration_cast<milliseconds>(epoch);
		
		return value.count();

	}

	/*
		Expresses information of one object image bound  
	*/
	class TrackingObject {

		Rect object;
		long start_time;

		int index;

	public:
		
		TrackingObject(const Rect& rect);
		bool is_overlapped(const Rect& object);
		void set_rect(Rect& object);
		bool is_valid();

	};


	/*
		Manager class for all tracking objects
	*/
	class TrackingObjectPool {

		vector<TrackingObject> pool;

		long current_time;

	public:
	
		int get_counts();
		void reset_current_time();
		void reflect(vector<Rect>& objects);

	private:

		bool is_continous(const Rect& rect);
		bool is_new(const Rect& rect);

	};
	
}
