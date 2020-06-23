#include "road_scene.hpp"

void marker::drawRoadScene(const std::vector<marker::RoadScene> &roadScene, cv::Mat &frame)
{
    CV_Assert(!frame.empty());

    for(std::vector<marker::RoadScene>::const_iterator sceneElemnt = roadScene.begin(); sceneElemnt != roadScene.end(); sceneElemnt++)
    {
        const std::vector<cv::Point> points {sceneElemnt->points};
        const int type {sceneElemnt->type};

        CV_Assert(marker::objectTypes.find(type) != marker::objectTypes.end());

        CV_Assert(points.size() >= 2);

        for(size_t index = 0; index < points.size()- 1; ++index)
        {
            cv::line(frame, points[index], points[index+1], marker::object2color.at(type), 2);
            cv::circle(frame, points[index], 2, cv::Scalar(0, 185, 255), 2);
        }

        cv::line(frame, points.front(), points.back(), marker::object2color.at(type), 2);

        cv::circle(frame, points.front(), 3, cv::Scalar(255, 0, 0), 3);
        cv::circle(frame, points.back(), 3, cv::Scalar(0, 0, 255), 3);

    }
}

cv::Mat marker::colorize(const cv::Mat &segmented)
{
    cv::Mat coloredSeg(segmented.size(), CV_8UC3);
    cv::Mat labelColours(1, 256, CV_8UC3, cv::Scalar::all(50)); //FIXME
    labelColours.at<cv::Vec3b>(0,0) = cv::Vec3b(0,0,255);
    labelColours.at<cv::Vec3b>(0,1) = cv::Vec3b(255,0,0);
    labelColours.at<cv::Vec3b>(0,2) = cv::Vec3b(0,255,70);

    cv::Mat segmented_;
    cv::cvtColor(segmented, segmented_, CV_GRAY2BGR);
    cv::LUT(segmented_, labelColours , coloredSeg);

    return coloredSeg;
}

marker::RoadScene::RoadScene(int _objectType):
    type{_objectType}, points{cv::Point(100,200), cv::Point(200,100), cv::Point(200, 200)}
{}

marker::RoadScene::RoadScene(cv::Mat &serialized)
{
    CV_Assert(!serialized.empty());
    CV_Assert(serialized.type() == CV_32S);
    CV_Assert(serialized.cols % 2);
    CV_Assert(serialized.rows == 1);

    int index{};
    type = serialized.at<int>(index++);

    CV_Assert(marker::objectTypes.find(type) != marker::objectTypes.end());


    for(;index < serialized.cols;)
    {
        const int x {serialized.at<int>(index++)};
        const int y {serialized.at<int>(index++)};
        points.push_back(cv::Point(x,y));
    }

    CV_Assert(index == serialized.cols);
}

cv::Mat marker::RoadScene::toMat() const
{
    CV_Assert(marker::objectTypes.find(type) != marker::objectTypes.end());

    const int nCols = 2*points.size() + 1;

    cv::Mat serialized(1, nCols, CV_32S);

    int index{};
    serialized.at<int>(index++) = type;

    for(std::vector<cv::Point>::const_iterator point = points.cbegin(); point != points.cend(); ++point)
    {
        serialized.at<int>(index++) = point->x;
        serialized.at<int>(index++) = point->y;
    }

    CV_Assert(index == nCols);

    return serialized;
}
