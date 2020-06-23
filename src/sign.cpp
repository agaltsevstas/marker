#include <fstream>
#include "sign.hpp"

void marker::drawSigns(size_t indexSign, cv::Mat &frame, const std::vector<marker::Sign> &signs, const marker::SignTypes &signTypes)
{
    for(std::vector<marker::Sign>::const_iterator sign = signs.cbegin() + indexSign; sign != signs.cend(); ++sign)
    {
        cv::rectangle(frame, sign->boundRect, cv::Scalar(0, 155, 255), 1);
        cv::circle(frame, sign->boundRect.br(), 5, cv::Scalar(0, 185, 255), 2);

        std::string capture {sign->signName};

        cv::putText(frame, capture, sign->boundRect.tl(), cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(0, 255, 0), 1);
    }
}

marker::Sign marker::convert(std::string signName, cv::Rect boundRect)
{
    CV_Assert(signName != "");
    CV_Assert(!boundRect.empty());

    marker::Sign object;

    object.signName = signName;
    object.boundRect.x = boundRect.x;
    object.boundRect.y = boundRect.y;
    object.boundRect.width = boundRect.width;
    object.boundRect.height = boundRect.height;

    return object;
}

marker::Sign::Sign(std::string _signName, const cv::Rect &_boundRect):
    signName{_signName}, boundRect{_boundRect}
{
    // CV_Assert(signTypes.find(signType) != signTypes.end());
}

std::ostream &marker::operator <<(std::ostream &oss, const marker::Sign &sign)
{
    oss << sign.signName << " " << sign.tl().x << " " << sign.tl().y;
    return  oss << " " << sign.br().x << " " << sign.br().y << std::endl;
}

marker::SignTypes marker::loadSignTypes(const std::string &path)
{

    SignTypes signTypes{};

    std::ifstream inFile;

    inFile.open(path);
    CV_Assert(inFile);

    std::string line;
    int id{};
    while (std::getline(inFile, line))
        signTypes[id++] = line;

    inFile.close();

    return signTypes;
}
