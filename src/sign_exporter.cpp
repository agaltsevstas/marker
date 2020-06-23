#include <fstream>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

#include "sign_marker.hpp"
#include "sign_storage.hpp"
#include "util.hpp"


namespace internal
{
    bool checkBB(const cv::Size &frameSize, const cv::Rect &bb)
    {
        const cv::Rect frameBB(cv::Point(0,0), frameSize);

        return (frameBB & bb).size().area() == bb.size().area();
    }
}

void marker::exportSigns(const std::string &video, const std::string &group)
{

    boost::filesystem::path resultSavePath {video};
    resultSavePath.replace_extension("");
    const std::string prefix {resultSavePath.filename().string()};

    try
    {
        boost::filesystem::create_directory(resultSavePath);
    }
    catch(const boost::filesystem::filesystem_error &e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    cv::VideoCapture cap{video};
    CV_Assert(cap.isOpened());
    marker::SignStorage storage{video, group};

    std::map<std::string, int> index;

    for(;;)
    {
        cv::Mat frame;

        cap >> frame;

        if(frame.empty())
            break;

        const int frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);

        std::vector<marker::Sign> signs {storage.read(frameIndex)};

        for(std::vector<marker::Sign>::iterator sign = signs.begin(); sign != signs.end(); ++sign)
        {
            const std::string folderName = sign->signName;
            const boost::filesystem::path savePath {resultSavePath /folderName};

            if(!boost::filesystem::is_directory(savePath))
            {
                try
                {
                    boost::filesystem::create_directory(savePath);
                }
                catch(const boost::filesystem::filesystem_error &e)
                {
                    std::cerr << e.what() << std::endl;
                    return;
                }
            }

            if(internal::checkBB(frame.size(), sign->boundRect))
                cv::imwrite(savePath.string() + "/" + prefix + "_" + std::to_string(index[folderName]++) + ".jpg", frame(sign->boundRect));
            else
                std::cerr << "bad sign" << std::endl;

        }

    }

}



void marker::exportSignsWithTXT(const std::string &video, const std::string &group)
{
    boost::filesystem::path resultSavePath {video};
    resultSavePath.replace_extension("");
    const std::string prefix {resultSavePath.filename().string()};
    resultSavePath /= "objects";

    try
    {
        boost::filesystem::create_directory(resultSavePath);
    }
    catch(const boost::filesystem::filesystem_error &e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    cv::VideoCapture cap{video};
    CV_Assert(cap.isOpened());
    marker::SignStorage storage{video, group};

    for(;;)
    {
        cv::Mat frame;

        cap >> frame;

        if(frame.empty())
            break;

        const int frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);

        std::vector<marker::Sign> signs {storage.read(frameIndex)};

        std::ostringstream oss{};

        for(std::vector<marker::Sign>::iterator sign = signs.begin(); sign != signs.end(); ++sign)
        {
            if(internal::checkBB(frame.size(), sign->boundRect))
                oss << *sign;
            else
                std::cerr << "bad sign out of roi" << std::endl;
        }

        if(!oss.str().empty())
        {
            const std::string savePath{resultSavePath.string() + "/" + prefix + "_" + std::to_string(frameIndex)};
            cv::imwrite(savePath + ".jpg", frame);
            std::ofstream file{savePath + ".txt"};
            CV_Assert(file.is_open());
            file << oss.str();
        }
    }
}
