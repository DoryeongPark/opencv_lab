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
static bool b_flag = false;

constexpr int WIDTH = 400;
constexpr int HEIGHT = 300;

constexpr int DATA_WIDTH = 240;
constexpr int DATA_HEIGHT = 320;

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using namespace ml;

int material_count = 200;

//ORB Detector & SURF extractor
Ptr<ORB> detector = ORB::create(15, 1.2f, 15, 10, 0, 2, ORB::HARRIS_SCORE, 31, 20);
Mat descriptor;
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

//Mouse Callback function
void on_mouse(int callback_event, int x, int y, int flags, void* param);
void expand_rect(Rect& rect, const int& pixel) noexcept;
void display_rects(Mat& cframe, Mat& binary, vector<Rect>& rects) noexcept;
void expand_object_roi(Mat& cframe_gray, Mat& binary, vector<Rect>& rects, int scope) noexcept;

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
		extractor->compute(cropped_mat, keypoints, descriptor);
		drawKeypoints(cropped_mat, keypoints, cropped_mat);

		imshow("Data", cropped_mat);
		imshow("Input", cropper->get_cframe());

	}
}


//Get area which has maximum points - Can be deprecated
int get_max_area8(Mat histogram) noexcept {

	int area_number = 0;
	int max_area_number = 0;
	float current_pixel_total = 0;
	float max_pixel_total = 0;

	for (int i = 0; i < histogram.rows; ++i) {
		if (i % 32 == 0 && i != 0) {
			if (current_pixel_total > max_pixel_total) {
				max_pixel_total = current_pixel_total;
				max_area_number = area_number;
			}
			++area_number;
			current_pixel_total = 0;
		}
		float current_pixel = histogram.at<float>(i, 0);
		current_pixel_total += current_pixel;
	}

	return max_area_number * 32;

}

