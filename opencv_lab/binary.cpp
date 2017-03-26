#include<iostream>
#include<windows.h>
#include<fstream>
#include<istream>
#include<algorithm>
#include<queue>
#include<vector>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>
#include<opencv2\xfeatures2d.hpp>
#include<opencv2\ml.hpp>

static bool drag = false;
static bool b_flag = false;

constexpr int WIDTH = 400;
constexpr int HEIGHT = 300;

constexpr int DATA_WIDTH = 160;
constexpr int DATA_HEIGHT = 160;

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using namespace ml;

int material_count = 1300;

//ORB Detector & SURF extractor
Mat descriptor;
Ptr<SURF> extractor = SURF::create();
Ptr<SVM> classifier = Algorithm::load<SVM>("classifier.yml");

Mat groups;
Mat samples;
vector<KeyPoint> keypoints;

class Cropper {
private:
	int first_x = -1;
	int first_y = -1;
	int second_x = -1;
	int second_y = -1;

	Mat cframe;

public:
	void update_frame(Mat& frame) {
		cframe = frame;
	}

	void determine_first_coordinate(const int& x, const int& y) {
		first_x = x;
		first_y = y;
	}

	void determine_second_coordinate(const int& x, const int& y) {
		second_x = x;
		second_y = y;
	}

	bool is_valid() {
		if (first_x == -1 || first_y == -1
			|| second_x == -1 || second_y == -1) {
			return false;
		}
		return true;
	}

	Mat get_matrix() noexcept {
		if (first_x == -1 || first_y == -1
			|| second_x == -1 || second_y == -1)
			return Mat();

		auto rect = Rect(Point(first_x, first_y), Point(second_x, second_y));
		first_x = -1; first_y = -1; second_x = -1; second_y = -1;

		return cframe(rect);
	}

	Mat& get_cframe() {
		return cframe;
	}

	Point get_first_point() {
		return Point(first_x, first_y);
	}

	Point get_second_point() {
		return Point(second_x, second_y);
	}

};

void on_mouse(int callback_event, int x, int y, int flags, void* param);
void create_undetectable_background(Mat& background, Mat& object_roi, Rect roi) noexcept;
void expand_rect(Rect& rect, const int& pixel) noexcept;
void display_rects(Mat& cframe, Mat& binary, vector<Rect>& rects) noexcept;

void on_mouse
(
	int callback_event, 
	int x, 
	int y, 
	int flags, 
	void* param
)
{

	Cropper* cropper = (Cropper*)param;

	if (callback_event == CV_EVENT_LBUTTONDOWN) {

		drag = true;
		cropper->determine_first_coordinate(x, y);

	}
	else if (callback_event == CV_EVENT_MOUSEMOVE && drag) {

		Mat copied_cframe;
		cropper->get_cframe().copyTo(copied_cframe);

		rectangle(copied_cframe,
			Rect(cropper->get_first_point(), Point(x, y)),
			Scalar(0, 255, 0));
		imshow("Input", copied_cframe);

	}
	else if (callback_event == CV_EVENT_LBUTTONUP) {

		drag = false;
		cropper->determine_second_coordinate(x, y);
		Mat cropped_mat = cropper->get_matrix();
		resize(cropped_mat, cropped_mat, Size(DATA_WIDTH, DATA_HEIGHT), 0, 0, CV_INTER_CUBIC);
		cvtColor(cropped_mat, cropped_mat, CV_BGR2GRAY);
		//detector->detect(cropped_mat, keypoints);
		FAST(cropped_mat, keypoints, 3);
		extractor->compute(cropped_mat, keypoints, descriptor);
		drawKeypoints(cropped_mat, keypoints, cropped_mat);

		imshow("Data", cropped_mat);
		imshow("Input", cropper->get_cframe());

	}
}

//	Normalize keypoints as size
void normalize_keypoints
(
	vector<KeyPoint>& keypoints,
	const int& size
)
{

	if (keypoints.size() <= size)
		return;

	int current_size = keypoints.size();
	int counter = 0;
	float interval = static_cast<float>(current_size) / size;

	vector<KeyPoint> copy;
	copy.reserve(keypoints.size());

	while (counter != size) {
		int index = static_cast<int>(interval * (float)counter);
		copy.emplace_back(keypoints[index]);
		++counter;
	}

	keypoints = copy;
}

