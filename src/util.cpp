#include <boost/filesystem.hpp>

#include "util.hpp"
#include "sign_marker.hpp"

long internal::jump(std::string frameCount)
{
    cv::Mat frame(300, 640, CV_8UC3, cv::Scalar::all(0));

    const std::string winName {"jump"};
    cv::namedWindow(winName);

    std::string str{};

    for(;;)
    {
        cv::Mat frameCopy;
        frame.copyTo(frameCopy);

        cv::Scalar color_message{cv::Scalar(0, 0, 255)};

        bool condition {};

        try
        {
            if(std::stoi(str) <= std::stoi(frameCount))
            {
                color_message = cv::Scalar(0, 255, 0);

                condition = true;
            }
        }
        catch(...){}

        cv::putText(frameCopy, "jump to: " + str, cv::Point(100,100), cv::FONT_HERSHEY_PLAIN, 2., color_message , 1);

        cv::imshow(winName, frameCopy);
        int key = cv::waitKey(0) & 0xff;

        if(key == 8) //delete symbol
        {
            if(str.length() > 0)
            {
                str.erase(str.length() - 1);
            }
        }
        else if(key == 32 && condition) //space
        {
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

    return std::stoi(str);
}

inline std::string addNewSuffix(const std::string &originalFilePath, const std::string &newSuffix)
{
    boost::filesystem::path originalPath {originalFilePath};
    return originalPath.replace_extension("").string() + newSuffix;
}
std::string internal::getFileName(const std::string &video)
{
    boost::filesystem::path originalPath {video};
    return originalPath.filename().string();
}

std::string internal::getPath(const std::string &video)
{
    return video;
}

std::string internal::getSceneXMLPath(const std::string &video)
{
    return addNewSuffix(video, "_scene.xml");
}

std::string internal::getSignXMLPath(const std::string &video, const std::string &group)
{
    return addNewSuffix(video, ("_" + group + ".xml"));
}
std::string internal::getLayoutXMLPath(const std::string &video)
{
    return addNewSuffix(video, "_layout.xml");
}
