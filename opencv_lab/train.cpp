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
using namespace cv;
using namespace xfeatures2d;
using namespace ml;

namespace fs = experimental::filesystem;

const int DATA_WIDTH = 160;
const int DATA_HEIGHT = 160;

//	Allocate increment number for the directory files
void number_files
(
	const fs::path& dir_path
)
{
	vector<string> file_names;

	if (!fs::exists(dir_path))
		return;

	fs::directory_iterator end_iter;

	for
(
		fs::directory_iterator iter(dir_path);
		iter != end_iter;
		++iter
	)
	{
		if (fs::is_directory(iter->status()))
		{
			continue;
		}

		file_names.push_back(iter->path().string().substr(dir_path.string().size() + 1));
	}
	
	vector<string> new_names;

	for (int i = 0; i < file_names.size(); ++i) {
		string new_name = dir_path.string() + "//" + to_string(i) + ".jpg";
		if (!ifstream(new_name).good())
			new_names.emplace_back(new_name);
	}
		
	for (int i = 0; i < file_names.size(); ++i) {
		string current_name = dir_path.string() + "//" + file_names.at(i);
		auto left_side_removed = current_name.substr(0, current_name.find_first_of('.'));
		auto both_side_removed = current_name.substr(
														left_side_removed.find_last_of('/') + 1, 
									                    left_side_removed.size() - (left_side_removed.find_last_of('/') + 1)
													);
		auto file_number = stoi(both_side_removed);

		if (0 <= file_number && file_number < file_names.size())
			continue;
		
		rename(current_name.c_str(), new_names.back().c_str());
		new_names.pop_back();
	}

	cout << "Total: " << file_names.size() << endl;
}

//	Normalize keypoints as size
void normalize_keypoints
(
	vector<KeyPoint>& keypoints, 
	const int size
) 
{

	auto current_size = keypoints.size();
	auto loop = size / 2;
	
	if (current_size < size)
		return;
	
	vector<KeyPoint> copy;

	for (int i = 0; i < loop; ++i) {
		copy.emplace_back(keypoints[current_size - 1 - i]);
		copy.emplace_back(keypoints[i]);
	}

	keypoints = copy;

}

void main() {
	
	Mat groups;
	Mat samples;
	vector<KeyPoint> keypoints;

	Mat descriptors;

	Ptr<SURF> extractor = SURF::create();

	number_files("training_data");

	//Sample of similar images
	for (int i = 0; i <= 3000; ++i) {
		stringstream nn;
		nn << "training_data/" << i << ".jpg";

		if (!ifstream(nn.str()).good()) {
			continue;
		}
		
		Mat train_x = imread(nn.str());
		cvtColor(train_x, train_x, CV_BGR2GRAY);

		FAST(train_x, keypoints, 3);

		//DEBUG: Before normalization
		Mat train_x_before_norm = train_x.clone();
		drawKeypoints(train_x_before_norm, keypoints, train_x_before_norm);
		imshow("Original Keypoints", train_x_before_norm);
		moveWindow("Original Keypoints", 0, 0);
		waitKey(10);

		normalize_keypoints(keypoints, 16);
		
		//DEBUG: After normalizations
		Mat train_x_after_norm = train_x.clone();
		drawKeypoints(train_x_after_norm, keypoints, train_x_after_norm);
		imshow("Normalized Keypoints", train_x_after_norm);
		moveWindow("Normalized Keypoints", 160, 0);
		waitKey(10);
		
		extractor->compute(train_x, keypoints, descriptors);
		descriptors = descriptors.reshape(1, 1);
		
		try {
			samples.push_back(descriptors);
			groups.push_back(0);
		}
		catch (Exception e) {
			cout << "Exception - " + nn.str() << endl;
		}

		keypoints.clear();

	}
	
	Ptr<SVM> classifierSVM = SVM::create();

	classifierSVM->setType(SVM::ONE_CLASS);
	classifierSVM->setKernel(SVM::RBF);
	classifierSVM->setDegree(3);
	classifierSVM->setNu(0.05);
	classifierSVM->setGamma(0.1);
	classifierSVM->setCoef0(0);
	classifierSVM->setP(0);
	classifierSVM->setTermCriteria(cvTermCriteria(CV_TERMCRIT_ITER,
		500, FLT_EPSILON));
	classifierSVM->train(samples, ml::ROW_SAMPLE, groups);
	
	classifierSVM->save("classifier.yml");

	waitKey(10);

}