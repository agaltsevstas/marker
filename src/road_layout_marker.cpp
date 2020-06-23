#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

#include "util.hpp"
#include "road_layout_marker.hpp"
#include "forward_player.hpp"
#include "road_layout.hpp"
#include "road_layout_storage.hpp"

namespace internal
{

void roadLayoutCallback(int event, int x, int y, int flags, void* userdata)
{
    std::pair<bool*, std::vector<marker::RoadLayout>*> *data = static_cast<std::pair<bool*, std::vector<marker::RoadLayout>*>*>(userdata);

    std::vector<marker::RoadLayout> &lines{*(data->second)};
    bool &isChanged {*(data->first)};

    const cv::Point pt{x,y};

    static bool onMove{};

    static std::vector<cv::Point>::iterator selectedPoint{};

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        for(size_t index = 0; index < lines.size(); ++index)
        {
            marker::RoadLayout &roadLayout{lines[index]};

            for(std::vector<cv::Point>::iterator linePoint = roadLayout.points.begin(); linePoint != roadLayout.points.end(); ++linePoint)
            {
                if((pt - *linePoint).ddot(pt - *linePoint) < 100)
                {
                    selectedPoint = linePoint;
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

        if(!lines.empty())
        {
            double minDist {std::numeric_limits<double>::max()};
            std::vector<cv::Point>::iterator position{};
            size_t lineIndex{};

            for(size_t index = 0; index < lines.size(); ++index)
            {
                marker::RoadLayout &roadLayout{lines[index]};

                CV_Assert(roadLayout.points.size() >= 2);

                for(std::vector<cv::Point>::iterator point = roadLayout.points.begin(); point != roadLayout.points.end(); ++point) {
                    const double currDist {(pt - *point).ddot(pt - *point)};

                    if(currDist < minDist)
                    {
                        position = point;
                        lineIndex = index;
                        minDist = currDist;
                    }
                }
            }

            if(event == cv::EVENT_RBUTTONDOWN)
            {
                if(minDist < 300)
                {
                    lines[lineIndex].points.erase(position);

                    if(lines[lineIndex].points.size() < 2)
                        lines.erase(lines.begin() + lineIndex);

                }
            }
            else
                lines[lineIndex].points.insert(position, pt);
        }

    }
    else if(flags & cv::EVENT_FLAG_LBUTTON)
    {
        if(onMove)
            *selectedPoint = pt;
    }

}

marker::RoadLayout addLine()
{
    const std::string winName{"add line"};
    cv::namedWindow(winName);

    cv::Mat black(300, 640, CV_8UC3, cv::Scalar::all(0));

    cv::Point pt(250, 10);

    for(marker::RoadLayoutTypes::const_iterator it = marker::roadLayoutTypes.cbegin(); it != marker::roadLayoutTypes.cend(); ++it)
    {
        std::string line_description {std::to_string(it->first) + " -> " + it->second};

        cv::putText(black, line_description, pt, cv::FONT_HERSHEY_PLAIN, 1., cv::Scalar(0, 169, 255));

        pt.y += 20;
    }

    std::string str{};

    int lineType{};

    for(;;)
    {
        cv::Mat frame;
        black.copyTo(frame);

        cv::putText(frame, "line type: " + str, cv::Point(10,10), cv::FONT_HERSHEY_PLAIN, 1., cv::Scalar(0, 0, 255), 1);

        std::string message{"Unknown line type: " + str};
        cv::Scalar color{cv::Scalar(0, 0, 255)};

        try
        {
            const int value {std::stoi(str)};

            if(marker::roadLayoutTypes.find(value) != marker::roadLayoutTypes.end())
            {
                color = cv::Scalar(0, 255, 0);
                message = "new line is " + marker::roadLayoutTypes.at(value);
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
            lineType = std::stoi(str);
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

    return lineType;
}
}


void marker::markRoadLayout(const std::string &video)
{
    marker::ForwardPlayer cap{video};
    marker::RoadLayoutStorage storage{internal::getLayoutXMLPath(video)};

    const std::string frameCount {std::to_string(int(cap.get(cv::CAP_PROP_FRAME_COUNT)))};
    const std::string helpMessage {"press space to enable \'edit\' mode"};
    const std::vector<std::string> editHelpMessage
    {
        "\'j\' - jump to particular frame",
        "\'n\' - next frame",
        "\'p\' - previous frame",
        "\'a\' - add road layout"
    };

    const std::string winName {"road layout scene marker"};
    cv::namedWindow(winName);


    bool signChanged {};
    std::vector<marker::RoadLayout> lines;
    auto flagAndLines = std::make_pair(&signChanged, &lines);

    cv::setMouseCallback(winName, internal::roadLayoutCallback, &flagAndLines);

    bool onPause{true};
    cv::Mat frame = cap.next();
    long frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
    lines = storage.read(frameIndex);

    for(;;)
    {
        if(!onPause)
        {
            frame = cap.next();
            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            lines = storage.read(frameIndex);
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

        marker::drawRoadLayout(frameCopy, lines);

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
            lines = storage.read(frameIndex);
        }
        else if(key == 'n' && onPause)
        {
            frame = cap.next();

            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            lines = storage.read(frameIndex);
        }
        else if(key == 'j' && onPause)
        {
//            frame = cap.jump(internal::jump(frameCount));
            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            lines = storage.read(frameIndex);
        }
        else if(key == 'a' && onPause)
        {
            try
            {
                lines.push_back(marker::RoadLayout(internal::addLine()));
                signChanged = true;
            }
            catch(const internal::Cancel &){}
        }
        else if(key == 'A' && onPause)
        {
            frame = cap.next();

            if(!frame.empty())
                storage.write(frameIndex+1, lines); //extend to next frame

            frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
            lines = storage.read(frameIndex);
        }

        if(signChanged)
            storage.write(frameIndex, lines);

    }

    cv::destroyWindow(winName);
}
