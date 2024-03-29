//Includes all the headers necessary to use the most common public pieces of the ROS system.
#include <ros/ros.h>
//Use image_transport for publishing and subscribing to images in ROS
#include <image_transport/image_transport.h>
//Use cv_bridge to convert between ROS and OpenCV Image formats
#include <cv_bridge/cv_bridge.h>
//Include some useful constants for image encoding. Refer to: http://www.ros.org/doc/api/sensor_msgs/html/namespacesensor__msgs_1_1image__encodings.html for more info.
#include <sensor_msgs/image_encodings.h>
//Include headers for OpenCV Image processing
#include <opencv2/imgproc/imgproc.hpp>
//Include headers for OpenCV GUI handling
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;

//Store all constants for image encodings in the enc namespace to be used later.
namespace enc = sensor_msgs::image_encodings;
 
//Declare a string with the name of the window that we will create using OpenCV where processed images will be displayed.
//static const char WINDOW[] = "Image Processed";
 
//Use method of ImageTransport to create image publisher
image_transport::Publisher pub;
 
//This function is called everytime a new image is published
void imageCallback(const sensor_msgs::ImageConstPtr& original_image)
{
    //Convert from the ROS image message to a CvImage suitable for working with OpenCV for processing
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        //Always copy, returning a mutable CvImage
        //OpenCV expects color images to use BGR channel order.
        cv_ptr = cv_bridge::toCvCopy(original_image, sensor_msgs::image_encodings::MONO8);
    }
    catch (cv_bridge::Exception& e)
    {
        //if there is an error during conversion, display it
        ROS_ERROR("stream_filter::bilateral_filter.cpp::cv_bridge exception: %s", e.what());
        return;
    }
    //TODO make a proper dst matrix which doesnt need to read in the original image
    //cv::Mat dst_ptr;
    //dst_ptr =zeros( cv_ptr->image.size(), cv_ptr->image.type() );

    //Gaussian blur
    cv::GaussianBlur( cv_ptr->image, cv_ptr->image, cv::Size(9, 9), 2, 2 );

    //bilateral filter
    //cv_bridge::CvImagePtr dst_ptr;
    //dst_ptr = cv_bridge::toCvCopy(original_image, sensor_msgs::image_encodings::BGR8);;
    //use bilateral filter on camera stream
    //cv::bilateralFilter ( cv_ptr->image, dst_ptr->image, 10, 20, 20 );

    std::vector<cv::Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    cv::HoughCircles( cv_ptr->image, circles, cv::Canny::CV_HOUGH_GRADIENT, 1, cv_ptr->image.rows/8, 200, 100, 0, 0 );
  
    /// Draw the circles detected
    //for( size_t i = 0; i < circles.size(); i++ )
    //{
    //    Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
    //    int radius = cvRound(circles[i][2]);
    //    // circle center
    //    circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
    //    // circle outline
    //    circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
    //}
 
 
   
    //Convert the CvImage to a ROS image message and publish it on the "camera/image_processed" topic.
    pub.publish(cv_ptr->toImageMsg()); //change cv_ptr to dst_ptr if bilateral filter
    //pub.publish(original_image);
}
 
/**
* This tutorial demonstrates simple image conversion between ROS image message and OpenCV formats and image processing
*/
int main(int argc, char **argv)
{
    /**
    * The ros::init() function needs to see argc and argv so that it can perform
    * any ROS arguments and name remapping that were provided at the command line. For programmatic
    * remappings you can use a different version of init() which takes remappings
    * directly, but for most command-line programs, passing argc and argv is the easiest
    * way to do it.  The third argument to init() is the name of the node. Node names must be unique in a running system.
    * The name used here must be a base name, ie. it cannot have a / in it.
    * You must call one of the versions of ros::init() before using any other
    * part of the ROS system.
    */
        ros::init(argc, argv, "image_processor");
    /**
    * NodeHandle is the main access point to communications with the ROS system.
    * The first NodeHandle constructed will fully initialize this node, and the last
    * NodeHandle destructed will close down the node.
    */
        ros::NodeHandle nh;
    //Create an ImageTransport instance, initializing it with our NodeHandle.
        image_transport::ImageTransport it(nh);
    //OpenCV HighGUI call to create a display window on start-up.
    //cv::namedWindow(WINDOW, cv::WINDOW_AUTOSIZE);
    //cv::namedWindow( "Display window", cv::WINDOW_NORMAL);
    //changed 1 from CV_WINDOW_AUTOSIZE
    /**
    * Subscribe to the "camera/image_raw" base topic. The actual ROS topic subscribed to depends on which transport is used.
    * In the default case, "raw				s								" transport, the topic is in fact "camera/image_raw" with type sensor_msgs/Image. ROS will call
    * the "imageCallback" function whenever a new image arrives. The 2nd argument is the queue size.
    * subscribe() returns an image_transport::Subscriber object, that you must hold on to until you want to unsubscribe.
    * When the Subscriber object is destructed, it will automatically unsubscribe from the "camera/image_raw" base topic.
    */
        image_transport::Subscriber sub = it.subscribe("camera/image_raw", 1, imageCallback);
    //OpenCV HighGUI call to destroy a display window on shut-down.
    //cv::destroyWindow(WINDOW);
    /**
    * The advertise() function is how you tell ROS that you want to
    * publish on a given topic name. This invokes a call to the ROS
    * master node, which keeps a registry of who is publishing and who
    * is subscribing. After this advertise() call is made, the master
    * node will notify anyone who is trying to subscribe to this topic name,
    * and they will in turn negotiate a peer-to-peer connection with this
    * node.  advertise() returns a Publisher object which allows you to
    * publish messages on that topic through a call to publish().  Once
    * all copies of the returned Publisher object are destroyed, the topic
    * will be automatically unadvertised.
    *
    * The second parameter to advertise() is the size of the message queue
    * used for publishing messages.  If messages are published more quickly
    * than we can send them, the number here specifies how many messages to
    * buffer up before throwing some away.
    */
        pub = it.advertise("camera/image_processed", 1);

    /**
    * In this application all user callbacks will be called from within the ros::spin() call.
    * ros::spin() will not return until the node has been shutdown, either through a call
    * to ros::shutdown() or a Ctrl-C.
    */
        ros::spin();

    //ROS_INFO is the replacement for printf/cout.
    ROS_INFO("stream_filter::bilateral_filter.cpp::No error.");

}
