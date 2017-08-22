#include <ARToolKitPlus/TrackerSingleMarker.h>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <iostream>

using namespace cv;

void cvMatToRawData(const cv::Mat& img, std::vector<unsigned char>& rawData){
    /* convert the image into a gray level image*/
    cv::Mat gray;
    if( img.channels() > 1)
        cv::cvtColor(img, gray, CV_RGB2GRAY);
    else
        gray = img;
    rawData.resize(gray.rows*gray.cols);
    for( size_t i = 0; i < gray.rows; i++){
        for( size_t j = 0; j < gray.cols; j++)
            rawData[i*gray.cols+j] = gray.at<unsigned char>(i,j);
    }
}

int main(int argc, char** argv){
    using namespace std;
    using ARToolKitPlus::TrackerSingleMarker;
    
    char fileName[256];
    int num_Done=0;
    ARFloat Area_mark,Area_img;
    ARFloat Fitness,Percentage;
    ARFloat Area_best;
    double threshold_F=0.75,threshold_P=0.1;
    
    struct passwd* pw = getpwuid(getuid());    
    const char* home_dir = pw->pw_dir;
    
    int checked_image_num = 0;
    for(int num_picture=1;num_picture<2003;num_picture++)
    {
        sprintf(fileName,"%s/Result/%d.png",home_dir, num_picture);
        cv::Mat img = cv::imread(fileName,cv::IMREAD_GRAYSCALE);
        if(img.data!=0)
        {
            std::cout << "[checking NO. " << checked_image_num+1 << " ]";
            /* convert the opencv matrix into a raw camera buffer*/
            std::vector<unsigned char> cameraBuffer;
            cvMatToRawData(img, cameraBuffer);
            TrackerSingleMarker tracker(img.cols, img.rows, 8, 6, 6, 6, 0);
            Area_img=img.cols*img.rows;
            tracker.setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_LUM);
            
            // load a camera calibration file.
            if (!tracker.initWithoutCameraFile())
            {
                std::cerr << "ERROR: init() failed" << std::endl;
                return -1;
            } 
            
            double pattern_width = 100.0;// /mm
            bool useBCH = true;
            tracker.setPatternWidth(pattern_width);
            tracker.setBorderWidth(useBCH ? 0.125 : 0.25);
            
            int thresholds[12] = {20,40,60,80,100,120,140,160,180,200,220,240};
            
            std::vector<int> markerId_tmp;
            int id_best = -1;
            float conf_best = 0;
            ARFloat T_best[16];
            ARFloat corners_best[4][2];
            for( size_t i = 0; i < 12; i++){
                tracker.setThreshold(thresholds[i]);
                
                // let's use lookup-table undistortion for high-speed
                // note: LUT only works with images up to 1024x1024
                tracker.setUndistortionMode(ARToolKitPlus::UNDIST_LUT);
                
                // switch to simple ID based markers
                // use the tool in tools/IdPatGen to generate markers
                tracker.setMarkerMode(useBCH ? ARToolKitPlus::MARKER_ID_BCH : ARToolKitPlus::MARKER_ID_SIMPLE);
                
                ARToolKitPlus::ARMarkerInfo* marker_info = 0;
                markerId_tmp = tracker.calc(&cameraBuffer[0], &marker_info);
                if( markerId_tmp.size() > 0){
                    id_best = tracker.selectBestMarkerByCf(); /*choose the marker with highest confidence*/
                    float conf = tracker.getConfidence();
                    
                    if( conf > conf_best){
                        conf_best = conf;
                        std::copy(tracker.getModelViewMatrix(),tracker.getModelViewMatrix()+16,T_best);
                        const ARToolKitPlus::ARMarkerInfo* marker_info = tracker.getMarkerInfoById(id_best);
                        for( size_t s = 0; s < 4; s++){
                            for( size_t t = 0; t < 2; t++){
                                corners_best[s][t] = marker_info->vertex[s][t];
                            }
                        }
                    }
                }
            }
            std::cout << "Found marker: " << id_best << ", recoginized result: "<< num_picture;
                        
            cv::Mat todraw;
            cv::cvtColor(img,todraw,CV_GRAY2RGB);
            
            if( id_best >= 0){
                /*visualize those markers*/
                double R[9],t[3];
                const ARFloat* T = T_best;
                
                ARFloat Xmax_corners_best=corners_best[0][0];
                ARFloat Ymax_corners_best=corners_best[0][1];
                ARFloat Xmin_corners_best=corners_best[0][0];
                ARFloat Ymin_corners_best=corners_best[0][1];
                ARFloat X_Differ,Y_Differ;
                ARFloat X_centre_best,Y_centre_best;// centre of the markers
                ARFloat corner_square[1][2];
                
                /**1.draw the detected corners*/
                for( size_t i = 0; i < 4; i++){
                    cv::circle(todraw,cv::Point2f(corners_best[i][0],
                               corners_best[i][1]),3,cv::Scalar(0,255,255),1,CV_AA);
                    
                    if(corners_best[i][0]>Xmax_corners_best)
                        Xmax_corners_best=corners_best[i][0];
                    
                    if(corners_best[i][0]<Xmin_corners_best)
                        Xmin_corners_best=corners_best[i][0];
                    
                    if(corners_best[i][1]>Ymax_corners_best)
                        Ymax_corners_best=corners_best[i][1];
                    
                    if(corners_best[i][1]<Ymin_corners_best)
                        Ymin_corners_best=corners_best[i][1];
                }
                
                /*compute the Aera*/
                X_Differ=(Xmax_corners_best-Xmin_corners_best);
                Y_Differ=(Ymax_corners_best-Ymin_corners_best);
                X_centre_best=0.5*X_Differ+Xmin_corners_best;
                Y_centre_best=0.5*Y_Differ+Ymin_corners_best;
                if(X_Differ>Y_Differ)
                {
                    Area_best=X_Differ*X_Differ;
                    corner_square[0][0]=X_centre_best-0.5*X_Differ;
                    corner_square[0][1]=Y_centre_best-0.5*X_Differ;
                    rectangle(todraw, Point(corner_square[0][0],corner_square[0][1]),
                            Point(corner_square[0][0]+X_Differ, corner_square[0][1] + X_Differ),
                            Scalar(0, 0, 255), 2, 8);
                }
                else
                {
                    Area_best=Y_Differ*Y_Differ;
                    corner_square[0][0]=X_centre_best-0.5*Y_Differ;
                    corner_square[0][1]=Y_centre_best-0.5*Y_Differ;
                    rectangle(todraw, Point(corner_square[0][0],corner_square[0][1]),
                            Point(corner_square[0][0]+Y_Differ, corner_square[0][1] + Y_Differ),
                            Scalar(0, 0, 255), 2, 8);
                }
                
                ARFloat corners_best_temp[4][2];
                
                for (int i=1;i<4;i++)
                {
                    corners_best_temp[0][0]=0;corners_best_temp[0][1]=0;
                    corners_best_temp[i][0]=corners_best[0][0]-corners_best[i][0];
                    corners_best_temp[i][1]=corners_best[0][1]-corners_best[i][1];
                }
                /*compute the Aera by an algebraic formula:Aera=0.5*(|x0y1-x1y0|+|x1y2-x2y1|+|x3y2-x2y3|)*/
                Area_mark=fabs(corners_best_temp[1][0]*corners_best_temp[2][1]
                        -corners_best_temp[2][0]*corners_best_temp[1][1])+
                        fabs(corners_best_temp[2][0]*corners_best_temp[3][1]
                        -corners_best_temp[3][0]*corners_best_temp[2][1]);
                Fitness=(0.5*Area_mark)/(Area_best);
                Percentage= Area_mark/Area_img;
                
                if(id_best == num_picture){
                    std::cout << " Square Fitness is:" << Fitness*100.0f << "%, Area Percentage is:" << Percentage* 100.0f << "%" ;
                    if(Fitness>threshold_F && Percentage >threshold_P){
                        std::cout << " - Result is correct but not good!" << std::endl;
                        num_Done=num_Done+1;
                    }else{
                        std::cout << " - Result is correct and good!" << std::endl;
                    }                
                }else{
                    std::cout << " - Result is wrong!" << std::endl;
                }
            }
            cv::imshow("image", todraw);
            cv::waitKey(200);
            char resultName[256];
            sprintf(resultName,"%s/Result/%d_judge.png", home_dir, num_picture);
            imwrite(resultName,todraw);
            checked_image_num++;
            if( checked_image_num > 20)
                break;
        }
    }
    std::cout << "The number of successful identification is:" << num_Done << " in " <<  checked_image_num << std::endl;
    return 0;
}
