#include"tracking.hpp"

using namespace tracking;

//==========================================================
//
//	Tracking function definition
//
//==========================================================

TrackingObject::TrackingObject(){

	auto now = system_clock::now();
	auto now_ms = time_point_cast<milliseconds>(now);
	auto epoch = now_ms.time_since_epoch();
	auto value = duration_cast<milliseconds>(epoch);
	start_time = value.count();

}


bool TrackingObject::is_overlapped
(
	const Rect& object
)
{
	
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
	const vector<Rect>& objects
) 
{



}

bool TrackingObjectPool::is_continous
(
	const Rect& object
) 
{

}

bool TrackingObjectPool::is_new(
	const Rect& object
) 
{

}