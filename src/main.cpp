#include <iostream>

#include <opencv2/opencv.hpp>

#include "sign_marker.hpp"
#include "sign.hpp"
#include "road_layout_marker.hpp"
#include "road_scene_marker.hpp"

int main(int argc, char* argv[])
{
    const std::string keys
    {
        "{help h usage ? |      | print this message               }"
        "{video          |      | path to video file               }"
        "{mode           |      | object, road or scene            }"
        "{types          |      | path to txt with types           }"
        "{group          |      | signs/objects/passengers         }"
        "{export         |      | export objects/scene             }"
        "{export_txt     |      | export objects with txt files    }"
        "{prototxt       |      | path to deeplab2 prototxt file   }"
        "{weights        |      | path to deeplab2 weights file    }"        
    };

    cv::CommandLineParser parser(argc, argv, keys);

    parser.about("Marker app");

    if (parser.has("help"))
    {
        parser.printMessage();
        return EXIT_SUCCESS;
    }

    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }

    std::string pathToVideo {parser.get<std::string>("video")};
    std::replace(pathToVideo.begin(), pathToVideo.end(), '\\', '/');
    std::cout << "pathToVideo: " << pathToVideo << std::endl;

    if(pathToVideo.empty())
    {
        parser.printMessage();
        CV_Error(cv::Error::StsBadArg, "-> Path to input video file cannot be empty <-");
    }

    const std::string mode {parser.get<std::string>("mode")};
    const bool exportData{parser.has("export")};
    const bool exportDataTXT{parser.has("export_txt")};
    std::string group {parser.get<std::string>("group")};

    if(group.empty())
    {
        parser.printMessage();
        CV_Error(cv::Error::StsBadArg, "Unknown group");
    }

    if(mode == "object")
    {
        std::string pathToTxt {parser.get<std::string>("types")};
        std::replace(pathToTxt.begin(), pathToTxt.end(), '\\', '/');

        const marker::SignTypes signTypes{marker::loadSignTypes(pathToTxt)};

        if(exportData)
            marker::exportSigns(pathToVideo, group);
        else if(exportDataTXT)
            marker::exportSignsWithTXT(pathToVideo, group);
        else
            marker::markSigns(pathToVideo, signTypes, group);
    }
    else if(mode == "road")
    {
        marker::markRoadLayout(pathToVideo);
    }
    else if(mode == "scene")
    {
        if(exportData)
            marker::exportScene(pathToVideo, group);
        else
            marker::markScene(pathToVideo, parser.get<std::string>("prototxt"), parser.get<std::string>("weights"));
    }
    else
    {
        parser.printMessage();
        CV_Error(cv::Error::StsBadArg, "-> Unknown mode <-");
    }

    return EXIT_SUCCESS;
}
