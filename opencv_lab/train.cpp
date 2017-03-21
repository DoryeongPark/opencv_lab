#include<iostream>
#include<vector>
#include<fstream>
#include<istream>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>
#include<opencv2\xfeatures2d.hpp>
#include<opencv2\ml.hpp>

#include<Windows.h>
#include<experimental/filesystem>

using namespace std;
using namespace experimental::filesystem;
using namespace cv;
using namespace xfeatures2d;
using namespace ml;

void get_file_names(const string& directory) {
	
	vector<string> file_names;

	if (!exists(directory))
		return;
	
	path directory(directory);
	directory_iterator dir_iter(directory), end;
	


}

void normalize_keypoints(vector<KeyPoint>& keypoints, int size) {
	
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

void main() {

	int width = 300;
	int height = 400;

	Mat groups;
	Mat samples;
	vector<KeyPoint> keypoints;

	Mat descriptors;

	Ptr<SURF> extractor = SURF::create();

	vector<string> file_names;

	get_file_names();

	//Sample of similar images
	for (int i = 1; i <= 1000; ++i) {
		stringstream nn;
		nn << "training_data/" << i << ".jpg";

		if (!ifstream(nn.str()).good()) {
			continue;
		}
		
		Mat img = imread(nn.str());
		
		FAST(img, keypoints, 4);
		normalize_keypoints(keypoints, 15);
		
		extractor->compute(img, keypoints, descriptors);

		drawKeypoints(img, keypoints, img);
		imshow("Training Data", img);
		waitKey(10);

		descriptors = descriptors.reshape(1, 1);
		
		try {
			samples.push_back(descriptors);
			groups.push_back(0);
		}
		catch (Exception e) {
			cout << "Remove - " + nn.str() << endl;
		}

		keypoints.clear();

	}
	
	Ptr<SVM> classifierSVM = SVM::create();

	classifierSVM->setType(SVM::ONE_CLASS);
	classifierSVM->setKernel(SVM::RBF);
	classifierSVM->setDegree(3);
	classifierSVM->setGamma(0.1);
	classifierSVM->setCoef0(0);
	classifierSVM->setC(1);
	classifierSVM->setNu(0.1);
	classifierSVM->setP(0);
	classifierSVM->setTermCriteria(cvTermCriteria(CV_TERMCRIT_ITER,
		500, FLT_EPSILON));

	classifierSVM->train(samples, ml::ROW_SAMPLE, groups);
	classifierSVM->save("classifier.yml");

	waitKey(10);
}