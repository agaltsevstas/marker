#include <road_scene_marker.hpp>

#include <boost/filesystem.hpp>

#include "forward_player.hpp"
#include "road_scene_storage.hpp"
#include "util.hpp"

namespace internal
{
cv::Mat convertToLabels(const cv::Size &frameSize, const std::vector<marker::RoadScene> &scene)
{
    CV_Assert(!frameSize.empty());
    CV_Assert(!scene.empty());
    cv::Mat labels(frameSize, CV_8U, cv::Scalar::all(255));


    for(int row = 0; row < labels.rows; ++row)
    {
        for(int col = 0; col < labels.cols; ++col)
        {
            const cv::Point pt{col, row};

            for(auto object = scene.cbegin(); object != scene.cend(); ++object)
            {
                const double isIncide {cv::pointPolygonTest(object->points, pt, false)};

                if(isIncide == 1.)
                {
                    if(labels.at<uchar>(row, col) != 255)
                        std::cout << "warning: overlap of hulls detected" << std::endl;
                    labels.at<uchar>(row, col) = object->type;
                }

            }
        }
    }

    return labels;
}

}

void marker::exportScene(const std::string &video, const std::string &group)
{
    marker::ForwardPlayer cap{video};
    marker::RoadSceneStorage storage{internal::getSceneXMLPath(video)};

    boost::filesystem::path resultSavePath {video};
    resultSavePath.replace_extension("");
    const std::string prefix {resultSavePath.string()};

    try
    {
        boost::filesystem::create_directory(resultSavePath);
        boost::filesystem::create_directory(resultSavePath / "images");
        boost::filesystem::create_directory(resultSavePath / "labels");
    }
    catch(const boost::filesystem::filesystem_error &e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    cv::Ptr<cv::VideoWriter> imageWriter, labelsWriter;

    cv::Mat frame;
    for(;;)
    {
        frame = cap.next();

        if(frame.empty())
            break;

        const std::vector<marker::RoadScene> &roadScene{storage.read(cap.get(cv::CAP_PROP_POS_FRAMES))};

        if(roadScene.empty())
            continue;

        const cv::Mat labels {internal::convertToLabels(frame.size(), roadScene)};

        if(imageWriter.empty())
        {
            imageWriter = cv::makePtr<cv::VideoWriter>(prefix + "/images/%04d.png", CV_FOURCC('H','Z', 'H', 'Z'), 1, frame.size()); //FIXME found fourcc for this
            labelsWriter = cv::makePtr<cv::VideoWriter>(prefix + "/labels/%04d.png", CV_FOURCC('H','Z', 'H', 'Z'), 1, frame.size());
        }


        imageWriter->write(frame);
        labelsWriter->write(labels);

    }
}

