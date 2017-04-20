#include"tracking.hpp"

using namespace tracking;

//==========================================================
//
//	Tracking function definition
//
//==========================================================

TrackingObject::TrackingObject(const Rect& object){

	reset();
	this->object = object;
	index = 1;

}


bool TrackingObject::is_overlapped
(
	const Rect& current_object
)
{
	
	Point edges[4];
	
	edges[0] = Point(current_object.x, current_object.y);
	edges[1] = Point(current_object.x + current_object.width, current_object.y);
	edges[2] = Point(current_object.x, current_object.y + current_object.height);
	edges[3] = Point(current_object.x + current_object.width, current_object.y + current_object.height);

	for (int i = 0; i < 4; ++i)
		if (object.contains(edges[i]))
			return true;

	return false;
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
	milliseconds duration_milliseconds = this->pick();
	
	//========================================
	// Judge Something with duration and index
	//========================================

	if (duration_milliseconds.count() < 10 && index == 1)
		return true;

	//10 will be variable depends on frame_rate 
	return (duration_milliseconds.count() / index) < 100;

}

Rect& TrackingObject::get_object()
{
	return object;
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

	//For objects from current image
	for (auto& current_object : objects) {
		
		bool is_reflected = false;

		//For tracking objects from previous image
		for (auto& tracking_object : pool) {

			if (tracking_object.is_overlapped(current_object)) {

				tracking_object.update(current_object);
				is_reflected = true;

			}
			
		}

		if (!is_reflected) {
			
			pool.emplace_back(TrackingObject{ current_object });

		}

	}

	delete[] overlapped_histogram;

	//Remove low indexed object	
	for (auto iter = pool.begin();
		iter != pool.end();) {

		if (!iter->is_valid()) {

			iter = pool.erase(iter);
			continue;

		}

		++iter;
	}
		
}

void TrackingObjectPool::display_objects
(
	Mat& current_frame
)
{

	Mat showing_frame;
	current_frame.copyTo(showing_frame);

	for (auto& object : pool)
		rectangle(showing_frame, object.get_object(), Scalar{ 0, 0, 255 });

	imshow("Tracking objects", showing_frame);

}


