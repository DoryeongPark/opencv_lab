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

	using namespace std::chrono;
	using std::chrono::system_clock;

	// - Note
	//      Stop watch.
	//      - `pick()`  : Acquire elapsed time from start
	//      - `reset()` : Reset the starting time_point
	template <typename Clock>
	class stop_watch
	{
	public:
		using clock_type = Clock;
		using time_point = typename clock_type::time_point;
		using duration = typename clock_type::duration;

	protected:
		time_point start = clock_type::now();

	public:
		template <typename UnitType = std::chrono::milliseconds>
		decltype(auto) pick() const noexcept
		{
			duration span = clock_type::now() - start;
			return std::chrono::duration_cast<UnitType>(span);
		};

		template <typename UnitType = std::chrono::milliseconds>
		decltype(auto) reset() noexcept
		{
			auto span = this->pick<UnitType>();
			// reset start time point
			start = clock_type::now();
			return std::chrono::duration_cast<UnitType>(span);
		}
	};

	/*
		Expresses information of one object image bound  
	*/
	class TrackingObject : stop_watch<system_clock>
	{

		Rect object;
		//system_clock::time_point start_time;

		int index;

	public:
		
		explicit TrackingObject(const Rect& rect);
		bool is_overlapped(const Rect& object);
		void update(Rect& object);
		bool is_valid();
		Rect& get_object();

	};


	/*
		Manager class for all tracking objects
	*/
	class TrackingObjectPool
	{

		vector<TrackingObject> pool;

	public:
	
		int get_counts();
		void reflect(vector<Rect>& objects);
		void display_objects(Mat& current_frame);

	};
	
}
