#include <opencv2/opencv.hpp>
#include <road_scene_marker.hpp>

#ifdef DEEPLABV2_FOUND

#include <boost/filesystem.hpp>

#include "forward_player.hpp"
#include "util.hpp"
#include "segmentation.hpp"
#include "road_scene.hpp"
#include "road_scene_storage.hpp"

namespace internal
{

/*! @brief Апроксимация сегментации с помощью оптического потока
 * @param prevFrame - предыдущий кадр
 * @param frame - кадр, на который производиться аппроксимация
 * @param scene - вектор объектов дорожной сцены
 *
*/
void updateWithOpticlFlow(const cv::Mat &prevFrame, const cv::Mat frame, std::vector<marker::RoadScene> &scene)
{

//#define DEBUG
#ifdef DEBUG
    cv::Mat copy;
    frame.copyTo(copy);
#endif

    std::vector<cv::Point2f> prevPoints;

    for(auto sceneObject = scene.cbegin(); sceneObject != scene.cend(); ++sceneObject)
        for(auto point = sceneObject->points.cbegin(); point != sceneObject->points.cend(); ++point)
            prevPoints.push_back(*point);

    std::vector<cv::Point2f> points;
    std::vector<uchar> forwardStatus;
    cv::calcOpticalFlowPyrLK(prevFrame, frame, prevPoints, points, forwardStatus, cv::noArray());

    CV_Assert(points.size() == prevPoints.size());
    CV_Assert(forwardStatus.size() == prevPoints.size());

    std::vector<uchar> backwardStatus;
    std::vector<cv::Point2f> backwardPoints;
    cv::calcOpticalFlowPyrLK(frame, prevFrame, points, backwardPoints, backwardStatus, cv::noArray());

    CV_Assert(points.size() == backwardPoints.size());
    CV_Assert(backwardStatus.size() == backwardPoints.size());

    size_t index{};
    for(auto sceneObject = scene.begin(); sceneObject != scene.end(); ++sceneObject)
    {
        for(auto point = sceneObject->points.begin(); point != sceneObject->points.end(); ++point)
        {
#ifdef DEBUG
            cv::Scalar color(0,0,255);
#endif
            if(forwardStatus[index] && backwardStatus[index])
            {
                const cv::Point reProjectionError{prevPoints[index] - backwardPoints[index]};
                if(sqrt(reProjectionError.ddot(reProjectionError)) < 3.)
                {
                    *point = points[index];
#ifdef DEBUG
                    color = cv::Scalar(255,0,0);
                }
                else
                    color = cv::Scalar(169, 0, 255);
#else
                }
#endif

            }
#ifdef DEBUG
            cv::circle(copy, prevPoints[index], 2, color, 2);
            cv::circle(copy, points[index], 2, color, 2);
            cv::line(copy, prevPoints[index], points[index], cv::Scalar(0,255,0));
#endif
            index++;
        }
    }

    CV_Assert(index == prevPoints.size());

#ifdef DEBUG
    cv::imshow("optical flow", copy);
    cv::waitKey();
#endif
}

std::vector<marker::RoadScene> extractObjects(const cv::Mat &segmented, int objectType)
{
    cv::Mat objects{segmented == cv::Mat(segmented.size(), segmented.type(), cv::Scalar::all(objectType))};

    cv::morphologyEx(objects, objects, cv::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(15, 15)));
    cv::morphologyEx(objects, objects, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(15, 15)));

    cv::Mat edgesObject;
    cv::Canny(objects, edgesObject, 1, 2);
    std::vector<std::vector<cv::Point> > contoursObject;
    cv::findContours(edgesObject, contoursObject, cv::RETR_EXTERNAL , cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    std::vector<marker::RoadScene> result;
    for(size_t index = 0; index < contoursObject.size(); ++index)
    {
        std::vector<cv::Point> points{};
        cv::convexHull(contoursObject[index], points);
        result.push_back(marker::RoadScene(objectType, points));
    }

    return result;
}

