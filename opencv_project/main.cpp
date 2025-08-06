#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main() {
    VideoCapture video(0); // Use 0 for your default camera
    if (!video.isOpened()) {
        cout << "Error opening video stream" << endl;
        return -1;
    }

    CascadeClassifier facedetect;
    if (!facedetect.load("haarcascade_frontalface_default.xml")) {
        cout << "Error loading cascade classifier" << endl;
        return -1;
    }

    Mat img;
    while (true) {
        video.read(img);
        if (img.empty()) {
            cout << "Empty frame received" << endl;
            break;
        }

        vector<Rect> faces;
        Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);
        facedetect.detectMultiScale(gray, faces, 1.3, 5);

        cout << faces.size() << " faces detected" << endl;

        for (size_t i = 0; i < faces.size(); i++) {
            rectangle(img, faces[i], Scalar(50, 50, 255), 3);
        }

        rectangle(img, Point(0, 0), Point(250, 70), Scalar(50, 50, 255), FILLED);
        putText(img, to_string(faces.size()) + (faces.size() == 1 ? " Face Found" : " Faces Found"),
            Point(10, 40), FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);

        imshow("Frame", img);
        if (waitKey(1) == 27) { // ESC key
            break;
        }
    }

    video.release();
    destroyAllWindows();
    return 0;
}
