#include "sign_storage.hpp"

marker::SignStorage::SignStorage(const std::string &_path, const std::string &group):
    path{internal::getPath(_path)}
{
    std::string pathToXML {internal::getSignXMLPath(_path, group)};
    CV_Assert(!pathToXML.empty());

    tinyxml2::XMLDocument xmlDoc {};
    tinyxml2::XMLError Loaded {xmlDoc.LoadFile(pathToXML.c_str())};

    if(Loaded == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLNode * rootElement {xmlDoc.FirstChildElement(rootTag)}; // корневой элемент xml документа
        xmlDoc.InsertFirstChild(rootElement);
        CV_Assert(rootElement != nullptr);

        tinyxml2::XMLElement * inputElement {rootElement->FirstChildElement(inputTag)}; // элемент входного файла
        CV_Assert(inputElement != nullptr);

        auto del = internal::getFileName(path);
        const char* fileName_ {inputElement->Attribute("name")};
        CV_Assert(fileName_ == internal::getFileName(path));

        for (tinyxml2::XMLElement * frameElement = rootElement->FirstChildElement(frameTag); frameElement != NULL; frameElement = frameElement->NextSiblingElement()) //элемент кадра видео
        {
            std::string nodeName {frameElement->Attribute("number")};
            CV_Assert(!nodeName.empty());

            for (tinyxml2::XMLElement * objectElement = frameElement->FirstChildElement(objectTag); objectElement != NULL; objectElement = objectElement->NextSiblingElement()) // элемент найденного объекта
            {

                cv::Rect boundRect {};

                std::string signName {objectElement->Attribute("className")};
                CV_Assert(signName != "");

                objectElement->QueryIntAttribute("x", &boundRect.x);
                CV_Assert(boundRect.x >= 0);

                objectElement->QueryIntAttribute("y", &boundRect.y);
                CV_Assert(boundRect.y >= 0);

                objectElement->QueryIntAttribute("width", &boundRect.width);
                CV_Assert(boundRect.width > 0);

                objectElement->QueryIntAttribute("height", &boundRect.height);
                CV_Assert(!boundRect.empty());

                objects_in_frame.push_back(marker::convert(signName, boundRect));
            }

            if(!objects_in_frame.empty())
            {
                storage[std::stoi(nodeName)] = objects_in_frame;
                objects_in_frame.clear();
            }
        }
    }
}

void marker::SignStorage::write(long frameIndex, const std::vector<marker::Sign> &signs, const std::string group)
{
    storage[frameIndex] = signs;
    std::string pathToXML {internal::getSignXMLPath(path, group)};

    tinyxml2::XMLDocument xmlDoc {}; //документ xml

    tinyxml2::XMLNode * rootElement {xmlDoc.NewElement(rootTag)}; // корневой элемент xml документа
    xmlDoc.InsertFirstChild(rootElement);

    tinyxml2::XMLElement * inputElement {xmlDoc.NewElement(inputTag)}; // элемент входного файла
    std::string pathFileInUpFolder {internal::getFileName(path)};
    const char* fileName_ {pathFileInUpFolder.c_str()};
    inputElement->SetAttribute("name", fileName_);
    rootElement->InsertEndChild(inputElement);

    for(std::map<long, std::vector<marker::Sign>>::const_iterator it = storage.begin(); it != storage.end(); ++it)
    {
        const std::vector<marker::Sign> &signs {it->second};

        if(signs.empty())
            continue;
        else
        {
            tinyxml2::XMLElement * frameElement {xmlDoc.NewElement(frameTag)}; // элемент входного файла
            frameElement->SetAttribute("number", it->first);

            for(auto line = signs.begin(); line != signs.end(); ++line)
            {
                tinyxml2::XMLElement * objectElement {xmlDoc.NewElement(objectTag)}; // элемент найденного объекта
                objectElement->SetAttribute("className", line->signName.c_str());
                objectElement->SetAttribute("x", line->boundRect.x);
                objectElement->SetAttribute("y", line->boundRect.y);
                objectElement->SetAttribute("width", line->boundRect.width);
                objectElement->SetAttribute("height", line->boundRect.height);
                frameElement->InsertEndChild(objectElement);
            }

            rootElement->InsertEndChild(frameElement);
        }
    }

    xmlDoc.SaveFile(pathToXML.c_str());
}

std::vector<marker::Sign> marker::SignStorage::read(long frameIndex)
{
   return storage[frameIndex];
}

std::vector<marker::Sign> marker::SignStorage::jumpForward(long &frameIndex)
{
    --frameIndex;

    for(;frameIndex >= 1; --frameIndex)
    {
        if(storage[frameIndex].size() != 0)
            return storage[frameIndex];
    }
    return storage[frameIndex];
}

std::vector<marker::Sign> marker::SignStorage::jumpBack(long &frameIndex, const long frameCount)
{
    ++frameIndex;

    for(;frameIndex <= frameCount; ++frameIndex)
    {
        if(storage[frameIndex].size() != 0)
           return storage[frameIndex];
    }
    return storage[frameIndex];
}

