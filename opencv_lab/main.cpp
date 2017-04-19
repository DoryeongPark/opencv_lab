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
#include<opencv2\xfeatures2d.hpp>
#include<opencv2\ml.hpp>

static bool drag_mouse = false;
static bool b_flag = false;

constexpr int FAST_N = 9;
constexpr int NORMALIZATION_SIZE = 12;

constexpr int WIDTH = 400;
constexpr int HEIGHT = 300;

constexpr int DATA_WIDTH = 160;
constexpr int DATA_HEIGHT = 160;

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using namespace ml;

int material_count = 3000;
int test_frame_number = 1;

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
		//detector->detect(cropped_mat, keypoints);
		FAST(cropped_mat, keypoints, FAST_N);
		extractor->compute(cropped_mat, keypoints, descriptor);
		drawKeypoints(cropped_mat, keypoints, cropped_mat);

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
		threshold(binary, binary, 13, 255, THRESH_BINARY);

		////DEBUG: Before erosion
		//imshow("Before Erosion", binary);
		//waitKey(10);
		
		morphologyEx(binary, binary, MORPH_OPEN, opener);

		////DEBUG: Before dilation
		//imshow("Before Dilation", binary);
		//waitKey(10);

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
			approxPolyDP(Mat{ contours[i] }, contours_polygon[i], 1, true);
			bounded_rects[i] = boundingRect(Mat{ contours_polygon[i] });
		}

		char ch;
		ch = waitKey(10);

		//B Key - For brend color and binary
		if (ch == 66 || ch == 98)
			b_flag = !b_flag;

		if(b_flag)
			brend_color_and_binary(cframe, binary);

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
		/*imshow("Background", background);
		imshow("Binary", binary);*/
		waitKey(10);

	}
	
	destroyAllWindows();
	imwrite("Background.jpg", accumulator);
}