Mat& refine_data
(
	Mat& cframe,
	Mat& cframe_gray,
	Mat& binary,
	Rect& rect,
	Mat& result
) 
noexcept {

	int width = rect.width;
	int height = rect.height;
	float ratio = ((float)DATA_HEIGHT / 1.2f) / (float)height;

	int modified_width = (int)((float)rect.width * ratio);
	int modified_height = (int)((float)DATA_HEIGHT / 1.2f);

	if (modified_width > DATA_WIDTH)
		return Mat();

	Mat object_cframe = cframe_gray(rect);
	Mat object_binary = binary(rect);

	resize(object_cframe, object_cframe, Size(modified_width, modified_height), 0, 0, CV_INTER_CUBIC);
	resize(object_binary, object_binary, Size(modified_width, modified_height), 0, 0, CV_INTER_AREA);

	threshold(object_binary, object_binary, 0, 1, CV_THRESH_BINARY);
	object_cframe = object_cframe.mul(object_binary);

	result = Mat(DATA_HEIGHT, DATA_WIDTH, CV_8UC1);
	result = Scalar::all(0);

	Rect roi;

	roi.width = modified_width;
	roi.height = modified_height;
	roi.x = static_cast<int>((float)(DATA_WIDTH - modified_width) / 2);
	roi.y = static_cast<int>((float)(DATA_HEIGHT - modified_height) / 2);

	create_undetectable_background(result, object_cframe, roi);
	waitKey(10);

}

void save_objects_as_file
(
	Mat& cframe,
	Mat& cframe_gray,
	Mat& binary,
	vector<Rect>& final_rects
)
noexcept {

	for (Rect& rect : final_rects) {

		Mat result;

		refine_data(cframe, cframe_gray, binary, rect, result);
		
		if (result.empty())
			continue;

		imwrite(to_string(++material_count) + ".jpg", result);
	}

}


void classify
(
	Mat& cframe,
	Mat& cframe_gray,
	Mat& binary,
	vector<Rect>& final_rects
)
noexcept {

	int counter = 0;

	for_each
	(
		final_rects.begin(), 
		final_rects.end(), 
		[&](Rect& rect)->void
	{
		keypoints.clear();

		Mat candidate;

		refine_data(cframe, cframe_gray, binary, rect, candidate);

		if (candidate.empty())
			return;
		
		FAST(candidate, keypoints, 3);
		normalize_keypoints(keypoints, 22);

		if (keypoints.size() != 22)
			return;

		extractor->compute(candidate, keypoints, descriptor);
		descriptor = descriptor.reshape(1, 1);
		
		auto result = classifier->predict(descriptor);
		
		imwrite(to_string(material_count++) + ".jpg", candidate);

		if (result == 1.0f) {
			putText(candidate, "CORRECT", Point(7, 10), 3, 0.25, Scalar(255));
			++counter;
		}
		else {
			putText(candidate, "NOT CORRECT", Point(7, 10), 3, 0.25, Scalar(255));
		}

		imshow("Candidate", candidate);
		moveWindow("Candidate", 0, 0);
		waitKey(10);

		
	}
	);
	
	cout << "-----------------------" << endl;
	cout << final_rects.size() << " Blobs detected" << endl;
	cout << counter << " People detected" << endl;
	cout << "-----------------------" << endl;

	
}