void save_objects_as_file(Mat& cframe, Mat& cframe_gray, Mat& binary, vector<Rect>& final_rects) noexcept {

	for (Rect& rect : final_rects) {
		int width = rect.width;
		int height = rect.height;
		float ratio = (float)(DATA_HEIGHT / 2) / (float)height;

		int modified_width = (int)((float)rect.width * ratio);
		int modified_height = DATA_HEIGHT / 2;

		if (modified_width > DATA_WIDTH)
			continue;

		Mat object_cframe = cframe_gray(rect);
		Mat object_binary = binary(rect);

		resize(object_cframe, object_cframe, Size(modified_width, modified_height), 0, 0, CV_INTER_CUBIC);
		resize(object_binary, object_binary, Size(modified_width, modified_height), 0, 0, CV_INTER_AREA);

		threshold(object_binary, object_binary, 0, 1, CV_THRESH_BINARY);
		object_cframe = object_cframe.mul(object_binary);

		Mat background = Mat(DATA_HEIGHT, DATA_WIDTH, CV_8UC1);
		background = Scalar::all(0);

		Mat result = background.clone();

		Rect roi;
		roi.width = modified_width;
		roi.height = modified_height;
		roi.x = (DATA_WIDTH - modified_width) / 2;
		roi.y = (DATA_HEIGHT - modified_height) / 2;

		Mat background_roi = background(roi);

		addWeighted(background_roi, 0, object_cframe, 1, 0.0, result(roi));

		imwrite(to_string(++material_count) + ".jpg", result);

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

void expand_object_roi(Mat& cframe_gray, Mat& binary, vector<Rect>& rects, int scope) noexcept {
	
	for (auto iter = rects.begin(); iter != rects.end();) {		
		if (iter->width <= 1 || iter->height <= 1) {
			++iter;
			continue;
		}

		Mat roi_gray = cframe_gray(*iter);
		Mat roi_binary = binary(*iter);

		int roi_x = iter->x;
		int roi_y = iter->y;
		int roi_width = iter->width;
		int roi_height = iter->height;
		
		Mat histogram;
		const int* channel_numbers = { 0 };
		float channel_range[] = { 0, 256 };
		const float* channel_ranges = channel_range;
		int hist_size = 256;

		//Calculate TOP 1 scope
		calcHist(&roi_gray, 1, channel_numbers, Mat(), histogram, 1, &hist_size, &channel_ranges);
		int min_scope = get_max_area8(histogram);
	
		//-----------------------------------------------------------------------------------
		//DEBUG - This code is to show improvement of cropping entire part [Before Extension]
		//-----------------------------------------------------------------------------------

		destroyWindow("Before Extension");
		imshow("Before Extension", cframe_gray(*iter));
		waitKey(10);
		cout << "Before Extension" << endl;
		cout << "---------------" << endl;
		cout << "x: " << iter->x << " " << "y: " << iter->y << " " << endl;
		cout << "Width: " << iter->width << endl;
		cout << "Height: " << iter->height << endl;
		cout << "---------------" << endl << endl;

		vector<int> index_vector;

		//Left Extension
		Mat left_mat_gray = roi_gray(Rect(0, 0, 1, roi_height));

		for (int i = 0; i < left_mat_gray.rows; ++i)
			index_vector.emplace_back(i);
		int idx_count = index_vector.size();
		
		while(idx_count != 0) {
			for (int i = 0; i < idx_count; ++i) {
				float current_px = static_cast<float>(left_mat_gray.at<uchar>(index_vector.at(i), 0));
				if (min_scope <= current_px && current_px < min_scope + 32) {
					index_vector.emplace_back(i);
					if (0 <= i - 1)
						index_vector.emplace_back(i - 1);
					if(i + 1 < idx_count)
						index_vector.emplace_back(i + 1);
				}
			}
			index_vector.erase(index_vector.begin(), index_vector.begin() + idx_count);
			sort(index_vector.begin(), index_vector.end());
			auto cut_point = unique(index_vector.begin(), index_vector.end());
			index_vector.erase(cut_point, index_vector.end());
			idx_count = index_vector.size();
			if (idx_count != 0) {
				if (roi_x == 0)
					break;
				*iter = Rect(--roi_x, roi_y, ++roi_width, roi_height);
				roi_gray = cframe_gray(*iter);
				
				//Reflect indexes to binary image
				roi_binary = binary(*iter);
				for (auto index : index_vector)
					roi_binary.at<uchar>(index, 0) = 255;

				left_mat_gray = roi_gray(Rect(0, 0, 1, roi_height));
			}
		}

		index_vector.clear();

		//Right Extension
		Mat right_mat = roi_gray(Rect(roi_width - 2, 0, 1, roi_height));

		for (int i = 0; i < right_mat.rows; ++i)
			index_vector.emplace_back(i);
		idx_count = index_vector.size();

		while (idx_count != 0) {
			for (int i = 0; i < idx_count; ++i) {
				float current_px = static_cast<float>(right_mat.at<uchar>(index_vector.at(i), 0));
				if (min_scope <= current_px && current_px < min_scope + 32) {
					index_vector.emplace_back(i);
					if (0 <= i - 1)
						index_vector.emplace_back(i - 1);
					if (i + 1 < idx_count)
						index_vector.emplace_back(i + 1);
				}
			}
			index_vector.erase(index_vector.begin(), index_vector.begin() + idx_count);
			sort(index_vector.begin(), index_vector.end());
			auto cut_point = unique(index_vector.begin(), index_vector.end());
			index_vector.erase(cut_point, index_vector.end());
			idx_count = index_vector.size();
			if (idx_count != 0) {
				if (roi_x + roi_width == cframe_gray.cols)
					break;
				*iter = Rect(roi_x, roi_y, ++roi_width, roi_height);
				roi_gray = cframe_gray(*iter);
				
				////Reflect indexes to binary image
				//roi_binary = binary(*iter);
				//for (auto index : index_vector)
				//	roi_binary.at<uchar>(index, roi_x + roi_width - 1) = 255;

				right_mat = roi_gray(Rect(roi_width - 2, 0, 1, roi_height));
			}
		}
		
		index_vector.clear();

		//Top Extension
		Mat top_mat = roi_gray(Rect(0, 0, roi_width, 1));

		for (int i = 0; i < top_mat.rows; ++i)
			index_vector.emplace_back(i);
		idx_count = index_vector.size();

		while (idx_count != 0) {
			for (int i = 0; i < idx_count; ++i) {
				float current_px = static_cast<float>(top_mat.at<uchar>(0, index_vector.at(i)));
				if (min_scope <= current_px && current_px < min_scope + 32) {
					index_vector.emplace_back(i);
					if (0 <= i - 1)
						index_vector.emplace_back(i - 1);
					if (i + 1 < idx_count)
						index_vector.emplace_back(i + 1);
				}
			}
			index_vector.erase(index_vector.begin(), index_vector.begin() + idx_count);
			sort(index_vector.begin(), index_vector.end());
			auto cut_point = unique(index_vector.begin(), index_vector.end());
			index_vector.erase(cut_point, index_vector.end());
			idx_count = index_vector.size();
			if (idx_count != 0) {
				if (roi_y == 0)
					break;
				*iter = Rect(roi_x, --roi_y, roi_width, ++roi_height);
				roi_gray = cframe_gray(*iter);

				////Reflect indexes to binary image
				//roi_binary = binary(*iter);
				//for (auto index : index_vector)
				//	roi_binary.at<uchar>(0, index) = 255;

				top_mat = roi_gray(Rect(0, 0, roi_width, 1));
			}
		}

		index_vector.clear();

		//Bottom Extension
		Mat bot_mat = roi_gray(Rect(0, roi_height - 2, roi_width, 1));

		for (int i = 0; i < bot_mat.rows; ++i)
			index_vector.emplace_back(i);
		idx_count = index_vector.size();

		while (idx_count != 0) {
			for (int i = 0; i < idx_count; ++i) {
				float current_px = static_cast<float>(bot_mat.at<uchar>(0, index_vector.at(i)));
				if (min_scope <= current_px && current_px < min_scope + 32) {
					index_vector.emplace_back(i);
					if (0 <= i - 1)
						index_vector.emplace_back(i - 1);
					if (i + 1 < idx_count)
						index_vector.emplace_back(i + 1);
				}
			}
			index_vector.erase(index_vector.begin(), index_vector.begin() + idx_count);
			sort(index_vector.begin(), index_vector.end());
			auto cut_point = unique(index_vector.begin(), index_vector.end());
			index_vector.erase(cut_point, index_vector.end());
			idx_count = index_vector.size();
			if (idx_count != 0) {
				if (roi_y == 0)
					break;
				*iter = Rect(roi_x, roi_y, roi_width, ++roi_height);
				roi_gray = cframe_gray(*iter);
				
				////Reflect indexes to binary image
				//roi_binary = binary(*iter);
				//for (auto index : index_vector)
				//	roi_binary.at<uchar>(roi_y + roi_height - 1, index) = 1;

				bot_mat = roi_gray(Rect(0, roi_height - 2, roi_width, 1));
			}
		}

		index_vector.clear();

		//-----------------------------------------------------------------------------------
		//DEBUG - This code is to show improvement of cropping entire part [After Extension]
		//-----------------------------------------------------------------------------------

		destroyWindow("After Extension");
		imshow("After Extension", cframe_gray(*iter));
		waitKey(10);
		cout << "After Extension" << endl;
		cout << "---------------" << endl;
		cout << "x: " << iter->x << " " << "y: " << iter->y << " " << endl;
		cout << "Width: " << iter->width << endl;
		cout << "Height: " << iter->height << endl;
		cout << "---------------" << endl << endl;

		/*expand_rect(*iter, 2);
		rectangle(binary, *iter, Scalar{ 255 });
		imshow("Binary", binary);
		waitKey(10);*/

		++iter;
	}
}

void brend_color_and_binary(Mat& cframe, Mat& binary) {

	for (int i = 0; i < binary.rows; ++i)
		for (int j = 0; j < binary.cols; ++j)
			if ((int)binary.at<uchar>(i, j) == 255) {
				cframe.at<Vec3b>(i, j)[0] = 0;
				cframe.at<Vec3b>(i, j)[1] = 0;
				cframe.at<Vec3b>(i, j)[2] = 255;
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

		findContours(
			binary, contours, hierarchy,
			CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point{ 0, 0 });

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

