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
	area = object.area();

}

Rect& TrackingObject::get_object() {

	return object;

}

int TrackingObject::get_area() {

	return object.area();

}

//NEED TO BE MODIFIED - IT DOESN'T COVER ALL CASES!
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

	this->object = object;
	this->area = object.area();
	this->reset();
	++this->tracking_point;

}

bool TrackingObject::is_valid()
{
	
	//Get duration
	milliseconds duration_milliseconds = this->pick();
	
	//========================================
	// Judge deletion with duration
	//========================================

	//Here will be variable cab be changed by frame_rate 
	return (duration_milliseconds.count()) < 800;

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

void TrackingObject::set_overlap_point(const int& _overlap_point) {
	
	overlap_point = _overlap_point;

}


void TrackingObject::increse_overlap_point() {
	
	++overlap_point;

}



int TrackingObject::get_tracking_point() {
	
	return tracking_point;
	
}

void TrackingObject::set_tracking_point(const int& _tracking_point) {
	
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

int TrackingObjectPool::get_counts() {

	int result = 0;
	
	for (auto& tracking_objects : pool)
		result += tracking_objects.get_number();

	return result;

}


void TrackingObjectPool::reflect
(
	Mat& current_frame, //Parameter for test
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
				tracking_object->increase_tracking_point();
				
			}

			++overlapped_index;

		}
				
		auto overlapped_indexes_size = overlapped_indexes.size();
		
		//Regard it as new object
		if (overlapped_indexes_size == 0) {
			
			pool.emplace_back(TrackingObject{ current_object });
			/*cout << "New Object Inserted - [" << current_object.x  << " " << current_object.y << " " 
				 << current_object.width << " " << current_object.height << "]" << endl;*/

			//Showing new object
			Mat showing_frame;
			current_frame.copyTo(showing_frame);
			rectangle(showing_frame, current_object, Scalar{ 0, 255, 0 });
			imshow("New Object", showing_frame);

		}
		else if(overlapped_indexes_size == 1) {

			//@@@ Execute it after loop @@@//
			if (pool[overlapped_indexes[0]].get_overlap_point() > 1){ 

				pool[overlapped_indexes[0]].decrease_number();
				auto&& another = TrackingObject{ current_object };
				another.set_tracking_point(pool[overlapped_indexes[0]].get_tracking_point());
				pool.emplace_back(another);
				cout << "Object branched" << endl;

			}
			else {

				pool[overlapped_indexes[0]].update(current_object);
			
			}

		}
		else {
			
			int tracking_point = 0;
			int total_number = 0;

			int number = 0;
			bool is_merged_with_noise = false;

			//Determine whether it is merged with noise or not
			for (auto& index : overlapped_indexes) {

				total_number += pool[index].get_number();
				tracking_point = pool[index].get_tracking_point();

				if (tracking_point < 15) {
					
					is_merged_with_noise = true;
					break;

				}
					
			}
			
			//Regard it as collsion with noise
			if (is_merged_with_noise)
				number = 1;

			//Regard it as collision with people
			else
				number = total_number;
			
			TrackingObject tracking_object{ current_object };
			tracking_object.set_number(number);
			tracking_object.set_tracking_point(tracking_point);
			pool.emplace_back(tracking_object);

			//Erase overlapped tracking objects
			sort(overlapped_indexes.rbegin(), 
				 overlapped_indexes.rend());
			
			for (auto& index : overlapped_indexes)
				pool.erase(pool.begin() + index);

			cout << "Object merged" << endl;
	
		}
			
	}

	//Remove object which has long duration	
	for (auto iter = pool.begin();
			  iter != pool.end();) {

		if (!iter->is_valid()) {
			
			Rect* erased_noise = &(iter->get_object());
			cout << "Detect noise - [" << erased_noise->x << " " << erased_noise->y 
				 << " " << erased_noise->width << " " << erased_noise->height 
				 << "]" << endl;
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
		rectangle(showing_frame, object.get_object(), Scalar{ 0, 0, 255 });
		putText(showing_frame, to_string(object.get_number()), 
			Point(object.get_object().x + object.get_object().width / 2, 
				  object.get_object().y + object.get_object().height / 3), 3, 0.4, Scalar{ 255, 0, 0 });
		putText(showing_frame, to_string(object.get_tracking_point()),
			Point(object.get_object().x + object.get_object().width / 2,
				object.get_object().y + object.get_object().height / 3 + 10), 3, 0.4, Scalar{ 0, 255, 0 });
	}
		
			
	imshow("Tracking objects", showing_frame);

}


void TrackingObjectPool::reset_overlap_points() {

	for (auto& tracking_object : pool)
		tracking_object.set_overlap_point(0);

}