void create_undetectable_background
(
	Mat& input_array, 
	Mat& object_roi, 
	Rect roi
)
noexcept{

	//Blend with black background
	addWeighted(input_array(roi), 0, object_roi, 1, 0.0, input_array(roi));
	
	////DEBUG: Before random number creation
	//vector<KeyPoint> keypoints;
	//Mat descriptors;
	//
	//FAST(input_array, keypoints, 4);
	//extractor->compute(input_array, keypoints, descriptors);
	//Mat input_array_before = input_array.clone();
	//drawKeypoints(input_array_before, keypoints, input_array_before);
	//
	//imshow("Before", input_array_before);
	//moveWindow("Before", 0, 0);
	//waitKey(10);
	
	//Size variables 
	const int BACKGROUND_ROWS = input_array.rows;
 	const int BACKGROUND_COLS = input_array.cols;

	const int OBJECT_ROWS = object_roi.rows;
	const int OBJECT_COLS = object_roi.cols;
	const int OFFSET = (DATA_HEIGHT - OBJECT_ROWS) / 2;

	//Data structures for BFS
	queue<Point> point_queue;

	vector<vector<bool>> point_checker
	(
		BACKGROUND_ROWS, 
		vector<bool>(BACKGROUND_COLS, false)
	);
	
	for (int i = 0; i < BACKGROUND_ROWS; ++i)
		for (int j = 0; j < BACKGROUND_COLS; ++j)
			if (input_array.at<uchar>(i, j) != 0)
				point_checker[i][j] = true;
	
	Point pointer = Point(BACKGROUND_COLS / 2, BACKGROUND_ROWS / 2);

	//Pixel storage 
	vector<uchar> left;
	vector<uchar> right;

	left.reserve(OBJECT_ROWS);
	right.reserve(OBJECT_ROWS);
	
	//Get max point
	for (int i = 0; i < OBJECT_ROWS; ++i) {
		for (int j = 0; j < OBJECT_COLS; ++j)
			if (static_cast<int>(object_roi.at<uchar>(i, j)) != 0) {
				left.emplace_back(object_roi.at<uchar>(i, j));
				break;
			}
		for(int j = OBJECT_COLS - 1; j >= 0; --j)
			if(static_cast<int>(object_roi.at<uchar>(i, j)) != 0){
				right.emplace_back(object_roi.at<uchar>(i, j));
				break;
			}
	}

	int histogram[8] = { 0, 0, 0, 0,
						 0, 0, 0, 0 };
	int max_point = -1;
	int max_value = -1;

	const int VALUE_SCOPE = 16;
	
	for (auto& pixel : left)
		++histogram[(static_cast<int>(pixel) / (VALUE_SCOPE * 2))];

	for (auto& pixel : right)
		++histogram[(static_cast<int>(pixel) / (VALUE_SCOPE * 2))];

	for (int i = 0; i < 8; ++i) 
		if (max_value < histogram[i]) {
			max_value = histogram[i];
			max_point = i;
		}

	const int STANDARD = max_point * (VALUE_SCOPE * 2) + VALUE_SCOPE;
	const int STANDARD_MIN = STANDARD - (VALUE_SCOPE * 2);
	const int STANDARD_MAX = STANDARD + (VALUE_SCOPE * 2);

	//Find start point
	while (point_queue.size() < 3) {
		int x = pointer.x;
		int y = pointer.y;

		for (int i = 1; i >= -1; --i) 
			for (int j = -1; j <= 1; ++j) {
				if (i == 0 && j == 0)
					continue;

				int checked_x = x + i;
				int checked_y = y + j;
				
				if (checked_x < 0 || BACKGROUND_COLS <= checked_x ||
					checked_y < 0 || BACKGROUND_ROWS <= checked_y)
					continue;
					
				if (!point_checker[checked_y][checked_x]) {
 					point_checker[checked_y][checked_x] = true;
					point_queue.push(Point(checked_x, checked_y));
				}
			}
		
		pointer = Point(pointer.x, pointer.y - 1);
	}	

	//Search routine
	while (!point_queue.empty()) {
		
		pointer = point_queue.front();
		point_queue.pop();

		int x = pointer.x;
		int y = pointer.y;

		int sum = 0;
		int count = 0;

		//Search 8-directions
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {

				if (i == 0 && j == 0)
					continue;

				int checked_x = x + i;
				int checked_y = y + j;
							
				if (checked_x < 0 || BACKGROUND_COLS <= checked_x ||
					checked_y < 0 || BACKGROUND_ROWS <= checked_y)
					continue;
				
				auto current_pixel = static_cast<int>(input_array.at<uchar>(checked_y, checked_x));

				if (current_pixel != 0) {
					sum += current_pixel;
					++count;
				}

				if(!point_checker[checked_y][checked_x]){
					point_checker[checked_y][checked_x] = true;
					point_queue.push(Point(checked_x, checked_y));
				}
			}
		}

		//Routine to get average
		if (count != 0) {
			int pixel_value = sum / count;

			if (STANDARD_MIN <= pixel_value &&
				pixel_value <= STANDARD_MAX)
				input_array.at<uchar>(pointer.y, pointer.x) = pixel_value;
			else {
				input_array.at<uchar>(pointer.y, pointer.x) = STANDARD;
			}
		}

		//If there are no points to search but not yet searched all
		if (point_queue.empty() && !point_checker[0][0]){
			while (true) {
				pointer = Point(pointer.x, pointer.y + 1);
				if (!point_checker[pointer.y + 1][pointer.x]){
					point_queue.push(Point(pointer.x, pointer.y + 1));
					break;
				}
			}
		}
	}

	/*Mat blurring_kernel = (
								Mat_<char>(5, 5) <<
								1, 1, 1, 1, 1,
								1, 1, 1, 1, 1,
								1, 1, 1, 1, 1,
								1, 1, 1, 1, 1,
								1, 1, 1, 1, 1
						  );
	Mat cloned_input_array = input_array.clone();
	
	filter2D
	(
		cloned_input_array, 
		input_array, 
		input_array.depth(), 
		blurring_kernel
	);*/

	blur(input_array, input_array, Size(5, 5), Point(3, 3));

	////DEBUG: After random background creation
	//FAST(input_array, keypoints, 4);
	//extractor->compute(input_array, keypoints, descriptors);
	//Mat input_array_after = input_array.clone();
	//drawKeypoints(input_array, keypoints, input_array);
	//imshow("After", input_array);
	//moveWindow("After", 240, 0);
	//waitKey(10);

}