void roadSceneCallback(int event, int x, int y, int flags, void* userdata)
{

    std::pair<bool*, std::vector<marker::RoadScene>*> *data = static_cast<std::pair<bool*, std::vector<marker::RoadScene>*>*>(userdata);

    std::vector<marker::RoadScene> &objects{*(data->second)};
    bool &isChanged {*(data->first)};

    const cv::Point pt{x,y};

    static bool onMove{};

    static std::vector<cv::Point>::iterator selectedPoint{};

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        for(size_t index = 0; index < objects.size(); ++index)
        {
            marker::RoadScene &sceneObject{objects[index]};

            for(std::vector<cv::Point>::iterator contourPoint = sceneObject.points.begin(); contourPoint != sceneObject.points.end(); ++contourPoint)
            {
                if((pt - *contourPoint).ddot(pt - *contourPoint) < 200)
                {
                    selectedPoint = contourPoint;
                    onMove = true;
                    break;
                }
            }
        }
    }
    else if(event == cv::EVENT_LBUTTONUP)
    {
        onMove = false;
        isChanged = true;
    }
    else if(event == cv::EVENT_RBUTTONDOWN || event == cv::EVENT_MBUTTONUP)
    {
        isChanged = true;

        if(!objects.empty())
        {
            double minDist {std::numeric_limits<double>::max()};
            std::vector<cv::Point>::iterator position{};
            size_t objectIndex{};

            for(size_t index = 0; index < objects.size(); ++index)
            {
                marker::RoadScene &roadScene{objects[index]};

                CV_Assert(roadScene.points.size() >= 2);

                for(std::vector<cv::Point>::iterator point = roadScene.points.begin(); point != roadScene.points.end(); ++point) {
                    const double currDist {(pt - *point).ddot(pt - *point)};

                    if(currDist < minDist)
                    {
                        position = point;
                        objectIndex = index;
                        minDist = currDist;
                    }
                }
            }

            if(event == cv::EVENT_RBUTTONDOWN)
            {
                if(minDist < 300)
                {
                    objects[objectIndex].points.erase(position);

                    if(objects[objectIndex].points.size() < 2)
                        objects.erase(objects.begin() + objectIndex);
                    else if(flags & cv::EVENT_FLAG_CTRLKEY)
                        objects.erase(objects.begin() + objectIndex);

                }
            }
            else
                objects[objectIndex].points.insert(position, pt);
        }

    }
    else if(flags & cv::EVENT_FLAG_LBUTTON)
    {
        if(onMove)
            *selectedPoint = pt;
    }

}

marker::RoadScene addObject()
{
    const std::string winName{"add object"};
    cv::namedWindow(winName);

    cv::Mat black(300, 640, CV_8UC3, cv::Scalar::all(0));

    cv::Point pt(250, 10);

    for(std::map<int, std::string>::const_iterator it = marker::objectTypes.cbegin(); it != marker::objectTypes.cend(); ++it)
    {
        std::string objectDescription {std::to_string(it->first) + " -> " + it->second};

        cv::putText(black, objectDescription, pt, cv::FONT_HERSHEY_PLAIN, 1., cv::Scalar(0, 169, 255));

        pt.y += 20;
    }

    std::string str{};

    int objectType{};

    for(;;)
    {
        cv::Mat frame;
        black.copyTo(frame);

        cv::putText(frame, "object type: " + str, cv::Point(10,10), cv::FONT_HERSHEY_PLAIN, 1., cv::Scalar(0, 0, 255), 1);

        std::string message{"Unknown object type: " + str};
        cv::Scalar color{cv::Scalar(0, 0, 255)};

        try
        {
            const int value {std::stoi(str)};

            if(marker::objectTypes.find(value) != marker::objectTypes.end())
            {
                color = cv::Scalar(0, 255, 0);
                message = "new object is " + marker::objectTypes.at(value);
            }

        }
        catch(...){}

        cv::putText(frame, message, cv::Point(10,30), cv::FONT_HERSHEY_PLAIN, 1., color, 1);

        cv::imshow(winName, frame);
        int key = cv::waitKey(1) & 0xff;

        if(key == 8)
        {
            if(str.length() > 0) //delete symbol
            {
                str.erase(str.length() - 1);
            }
        }
        else if(key == 13) //enter
        {
            objectType = std::stoi(str);
            break;
        }
        else if(key >= 48 && key <= 57)
            str += key;
        else if(key == 27) //ECS
        {
            cv::destroyWindow(winName);
            throw internal::Cancel{};
        }

    }

    cv::destroyWindow(winName);

    return objectType;
}

}


