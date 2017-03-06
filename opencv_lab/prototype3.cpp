#include<vector>
#include<deque>
#include<iostream>
#include<windows.h>
#include<fstream>
#include<istream>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>
#include<opencv2\xfeatures2d.hpp>
#include<opencv2\ml.hpp>

static int matarial_count = 850;
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
			|| second_x == -1 || second_y == -1 ) {
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
void save_objects(Mat& cframe, Mat& cframe_gray, Mat& binary, vector<Rect>& final_rects) noexcept;
void classify(Mat& cframe, Mat& cframe_gray, Mat& binary, vector<Rect>& final_rects) noexcept;
void train(Mat& data);

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
		detector->detect(cropped_mat, keypoints);
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

void save_objects(Mat& cframe, Mat& cframe_gray, Mat& binary, vector<Rect>& final_rects) noexcept{

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
		
		imwrite(to_string(++matarial_count) + ".jpg", result);
	}

}

void classify(Mat& cframe, Mat& cframe_gray, Mat& binary, vector<Rect>& final_rects) noexcept {
	
	for (auto iter = final_rects.begin(); iter != final_rects.end();) {
		int width = iter->width;
		int height = iter->height;
		float ratio = (float)(DATA_HEIGHT / 2) / (float)height;

		int modified_width = (int)((float)iter->width * ratio);
		int modified_height = DATA_HEIGHT / 2;

		if (modified_width > DATA_WIDTH) {
			++iter;
			continue;
		}

		Mat object_cframe = cframe_gray(*iter);
		Mat object_binary = binary(*iter);

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

		//Check with classifier
		keypoints.clear();
		detector->detect(result, keypoints);
		extractor->compute(result, keypoints, descriptors);
		descriptors = descriptors.reshape(1, 1);
		
		if (descriptors.cols != classifier->getVarCount()) {
			iter = final_rects.erase(iter);
			continue;
		}

		float is_matching = classifier->predict(descriptors.reshape(1,1));

		Mat result_clone = result.clone();

		Mat cframe_clone = cframe.clone();
		rectangle(cframe_clone, *iter, Scalar(0, 0, 255));

		if (is_matching != 1) {
			iter = final_rects.erase(iter);
			putText(result_clone, "NOT CORRECT", Point(20, 20), 1, 0.8, Scalar(255));
		}
		else {
			putText(result_clone, "CORRECT", Point(20, 20), 1, 0.8, Scalar(255));
			++iter;
		}

		imshow("Input", cframe_clone);
		imshow("Result", result_clone);

  		char ch = waitKey(10);

 		while ((ch = waitKey(10)) != 32 && ch != 27);

		if (ch == 32) { continue; }

   	}
	cvDestroyWindow("Result");
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

	if (!ifstream("classifier.yml")) {
		cout << "Classifier doesn't exist" << endl;
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
	Mat expander = getStructuringElement(MORPH_ELLIPSE, Size{ 5, 5 }, Point{ 2, 2 });
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
		threshold(binary, binary, 22, 255, THRESH_BINARY);

		accumulateWeighted(cframe_gray, accumulator, 0.001, mask);
		convertScaleAbs(accumulator, background);

		morphologyEx(binary, binary, MORPH_OPEN, noise_remover);
		morphologyEx(binary, binary, MORPH_CLOSE, expander);
		
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

		for (auto standard_rect = bounded_rects.begin(); standard_rect != bounded_rects.end(); ++standard_rect) {

			int width = standard_rect->width * 3;
			int height = standard_rect->height * 3;
			int x = standard_rect->x - standard_rect->width * 3 / 4;
			int y = standard_rect->y - standard_rect->height * 3 / 4;

			if (x < 0) x = 0;
			if (y < 0) y = 0;
			if (x + width >= 320) width = 320 - x;
			if (y + height >= 240) height = 240 - y;

			Rect expanded_rect(x, y, width, height);

			for (auto another_rect = bounded_rects.begin(); another_rect != bounded_rects.end(); ++another_rect) {
				if (*standard_rect == *another_rect)
					continue;

				if (expanded_rect.contains(Point{ another_rect->x + another_rect->width / 2,
					another_rect->y + another_rect->height / 2 })) {
					if (standard_rect->area() / 8 > another_rect->area()) {
						//Remove Noise of Binary image(Small Rect)
						expand_rect(*another_rect, 2);
						threshold(binary(*another_rect), binary(*another_rect), 0, 0, CV_THRESH_BINARY);
						
						//Remove Rect object 
						bounded_rects.erase(another_rect);
						another_rect = bounded_rects.begin();
						standard_rect = bounded_rects.begin();
					}
				}
			}
		}

		for (Rect& final_rect : bounded_rects) {
			expand_rect(final_rect, 4);
		}
	
		putText(cframe, "Count: " + to_string(bounded_rects.size()),
			Point{ 15, 15 },
			3,
			0.5,
			Scalar{ 255, 0, 0 });

		imshow("Input", cframe);
		imshow("Background", background);
		imshow("Binary", binary);
		
		char ch = waitKey(10);

		//Space key
		if(ch == 32){                
			//Save all objects detected as file
			save_objects(cframe, cframe_gray, binary, bounded_rects);
			classify(cframe, cframe_gray, binary, bounded_rects);
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

