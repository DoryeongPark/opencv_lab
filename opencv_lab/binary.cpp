#include<vector>
#include<deque>
#include<iostream>
#include<windows.h>
#include<fstream>
#include<istream>
#include<algorithm>
#include<functional>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>
#include<opencv2\xfeatures2d.hpp>
#include<opencv2\ml.hpp>

static bool drag = false;

constexpr int WIDTH = 320;
constexpr int HEIGHT = 240;

constexpr int DATA_WIDTH = 240;
constexpr int DATA_HEIGHT = 320;

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using namespace ml;

//ORB Detector & SURF extractor
Ptr<ORB> detector = ORB::create(15, 1.2f, 15, 10, 0, 2, ORB::HARRIS_SCORE, 31, 20);
Mat descriptors;
Ptr<SURF> extractor = SURF::create();
Ptr<SVM> classifier = SVM::create();

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
void expand_rect(Rect& rect, const int& pixel) noexcept;
void display_rects(Mat& cframe, Mat& binary, vector<Rect>& rects) noexcept;
void expand_object_roi(Mat& cframe_gray, vector<Rect>& rects) noexcept;

void on_mouse(int callback_event, int x, int y, int flags, void* param) {

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
		extractor->compute(cropped_mat, keypoints, descriptors);
		drawKeypoints(cropped_mat, keypoints, cropped_mat);

		imshow("Data", cropped_mat);
		imshow("Input", cropper->get_cframe());
	}
}

void expand_rect(Rect& rect, const int& pixel) noexcept {

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

void display_rects(Mat& cframe, Mat& binary, vector<Rect>& rects) noexcept {
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

void expand_object_roi(Mat& cframe_gray, vector<Rect>& rects) noexcept {
	
	for (auto iter = rects.begin(); iter != rects.end();) {
		
		if (iter->width <= 1 || iter->height <= 1) {
			++iter;
			continue;
		}

		Mat roi = cframe_gray(*iter);
		int roi_x = iter->x;
		int roi_y = iter->y;
		int roi_width = iter->width;
		int roi_height = iter->height;
		
		Mat histogram;
		const int* channel_numbers = { 0 };
		float channel_range[] = { 0, 256 };
		const float* channel_ranges = channel_range;
		int hist_size = 256;

		//Calculate TOP 1 Pixel Value 
		calcHist(&roi, 1, channel_numbers, Mat(), histogram, 1, &hist_size, &channel_ranges);

		int rank1_px = 0;
		int rank1_cnt = -1;

		for (int i = 0; i < histogram.rows; ++i) {
			float current_cnt = histogram.at<float>(i, 0);
			if (current_cnt > rank1_cnt) {
				rank1_px = i;
				rank1_cnt = current_cnt;
			}
		}

		int scope = 10;

		imshow("Before Extension", cframe_gray(*iter));
		waitKey(10);

		//Left Extension
		Mat left_mat = roi(Rect(0, 0, 1, roi_height));
		vector<int> left_indexes;

		for (int i = 0; i < left_mat.rows; ++i)
			left_indexes.emplace_back(i);
		int idx_count = left_indexes.size();
		
		while(idx_count != 0) {
			for (int i = 0; i < idx_count; ++i) {
				float current_px = static_cast<float>(left_mat.at<uchar>(left_indexes.at(i), 0));
				if (rank1_px - scope < current_px && current_px < rank1_px + scope) {
					left_indexes.emplace_back(i);
					if (0 <= i - 1)
						left_indexes.emplace_back(i - 1);
					if(i + 1 < idx_count)
						left_indexes.emplace_back(i + 1);
				}
			}
			left_indexes.erase(left_indexes.begin(), left_indexes.begin() + idx_count);
			sort(left_indexes.begin(), left_indexes.end(), greater<int>());
			auto cut_point = unique(left_indexes.begin(), left_indexes.end());
			left_indexes.erase(cut_point, left_indexes.end());
			idx_count = left_indexes.size();
			if (idx_count != 0) {
				if (roi_x == 0)
					break;
				*iter = Rect(--roi_x, roi_y, ++roi_width, roi_height);
				roi = cframe_gray(*iter);
				left_mat = roi(Rect(0, 0, 1, roi_height));
			}
		}

		imshow("After Extension", cframe_gray(*iter));
		waitKey(10);

		++iter;
	}
}

int main() {

	cout << "Object Counting Application [Demo version] - Made by Doryeong Park" << endl;

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
	camera.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, 240);

	//Initialize Matrix
	Mat cframe{ 240, 320, CV_8UC3 };
	Mat cframe_gray{ 240, 320, CV_8UC1 };
	Mat accumulator = Mat::zeros(Size{ 320, 240 }, CV_32FC1);
	Mat mask{ 240, 320, CV_8U };
	mask = Scalar{ 1 };

	Mat background{ 240, 320, CV_8UC1 };
	Mat binary;
	Mat noise_remover = getStructuringElement(MORPH_RECT, Size{ 3, 3 }, Point{ 1, 1 });
	Mat expander = getStructuringElement(MORPH_RECT, Size{ 3, 3 }, Point{ 1, 1 });
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
		threshold(binary, binary, 25, 255, THRESH_BINARY);

		morphologyEx(binary, binary, MORPH_ERODE, noise_remover);
		morphologyEx(binary, binary, MORPH_DILATE, expander);

		accumulateWeighted(cframe_gray, accumulator, 0.001, mask);
		convertScaleAbs(accumulator, background);

		findContours(
			binary, contours, hierarchy,
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point{ 0, 0 });

		vector<vector<Point>> contours_polygon{ contours.size() };
		vector<Rect> bounded_rects{ contours.size() };

		drawContours(
			binary, contours, -1, cv::Scalar::all(255),
			CV_FILLED, 8, hierarchy, INT_MAX
		);

		for (int i = 0; i < contours.size(); ++i) {
			approxPolyDP(Mat{ contours[i] }, contours_polygon[i], 3, true);
			bounded_rects[i] = boundingRect(Mat{ contours_polygon[i] });
		}

		/*for (Rect& final_rect : bounded_rects) {
			expand_rect(final_rect, 4);
		}*/

		
		imshow("Input", cframe);
		imshow("Background", background);
		imshow("Binary", binary);

		char ch = waitKey(10);

		//Space key
		if (ch == 32) {
			//display_rects(cframe, binary, bounded_rects);
			expand_object_roi(cframe_gray, bounded_rects);
			while ((ch = waitKey(10)) != 32 && ch != 27);
			if (ch == 27) break;
		}

		//ESC key
		if (ch == 27)
			break;

		Sleep(18);

	}

	destroyAllWindows();
	imwrite("Background.jpg", accumulator);
}

