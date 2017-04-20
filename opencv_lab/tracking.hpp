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
		int score;
		int area;
		
		int number = 1;

		int overlap_point = 0;

	public:
		
		explicit TrackingObject(const Rect& rect);
		bool is_overlapped(const Rect& object);
		void update(Rect& object);
		bool is_valid();
		Rect& get_object();
		int get_area();

		int get_number();
		void set_number(const int& _number);
		void decrease_number();

		int get_overlap_point();
		void set_overlap_point(const int& _overlap_point);
		void increse_overlap_point();
		
	
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

	private:
		void reset_overlap_points();

	};
	
}
