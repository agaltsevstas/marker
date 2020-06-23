#include "road_layout.hpp"

void marker::drawRoadLayout(cv::Mat &frame, const std::vector<marker::RoadLayout> &roadLayouts)
{
    for(size_t index = 0; index < roadLayouts.size(); ++index)
    {
        const marker::RoadLayout &roadLayout{roadLayouts[index]};

        CV_Assert(roadLayout.points.size() >= 2);

        for(size_t index = 0; index < roadLayout.points.size() - 1; ++index)
        {
            cv::line(frame, roadLayout.points[index], roadLayout.points[index+1], cv::Scalar(0, 255, 0), 2);
            cv::circle(frame, roadLayout.points[index], 2, cv::Scalar(0, 185, 255), 2);
            cv::putText(frame, marker::roadLayoutTypes.at(roadLayout.type), roadLayout.points[index], cv::FONT_HERSHEY_PLAIN, 1., cv::Scalar(0, 0, 255));
        }

        cv::circle(frame, roadLayout.points.back(), 2, cv::Scalar(0, 185, 255), 2);
        cv::putText(frame, marker::roadLayoutTypes.at(roadLayout.type), roadLayout.points.back(), cv::FONT_HERSHEY_PLAIN, 1., cv::Scalar(0, 0, 255));
    }
}

marker::RoadLayout::RoadLayout(int _type):
    type{_type}, points{cv::Point(100,100), cv::Point(200,200)}
{}

marker::RoadLayout::RoadLayout(const cv::Mat &serialized)
{
    CV_Assert(!serialized.empty());
    CV_Assert(serialized.type() == CV_32S);
    CV_Assert(serialized.cols % 2);
    CV_Assert(serialized.rows == 1);

    int index{};
    type = serialized.at<int>(index++);

    CV_Assert(marker::roadLayoutTypes.find(type) != marker::roadLayoutTypes.end());


    for(;index < serialized.cols;)
    {
        const int x {serialized.at<int>(index++)};
        const int y {serialized.at<int>(index++)};
        points.push_back(cv::Point(x,y));
    }

    CV_Assert(index == serialized.cols);
}

cv::Mat marker::RoadLayout::toMat() const
{
    CV_Assert(marker::roadLayoutTypes.find(type) != marker::roadLayoutTypes.end());

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