void expand_rect
(
	Rect& rect, 
	const int& pixel
)
noexcept {

	int x1 = rect.x;
	int y1 = rect.y;
	int x2 = rect.x + rect.width;
	int y2 = rect.y + rect.height;

	x1 -= pixel;
	y1 -= pixel;
	x2 += pixel;
	y2 += pixel;

	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 > WIDTH) x2 = WIDTH - 1;
	if (y2 > HEIGHT) y2 = HEIGHT - 1;

	rect.x = x1;
	rect.y = y1;
	rect.width = x2 - x1;
	rect.height = y2 - y1;
}

void display_rects
(
	Mat& cframe, 
	Mat& binary, 
	vector<Rect>& rects
)

noexcept {

	for (auto iter = rects.begin(); iter != rects.end();) {
		Mat cframe_clone = cframe.clone();
		rectangle(cframe_clone, *iter, Scalar(0, 0, 255), 0.5);
		Mat roi = binary(*iter);
		resize(roi, roi, Size(DATA_WIDTH, DATA_HEIGHT), 0, 0, CV_INTER_AREA);
		imshow("Input", cframe_clone);
		imshow("Binary ROI", roi);
		++iter;
		char ch = waitKey(10);
		while ((ch = waitKey(10)) != 32 && ch != 27);
	}
	
}


void brend_color_and_binary
(
	Mat& cframe, 
	Mat& binary
)
{

	for (int i = 0; i < binary.rows; ++i)
		for (int j = 0; j < binary.cols; ++j)
			if ((int)binary.at<uchar>(i, j) == 255) {
				cframe.at<Vec3b>(i, j)[0] = 0;
				cframe.at<Vec3b>(i, j)[1] = 0;
				cframe.at<Vec3b>(i, j)[2] = 255;
			}

}

