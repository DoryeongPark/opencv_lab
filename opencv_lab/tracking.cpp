#include"tracking.hpp"

using namespace tracking;

//==========================================================
//
//	Tracking Object function definition
//
//==========================================================

TrackingObject::TrackingObject(const Rect& object){

	this->reset();
	this->object = object;

}


Rect& TrackingObject::get_object() {

	return object;

}

bool TrackingObject::is_overlapped
(
	Rect& current_object
)
{

	return (object & current_object).area() > 0;

}

void TrackingObject::update
(
	Rect& object
)
{

	this->candidate = &object;
	this->reset();
	++this->tracking_point;

}

void TrackingObject::commit() {

	if (candidate != nullptr) {

		object = *candidate;
		candidate = nullptr;

	}
}

bool TrackingObject::is_valid()
{
	
	//Get duration
	milliseconds duration_milliseconds = this->pick();
	
	//========================================
	// Judge deletion with duration
	//========================================

	//Here will be variable cab be changed by frame_rate 
	return (duration_milliseconds.count()) < 1000 && number > 0;

}


int TrackingObject::get_number(){

	return number;

}


void TrackingObject::set_number(const int& _number) {

	number = _number;

}

void TrackingObject::decrease_number() {

	--number;

}


int TrackingObject::get_overlap_point() {

	return overlap_point;

}

void TrackingObject::set_overlap_point
(
	const int& _overlap_point
) 
{
	
	overlap_point = _overlap_point;

}


void TrackingObject::increse_overlap_point() {
	
	++overlap_point;

}


int TrackingObject::get_tracking_point() {
	
	return tracking_point;
	
}

void TrackingObject::set_tracking_point
(
	const int& _tracking_point
) 
{
	
	tracking_point = _tracking_point;

}

void TrackingObject::increase_tracking_point() {
	
	++tracking_point;

}


//==========================================================
//
//	TrackingObjectPool function definition
//
//==========================================================

int TrackingObjectPool::get_object_count() {

	int result = 0;
	
	for (auto& tracking_object : pool)
		if(tracking_object.get_tracking_point() > count_standard &&
		   tracking_object.get_number() > 0)
			result += tracking_object.get_number();

	return result;

}

void TrackingObjectPool::set_count_standard
(
	const int& _count_standard_point
)
{

	this->count_standard = _count_standard_point;

}


void TrackingObjectPool::reflect
(
	vector<Rect>& objects
)
{

	//For objects from current image
	for (auto& current_object : objects) {
		
		vector<int> overlapped_indexes;
		overlapped_indexes.reserve(objects.size());

		int overlapped_index = 0;
		
		//For tracking objects from previous image
		for (auto tracking_object = pool.begin();
				  tracking_object != pool.end(); 
				  ++tracking_object) {
			
			if (tracking_object->is_overlapped(current_object)) {

				overlapped_indexes.emplace_back(overlapped_index);
				tracking_object->increse_overlap_point();
				
			}

			++overlapped_index;

		}
		
		auto overlapped_indexes_size = overlapped_indexes.size();
		
		//Regard it as new object
		if (overlapped_indexes_size == 0) {
			
			pool.emplace_back(TrackingObject{ current_object });

		}
		else if(overlapped_indexes_size == 1) {
			//Dividing routine
			if (pool[overlapped_indexes[0]].get_overlap_point() > 1){ 

				pool[overlapped_indexes[0]].decrease_number();
				auto another = TrackingObject{ current_object };
				another.set_tracking_point(pool[overlapped_indexes[0]].get_tracking_point());
				pool.emplace_back(another);
				
			}
			else {

				pool[overlapped_indexes[0]].update(current_object);
			
			}

		}
		else {
			
			int current_tracking_point = 0;
			int max_tracking_point = 0;
			int total_number = 0;

			int number = 0;
			bool is_merged_with_noise = false;

			//Determine whether it is merged with noise or not
			for (auto& index : overlapped_indexes) {

				total_number += pool[index].get_number();
				current_tracking_point = pool[index].get_tracking_point();
				
				if (current_tracking_point > max_tracking_point)
					max_tracking_point = current_tracking_point;
				
				if (current_tracking_point < count_standard) 
					is_merged_with_noise = true;
					
			}
			
			//Regard it as collsion with noise
			if (is_merged_with_noise)
				number = 1;

			//Regard it as collision with people
			else
				number = total_number;
			
			TrackingObject tracking_object{ current_object };
			tracking_object.set_number(number);
			tracking_object.set_tracking_point(max_tracking_point);
			pool.emplace_back(tracking_object);

			//Erase overlapped tracking objects
			sort(overlapped_indexes.rbegin(), 
				 overlapped_indexes.rend());
			
			for (auto& index : overlapped_indexes) {
				pool.erase(pool.begin() + index);
			}
	
		}
			
	}

	//Remove object which has long duration	
	for (auto iter = pool.begin();
			  iter != pool.end();) {
		
		iter->commit();

		if (!iter->is_valid()) {
			
			Rect* erased_noise = &(iter->get_object());
			iter = pool.erase(iter);
			continue;

		}

		++iter;

	}

	reset_overlap_points();

}



void TrackingObjectPool::display_objects
(
	Mat& current_frame
)
{

	Mat showing_frame;
	current_frame.copyTo(showing_frame);

	for (auto& object : pool) {

		if (object.get_tracking_point() >= count_standard) 
			rectangle(showing_frame, object.get_object(), Scalar{ 255, 0, 0 });
		else 
			rectangle(showing_frame, object.get_object(), Scalar{ 0, 0, 255 });
		

		putText(showing_frame, to_string(object.get_number()), 
			Point(object.get_object().x + object.get_object().width / 2, 
				  object.get_object().y + object.get_object().height / 3), 3, 0.4, Scalar{ 255, 0, 0 });
		putText(showing_frame, to_string(object.get_tracking_point()),
			Point(object.get_object().x + object.get_object().width / 2,
				object.get_object().y + object.get_object().height / 3 + 10), 3, 0.4, Scalar{ 0, 255, 0 });

	}
		
	putText(showing_frame, "COUNT: " + to_string(get_object_count()), Point{ 10, 20 }, 4, 0.6, Scalar{ 255, 255, 0 });
	imshow("Tracking objects", showing_frame);

}


void TrackingObjectPool::reset_overlap_points() {

	for (auto& tracking_object : pool)
		tracking_object.set_overlap_point(0);

}







