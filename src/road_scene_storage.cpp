#include "road_scene_storage.hpp"

marker::RoadSceneStorage::RoadSceneStorage(const std::string &_pathToXML):
    pathToXML{_pathToXML}
{
    CV_Assert(!pathToXML.empty());

    cv::FileStorage fs{pathToXML, cv::FileStorage::READ};

    for(cv::FileNodeIterator it = fs.root().begin(); it != fs.root().end(); ++it)
    {
        CV_Assert((*it).isSeq());

        std::string nodeName {(*it).name()};
        CV_Assert(!nodeName.empty());

        const int frameIndex {std::stoi(nodeName.erase(0,6))};

        std::vector<marker::RoadScene> roadScene;

        for(cv::FileNodeIterator it2 = (*it).begin(); it2 != (*it).end();)
        {
            cv::Mat serializedLine;
            it2 >> serializedLine;

            CV_Assert(!serializedLine.empty());

            roadScene.push_back(marker::RoadScene(serializedLine));

        }

        storage[frameIndex] = roadScene;
    }
}
void marker::RoadSceneStorage::write(long frameIndex, const std::vector<marker::RoadScene> &scene)
{
    storage[frameIndex] = scene;
}

std::vector<marker::RoadScene> marker::RoadSceneStorage::read(long frameIndex)
{
    return storage[frameIndex];
}

marker::RoadSceneStorage::~RoadSceneStorage()
{
    cv::FileStorage fs{pathToXML, cv::FileStorage::WRITE};

    for(std::map<long, std::vector<marker::RoadScene>>::const_iterator it = storage.begin(); it != storage.end(); ++it)
    {
        const std::vector<marker::RoadScene> &roadLayout {it->second};

        if(roadLayout.empty())
            continue;
        else
        {
            fs << "scene_" + std::to_string(it->first) << "[";

            for(auto line = roadLayout.begin(); line != roadLayout.end(); ++line)
                fs << line->toMat();

            fs << "]";
        }
    }
}
