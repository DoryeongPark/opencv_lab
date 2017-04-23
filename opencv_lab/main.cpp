#include<iostream>
#include<windows.h>
#include<fstream>
#include<istream>
#include<iterator>
#include<algorithm>
#include<queue>
#include<vector>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>

#include"tracking.hpp"

static bool drag_mouse = false;
static bool b_flag = false;

constexpr int FAST_N = 9;
constexpr int NORMALIZATION_SIZE = 12;

constexpr int WIDTH = 400;
constexpr int HEIGHT = 300;

constexpr int DATA_WIDTH = 160;
constexpr int DATA_HEIGHT = 160;

static int frame_number = 0;

using namespace std;
using namespace cv;
using namespace tracking;

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

	void determine_first_coordinate
	(
		const int& x, 
		const int& y
	) 
	{
		first_x = x;
		first_y = y;
	}

	void determine_second_coordinate
	(
		const int& x, 
		const int& y
	) 
	{
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

bool is_overlapped
(
	const Rect& object,
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

		drag_mouse = true;
		cropper->determine_first_coordinate(x, y);

	}
	else if (callback_event == CV_EVENT_MOUSEMOVE && drag_mouse) {

		Mat copied_cframe;
		cropper->get_cframe().copyTo(copied_cframe);

		rectangle(copied_cframe,
			Rect(cropper->get_first_point(), Point(x, y)),
			Scalar(0, 255, 0));
		imshow("Input", copied_cframe);

	}
	else if (callback_event == CV_EVENT_LBUTTONUP) {

		drag_mouse = false;
		cropper->determine_second_coordinate(x, y);
		Mat cropped_mat = cropper->get_matrix();
		resize(cropped_mat, cropped_mat, Size(DATA_WIDTH, DATA_HEIGHT), 0, 0, CV_INTER_CUBIC);
		cvtColor(cropped_mat, cropped_mat, CV_BGR2GRAY);
	
		imshow("Data", cropped_mat);
		imshow("Input", cropper->get_cframe());

	}
}

/* 
	Calculate angle from two points
	Standard: x1, y1
	Returns: -180 ~ 180 [Top: Plus, Bottom: Minus, Right: 0 ~ 90, Left: 90 ~ 180]
*/
float calculate_angle
(
	const int& x1, const int& y1, 
	const int& x2, const int& y2
)
noexcept
{
	return atan2(static_cast<float>(y1 - y2), 
				 static_cast<float>(x2 - x1)) * 
				 180.0f / 3.141592f;
}


void expand_rect
(
	Rect& rect, 
	const int& pixel
) 
noexcept 
{

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


void display_color_binary
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

	//Video resolution Setting
	VideoCapture camera = VideoCapture{ file_name };
	camera.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

	//Initialize Matrix
	Mat cframe{ HEIGHT, WIDTH, CV_8UC3 };
	Mat gray_cframe{ HEIGHT, WIDTH, CV_8UC1 };
	Mat accumulator = Mat::zeros(Size{ WIDTH, HEIGHT }, CV_32FC1);
	Mat mask{ HEIGHT, WIDTH, CV_8U };
	mask = Scalar{ 1 };

	Mat background{ HEIGHT, WIDTH, CV_8UC1 };
	Mat binary;
	Mat opener = getStructuringElement(MORPH_RECT, Size{ 3, 3 }, Point{ 1, 1 });
	Mat closer = getStructuringElement(MORPH_ELLIPSE, Size{ 9, 11 }, Point{ 4, 5 });
	
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
		cvtColor(cframe, gray_cframe, COLOR_BGR2GRAY);
		accumulateWeighted(gray_cframe, accumulator, 1, mask);

	}

	//Initialize Cropper 
	Cropper cropper;

	//Initialize TrackingObjectPool
	TrackingObjectPool tracking_object_pool;

	//Playing video file
	while (true) {
		camera.read(cframe);

		if (cframe.empty())
			break;

		//Mouse event callback setting 
		cropper.update_frame(cframe);
		setMouseCallback("Input", on_mouse, &cropper);

		cvtColor(cframe, gray_cframe, COLOR_BGR2GRAY);

		absdiff(background, gray_cframe, binary);
		threshold(binary, binary, 16, 255, THRESH_BINARY);

		////DEBUG: Before erosion
		/*imshow("Before Erosion", binary);
		waitKey(10);*/
		
		morphologyEx(binary, binary, MORPH_OPEN, opener);

		////DEBUG: Before dilation
		/*imshow("Before Dilation", binary);
		waitKey(10);*/

		morphologyEx(binary, binary, MORPH_DILATE, closer);
		morphologyEx(binary, binary, MORPH_ERODE, opener);

		//Accumulate current frame to background frame
		accumulateWeighted(gray_cframe, accumulator, 0.001, mask);
		convertScaleAbs(accumulator, background);

		findContours
		(
			binary, contours, hierarchy,
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, 
			Point{ 0, 0 }
		);

		drawContours
		(
			binary, contours, -1, cv::Scalar::all(255),
			CV_FILLED, 8, hierarchy, INT_MAX
		);

		int contours_size = contours.size();
		
		vector<vector<Point>> contours_polygon{ (unsigned int)contours_size };
		vector<Rect> detected_objects{ (unsigned int)contours_size };
		
		for (int i = 0; i < contours_size; ++i) {

			approxPolyDP(Mat{ contours[i] }, contours_polygon[i], 1, true);
			detected_objects[i] = boundingRect(Mat{ contours_polygon[i] });

		}

		//Test Code
		for (int i = 0; i < contours_size; ++i) {
			
			for (int j = 0; j < contours_size; ++j) {
				
				if (i == j)
					continue;

				if (is_overlapped(detected_objects[i], detected_objects[j]))
					cout << "중복 발생" << endl;

			}
			
		}

		if (frame_number++ > 30) {

			tracking_object_pool.reflect(detected_objects);
			tracking_object_pool.display_objects(cframe);

		}

		cout << tracking_object_pool.get_counts() << endl;
		
		char ch;
		ch = waitKey(10);

		//B Key - For brend color and binary
		if (ch == 66 || ch == 98)
			b_flag = !b_flag;

		if(b_flag)
			display_color_binary(cframe, binary);

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
		//imshow("Background", background);
		//imshow("Binary", binary);
		waitKey(10);

	}
	
	destroyAllWindows();
	imwrite("Background.jpg", accumulator);

}

