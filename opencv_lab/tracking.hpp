#include<iostream>
#include<vector>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>

using namespace std;
using namespace cv;

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
		long startTrackingTime;

	public:
		
		

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
