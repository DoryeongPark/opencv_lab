#include<iostream>
#include<vector>
#include<fstream>
#include<istream>

#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<opencv2\highgui.hpp>
#include<opencv2\xfeatures2d.hpp>
#include<opencv2\ml.hpp>

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using namespace ml;

void main() {

	int width = 240;
	int height = 320;

	Mat groups;
	Mat samples;
	vector<KeyPoint> keypoints1;
	
	//ORB Detector
	Ptr<ORB> detector = ORB::create(30, 1.2f, 8, 15, 0, 2, ORB::HARRIS_SCORE, 31, 20);

	Mat descriptors1, descriptors2;

	Ptr<SURF> extractor = SURF::create();

	//Sample of similar images
	for (int i = 1; i <= 900; ++i) {
		stringstream nn;
		nn << "pedestrian_sample/" << i << ".jpg";

		if (!ifstream(nn.str()).good()) {
			continue;
		}
		
		Mat img = imread(nn.str());
		
		detector->detect(img, keypoints1);

		extractor->compute(img, keypoints1, descriptors1);

		drawKeypoints(img, keypoints1, img);
		imshow("Sample", img);
		waitKey(10);

		descriptors1 = descriptors1.reshape(1, 1);
		
		try {
			samples.push_back(descriptors1);
			groups.push_back(0);
		}
		catch (Exception e) {
			cout << "Remove - " + nn.str() << endl;
		}

		keypoints1.clear();

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
	
	////Test for 10 test samples - 7 & 9 must be classified.
	//for (int i = 1; i <= 10; ++i) {
	//	stringstream nn;

	//	nn << "testers/" << "unknown" << i << ".png";

	//	Mat unknown = imread(nn.str());
	//	Mat grayscaledUnknown;

	//	resize(unknown, unknown, Size(width, height), 0, 0, CV_INTER_NN);
	//	cvtColor(unknown, grayscaledUnknown, COLOR_BGR2GRAY);
	//	
	//	detector->detect(grayscaledUnknown, keypoints1);

	//	extractor->compute(grayscaledUnknown, keypoints1, descriptors2);

	//	float result = classifierSVM->predict(descriptors2.reshape(1, 1));

	//	if (result == 1)
	//		putText(unknown, "CORRECT", Point(20, 20), 1, 0.8, Scalar(0, 255, 0));
	//	else
	//		putText(unknown, "NOT CORRECT", Point(20, 20), 1, 0.8, Scalar(0, 0, 255));
	//	
	//	drawKeypoints(unknown, keypoints1, unknown);
	//	imshow("Result", unknown);
	//	waitKey(10);

	//	cout << nn.str() << ": class " << result << endl;

	//	keypoints1.clear();
	//}

	waitKey(10);

}