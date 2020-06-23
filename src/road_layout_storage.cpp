#include "road_layout_storage.hpp"

marker::RoadLayoutStorage::RoadLayoutStorage(const std::string &_pathToXML):
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

        std::vector<marker::RoadLayout> roadLayouts;

        for(cv::FileNodeIterator it2 = (*it).begin(); it2 != (*it).end();)
        {
            cv::Mat serializedLine;
            it2 >> serializedLine;

            CV_Assert(!serializedLine.empty());

            roadLayouts.push_back(marker::RoadLayout(serializedLine));

        }

        storage[frameIndex] = roadLayouts;
    }
}

void marker::RoadLayoutStorage::write(long frameIndex, const std::vector<marker::RoadLayout> &layout)
{
    storage[frameIndex] = layout;
}

std::vector<marker::RoadLayout> marker::RoadLayoutStorage::read(long frameIndex)
{
    return storage[frameIndex];
}

marker::RoadLayoutStorage::~RoadLayoutStorage()
{

    cv::FileStorage fs{pathToXML, cv::FileStorage::WRITE};

    for(std::map<long, std::vector<marker::RoadLayout>>::const_iterator it = storage.begin(); it != storage.end(); ++it)
    {
        const std::vector<marker::RoadLayout> &roadLayout {it->second};

        if(roadLayout.empty())
            continue;
        else
        {
            fs << "signs_" + std::to_string(it->first) << "[";

            for(auto line = roadLayout.begin(); line != roadLayout.end(); ++line)
                fs << line->toMat();

            fs << "]";
        }
    }
}
