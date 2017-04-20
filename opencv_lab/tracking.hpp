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

	struct IKnowTime {
	protected:
		long get_current_time();
	};


	/*
		Expresses information of one object image bound  
	*/
	class TrackingObject : IKnowTime
	{

		Rect object;
		long start_time;

		int index;

	public:
		
		TrackingObject(const Rect& rect);
		bool is_overlapped(const Rect& object);
		void update(Rect& object);
		bool is_valid();
		Rect& get_object();

	};


	/*
		Manager class for all tracking objects
	*/
	class TrackingObjectPool : IKnowTime
	{

		vector<TrackingObject> pool;

	public:
	
		int get_counts();
		void reflect(vector<Rect>& objects);
		void display_objects(Mat& current_frame);

	};
	
}