void marker::markScene(const std::string &video,
                     const std::string &prototxt,
                     const std::string &weights)
{
    marker::ForwardPlayer cap{video};

    marker::RoadSceneStorage storage{internal::getSceneXMLPath(video)};

    const std::string frameCount {std::to_string(int(cap.get(cv::CAP_PROP_FRAME_COUNT)))};
    const std::string helpMessage {"press space to enable \'edit\' mode"};
    const std::vector<std::string> editHelpMessage
    {
        "\'j\' - jump to particular frame",
        "\'n\' - next frame",
        "\'p\' - previous frame",
        "\'a\' - add object",
        "shift + \'a\' - perform auto segmentation"
    };

    const std::string winName {"road scene marker"};
    cv::namedWindow(winName);


    std::vector<marker::RoadScene> roadScene{};
    bool sceneChanged {};

    auto flagAndScene = std::make_pair(&sceneChanged, &roadScene);

    cv::setMouseCallback(winName, internal::roadSceneCallback, &flagAndScene);

    bool onPause{true};
    cv::Mat frame = cap.next();
    long frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
    roadScene = storage.read(frameIndex);

    cv::Ptr<marker::Segmentation> segmentation = marker::Segmentation::create(prototxt, weights, frame.size());

    for(;;)
    {
        if(!onPause)
        {
            frame = cap.next();
            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            roadScene = storage.read(frameIndex);

        }

        if(frame.empty())
            break;

        cv::Mat frameCopy;
        frame.copyTo(frameCopy);

        cv::putText(frameCopy, '#' + std::to_string(frameIndex) + '/' + frameCount, cv::Point(5,20), cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0,255,0), 2);

        if(!onPause)
            cv::putText(frameCopy, helpMessage, cv::Point(frame.cols - 500, frame.rows - 100), cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(0,0,255));
        else
        {
            cv::Point drawPoint(frame.cols - 550, frame.rows - 20);

            for(std::vector<std::string>::const_iterator it = editHelpMessage.cbegin(); it != editHelpMessage.end(); ++it)
            {
                cv::putText(frameCopy, *it, drawPoint, cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(0,0,255));
                drawPoint.y -= 30;
            }
        }

        marker::drawRoadScene(roadScene, frameCopy);

        cv::imshow(winName, frameCopy);
        int key = cv::waitKey(25) & 0xff;

        if(key == 27)
            break;
        else if(key == 32)
            onPause = !onPause;
        else if(key == 'p' && onPause)
        {
            frame = cap.previous();
            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            roadScene = storage.read(frameIndex);
        }
        else if(key == 'n' && onPause)
        {
            frame = cap.next();
            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            roadScene = storage.read(frameIndex);
        }
        else if(key == 'j' && onPause)
        {
            frame = cap.jump(internal::jump());
            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            roadScene = storage.read(frameIndex);
        }
        else if(key == 'a' && onPause)
        {
            roadScene.push_back(internal::addObject());
            sceneChanged = true;
        }
        else if(key == 'A' && onPause)
        {
            roadScene.clear();

            frame = cap.next();

            if(frame.empty())
                break;

            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);

            const cv::Mat segmented {segmentation->segment(frame)};
            //cv::imshow("segmented", marker::colorize(segmented));

            const std::vector<marker::RoadScene> humanCountours{internal::extractObjects(segmented, 0)}; //human
            roadScene.insert(roadScene.end(), humanCountours.begin(), humanCountours.end());

            const std::vector<marker::RoadScene> carsCountours{internal::extractObjects(segmented, 1)}; //cars
            roadScene.insert(roadScene.end(), carsCountours.begin(), carsCountours.end());

            const std::vector<marker::RoadScene> roadCountours{internal::extractObjects(segmented, 2)}; //road
            roadScene.insert(roadScene.end(), roadCountours.begin(), roadCountours.end());

            sceneChanged = true;
        }else if(key == 'O' && onPause)
        {
            cv::Mat prevFrame{};
            frame.copyTo(prevFrame);

            frame = cap.next();

            if(frame.empty())
                break;

            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);

            internal::updateWithOpticlFlow(prevFrame, frame, roadScene);

            sceneChanged = true;

        }
        if(sceneChanged)
        {
            storage.write(frameIndex, roadScene);
            sceneChanged = false;
        }

    }

    cv::destroyWindow(winName);
}
#else
void marker::markScene(const std::string &, const std::string &, const std::string &)
{
    CV_Error(cv::Error::StsBadFunc, "Build without deeplabv2");
}
#endif