int main() {

	cout << "People Counting Application [Demo version] - Made by Doryeong Park" << endl;

init:

	//Routine input file name
	string file_name = "";
	cout << "Please input file name: ";
	cin >> file_name;
	if (!ifstream(file_name).good()) {
		cout << "File doesn't exist" << endl;
		goto init;
	}

	//Load classifier
	classifier = Algorithm::load<SVM>("classifier.yml");

	//Video resolution Setting
	VideoCapture camera = VideoCapture{ file_name };
	camera.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

	//Initialize Matrix
	Mat cframe{ HEIGHT, WIDTH, CV_8UC3 };
	Mat cframe_gray{ HEIGHT, WIDTH, CV_8UC1 };
	Mat accumulator = Mat::zeros(Size{ WIDTH, HEIGHT }, CV_32FC1);
	Mat mask{ HEIGHT, WIDTH, CV_8U };
	mask = Scalar{ 1 };

	Mat background{ HEIGHT, WIDTH, CV_8UC1 };
	Mat binary;
	Mat opener = getStructuringElement(MORPH_RECT, Size{ 3, 3 }, Point{ 1, 1 });
	Mat closer = getStructuringElement(MORPH_ELLIPSE, Size{ 7, 9 }, Point{ 3, 4 });
	deque<Point> point_queue;

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	//Background Setting 
	if (ifstream("Background.jpg").good()) {
		Mat loaded_image = imread("Background.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		resize(loaded_image, loaded_image, Size(WIDTH, HEIGHT), 0, 0, CV_INTER_NN);
		accumulateWeighted(loaded_image, accumulator, 1, mask);
	}
	else {
		camera.read(cframe);
		if (cframe.empty())
			return 0;
		cvtColor(cframe, cframe_gray, COLOR_BGR2GRAY);
		accumulateWeighted(cframe_gray, accumulator, 1, mask);
	}

	//Initialize Cropper 
	Cropper cropper;

	//Playing video file
	while (true) {
		camera.read(cframe);

		if (cframe.empty())
			break;

		//Mouse event callback setting 
		cropper.update_frame(cframe);
		setMouseCallback("Input", on_mouse, &cropper);

		cvtColor(cframe, cframe_gray, COLOR_BGR2GRAY);

		absdiff(background, cframe_gray, binary);
		threshold(binary, binary, 16, 255, THRESH_BINARY);

		morphologyEx(binary, binary, MORPH_OPEN, opener);
		morphologyEx(binary, binary, MORPH_DILATE, closer);
		morphologyEx(binary, binary, MORPH_ERODE, opener);

		accumulateWeighted(cframe_gray, accumulator, 0.001, mask);
		convertScaleAbs(accumulator, background);

		findContours
		(
			binary, contours, hierarchy,
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point{ 0, 0 }
		);

		vector<vector<Point>> contours_polygon{ contours.size() };
		vector<Rect> bounded_rects{ contours.size() };

		drawContours
		(
			binary, contours, -1, cv::Scalar::all(255),
			CV_FILLED, 8, hierarchy, INT_MAX
		);

		for (int i = 0; i < contours.size(); ++i) {
			approxPolyDP(Mat{ contours[i] }, contours_polygon[i], 3, true);
			bounded_rects[i] = boundingRect(Mat{ contours_polygon[i] });
		}

		char ch;
		ch = waitKey(10);

		//B Key - For brend color and binary
		if (ch == 66 || ch == 98)
			b_flag = !b_flag;

		if(b_flag)
			brend_color_and_binary(cframe, binary);

		//S key - Save all objects as jpg file
		if (ch == 115 || ch == 83)
			save_objects_as_file(cframe, cframe_gray, binary, bounded_rects);

		//C key - Test classification
		if (ch == 67 || ch == 99) 
			classify(cframe, cframe_gray, binary, bounded_rects);
		
		
		//Space key
		if (ch == 32) {
			//expand_object_roi(cframe_gray, binary, bounded_rects, 13);
			//display_rects(cframe, binary, bounded_rects);
			while ((ch = waitKey(10)) != 32 && ch != 27);
			if (ch == 27) break;
		}

		//ESC key
		if (ch == 27)
			break;

		imshow("Input", cframe);
		imshow("Background", background);
		imshow("Binary", binary);
		waitKey(10);

	}
	
	destroyAllWindows();
	imwrite("Background.jpg", accumulator);
}

