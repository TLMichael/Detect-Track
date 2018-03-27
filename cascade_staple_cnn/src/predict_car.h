#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/trace.hpp>
using namespace cv;
using namespace cv::dnn;
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;
/* Find best class for the blob (i. e. class with maximal probability) */
static void getMaxClass(const Mat &probBlob, int *classId, double *classProb)
{
    Mat probMat = probBlob.reshape(1, 1); //reshape the blob to 1x1000 matrix
    Point classNumber;
    minMaxLoc(probMat, NULL, classProb, NULL, &classNumber);
    *classId = classNumber.x;
}
static std::vector<String> readClassNames(const char *filename )
{
    std::vector<String> classNames;
    std::ifstream fp(filename);
    if (!fp.is_open())
    {
        std::cerr << "File with classes labels not found: " << filename << std::endl;
        exit(-1);
    }
    std::string name;
    while (!fp.eof())
    {
        std::getline(fp, name);
        if (name.length())
            classNames.push_back( name.substr(name.find(' ')+1) );
    }
    fp.close();
    return classNames;
}

class CarPredict{
    public:
    void predict(Mat img);
    int getId() { return id; }
    double getProb() { return confidence; }
    string getName() { return name; }
    double getTime() { return costtime; }
    CarPredict();
    //~CarPredict();

    private:
    int id;
    double confidence;
    string name;
    double costtime;
    Net net;
    std::vector<String> classNames;
};


CarPredict::CarPredict()
{
     CV_TRACE_FUNCTION();
    
    String modelTxt = "net.prototxt";
    String modelBin = "model.caffemodel";
    String classNameFile = "words.txt";
    
    try {
        net = dnn::readNetFromCaffe(modelTxt, modelBin);
    }
    catch (const cv::Exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        if (net.empty())
        {
            std::cerr << "Can't load network by using the following files: " << std::endl;
            std::cerr << "prototxt:   " << modelTxt << std::endl;
            std::cerr << "caffemodel: " << modelBin << std::endl;
            std::cerr << "bvlc_googlenet.caffemodel can be downloaded here:" << std::endl;
            std::cerr << "http://dl.caffe.berkeleyvision.org/bvlc_googlenet.caffemodel" << std::endl;
            exit(-1);
        }
    }
    classNames = readClassNames(classNameFile.c_str());
}

void CarPredict::predict(Mat img)
{
    //Mat img = imread(imageFile);
    if (img.empty())
    {
        std::cerr << "img is empty." << std::endl;
        exit(-1);
    }
    //GoogLeNet accepts only 224x224 BGR-images
    Mat inputBlob = blobFromImage(img, 1.0f, Size(64, 64),
                                  Scalar(149, 127, 121), false);   //Convert Mat to batch of images
    net.setInput(inputBlob, "data");        //set the network input
    Mat prob = net.forward("prob");         //compute output
    cv::TickMeter t;
    for (int i = 0; i < 2; i++)
    {
        CV_TRACE_REGION("forward");
        net.setInput(inputBlob, "data");        //set the network input
        t.start();
        prob = net.forward("prob");                          //compute output
        t.stop();
    }
    int classId;
    double classProb;
    getMaxClass(prob, &classId, &classProb);//find the best class
    
    id = classId;
    name = classNames.at(classId);
    confidence = classProb;
    costtime = (double)t.getTimeMilli() / t.getCounter();

    // std::cout << "Best class: #" << classId << " '" << classNames.at(classId) << "'" << std::endl;
    // std::cout << "Probability: " << classProb * 100 << "%" << std::endl;
    // std::cout << "Time: " << (double)t.getTimeMilli() / t.getCounter() << " ms (average from " << t.getCounter() << " iterations)" << std::endl;
}