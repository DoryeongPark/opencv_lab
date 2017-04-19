#include"tracking.hpp"

using namespace tracking;

//==========================================================
//
//	Tracking function definition
//
//==========================================================

TrackingObject::TrackingObject(const Rect& object){
	
	start_time = get_current_time_milliseconds();
	this->object = object;
	index = 0;

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

void TrackingObject::update
(
	Rect& object
)
{

	this->object = object;
	++index;

}

bool TrackingObject::is_valid()
{
	
	//Get duration
	auto current_time = get_current_time_milliseconds();
	long duration_milliseconds = start_time - current_time;
	
	//===================================
	// Judge Something with gap and index
	//===================================

	return true;
	
}


//==========================================================
//
//	TrackingObjectPool function definition
//
//==========================================================

int TrackingObjectPool::get_counts() {

	return pool.size();

}


void TrackingObjectPool::reflect
(
	vector<Rect>& objects
) 
{

	//Remove low indexed object	
	for (auto iter = pool.begin(); 
			  iter != pool.end();) {

		if (!iter->is_valid()) {

			iter = pool.erase(iter);
			continue;

		}

		++iter;
	}
	 
	//For objects from current image
	for (auto&& current_object : objects) {
		
		bool is_reflected = false;

		//For tracking objects from previous image
		for (auto&& tracking_object : pool) {
			
			if (tracking_object.is_overlapped(current_object)) {

				tracking_object.update(current_object);
				is_reflected = true;

			}
			
		}

		if (!is_reflected) {
			
			pool.emplace_back(current_object);

		}

	}
		
}
