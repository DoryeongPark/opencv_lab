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
				
			}

			++overlapped_index;

		}
				
		auto overlapped_indexes_size = overlapped_indexes.size();
		
		//Regard it as new object
		if (overlapped_indexes_size == 0) {
			
			pool.emplace_back(TrackingObject{ current_object });
			cout << "New Object Inserted - [" << current_object.x  << " " << current_object.y << " " 
				 << current_object.width << " " << current_object.height << "]" << endl;

			//Testing new object
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
				pool.emplace_back(another);
				cout << "Updated - overlapped_indexes_size == 1, Overlap Point > 1" << endl;

			}
			else {

				pool[overlapped_indexes[0]].update(current_object);
				cout << "Updated - overlapped_indexes_size == 1, Overlap Point == 1" << endl;

			}

		}
		else {

			int total_area = 0;
			int number = 0;

			//Get total area size
			for (auto& index : overlapped_indexes) 
				total_area += pool[index].get_area();
			
			//Regard it as collsion of people
			if (total_area > current_object.area())
				for (auto& index : overlapped_indexes)
					number += pool[index].get_number();

			//Regard it as segmentation of people
			else
				number = 1;
			
			TrackingObject tracking_object{ current_object };
			tracking_object.set_number(number);
			pool.emplace_back(tracking_object);

			//Erase overlapped tracking objects
			sort(overlapped_indexes.rbegin(), 
				 overlapped_indexes.rend());
			
			for (auto& index : overlapped_indexes)
				pool.erase(pool.begin() + index);

			cout << "Object branched" << endl;
	
		}
			
	}

	//Remove object which has long duration	
	for (auto iter = pool.begin();
			  iter != pool.end();) {

		if (!iter->is_valid()) {

			iter = pool.erase(iter);
			cout << "ERASE ==" << endl;
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
		/*putText(showing_frame, to_string(object.get_number()), 
			Point(object.get_object().x + object.get_object().width / 3, 
				  object.get_object().y + object.get_object().height / 3), 3, 0.5, Scalar{ 255, 0, 0 });*/
	}
		
			
	imshow("Tracking objects", showing_frame);

}


void TrackingObjectPool::reset_overlap_points() {

	for (auto& tracking_object : pool)
		tracking_object.set_overlap_point(0);

}







