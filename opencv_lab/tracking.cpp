#include"tracking.hpp"

using namespace tracking;

//==========================================================
//
//	Tracking function definition
//
//==========================================================

TrackingObject::TrackingObject(const Rect& rect){

	auto now = system_clock::now();
	auto now_ms = time_point_cast<milliseconds>(now);
	auto epoch = now_ms.time_since_epoch();
	auto value = duration_cast<milliseconds>(epoch);
	start_time = value.count();

}


bool TrackingObject::is_overlapped
(
	const Rect& current_object
)
{

	auto min_x = object.x;
	auto max_x = min_x + object.width;

	auto min_y = object.y;
	auto max_y = min_y + object.height;

	return (min_x <= current_object.x && current_object.x <= max_x) && 
		   (min_y <= current_object.y && current_object.y <= max_y);

}

void TrackingObject::set_rect
(
	Rect& object
)
{

	this->object = object;

}

bool TrackingObject::is_valid
(
	const long& current_time
)
{

	//현재 시간 차와 출현 빈도 수를

}


//==========================================================
//
//	TrackingObjectPool function definition
//
//==========================================================

int TrackingObjectPool::get_counts() {

	return pool.size();

}

void TrackingObjectPool::set_current_time() {



}

void TrackingObjectPool::reflect
(
	vector<Rect>& objects
) 
{
	//For objects from current image
	for (auto&& current_object : objects) {
		
		//For tracking objects from previous image
		for (auto&& tracking_object : this->pool) {
			
			tracking_object.is_overlapped(current_object);
			
					
			
		}
	}
		
}

bool TrackingObjectPool::is_continous
(
	const Rect& object
) 
{
	return false;
}

bool TrackingObjectPool::is_new(
	const Rect& object
) 
{
	return false;
}