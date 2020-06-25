#include <opencv2/opencv.hpp>
#include <opencv2/tracking/tracker.hpp>

#include <boost/filesystem.hpp>
#include <boost/any.hpp>
#include <boost/variant.hpp>

#include "util.hpp"
#include "sign_marker.hpp"
#include "sign_storage.hpp"
#include "reverse_player.hpp"
#include "sign.hpp"

#define MAX_SIZE 3

namespace internal
{

    void checkBorder(const cv::Size frameSize, cv::Rect &bb)
    {
        if(bb.x < 0)
        {
            bb.x = 0;
        }
        if(bb.y < 0)
        {
            bb.y = 0;
        }
        if(bb.x + bb.width > frameSize.width)
        {
            bb.width -= bb.x + bb.width - frameSize.width;
        }
        if(bb.y + bb.height > frameSize.height )
        {
            bb.height -= bb.y + bb.height - frameSize.height;
        }
    }

    void mouseCallback(int event, int x, int y, int flags, void* userdata)
    {
        std::vector<boost::any> *data = static_cast<std::vector<boost::any>*>(userdata);

        bool &isChanged {*(boost::any_cast<bool*>(data->at(0)))};
        size_t &click {*(boost::any_cast<size_t*>(data->at(1)))};
        std::string &signName {*(boost::any_cast<std::string*>(data->at(2)))};
        cv::Mat &frameCopy {*(boost::any_cast<cv::Mat*>(data->at(3)))};
        std::vector<marker::Sign> &signs {*(boost::any_cast<std::vector<marker::Sign>*>(data->at(4)))};
        const marker::SignTypes &signTypes {*(boost::any_cast<const marker::SignTypes*>(data->at(5)))};

        cv::Point pt{x,y};

        static bool onMove{};
        static bool onScale{};
        static size_t selectedRect{};
        static cv::Point shift{};
        static cv::Size size{};
        static cv::Point br{};

        if(event & cv::EVENT_LBUTTONDOWN)
        {
            if(click && !signs.empty())
            {
                signs.push_back(marker::Sign(signName, cv::Rect(pt.x, pt.y, 100, 100)));
                marker::drawSigns(click, frameCopy, signs, signTypes);
                click = 0;
                signName.clear();
            }
            else if(!click && signs.empty() && !signName.empty())
            {
                signs.push_back(marker::Sign(signName, cv::Rect(pt.x, pt.y, 100, 100)));
                marker::drawSigns(click, frameCopy, signs, signTypes);
//                signName.clear();
            }

            for(size_t index = 0; index < signs.size(); ++index)
            {                
                if(sqrt((signs[index].br() - pt).ddot(signs[index].br() - pt)) < 8.)
                {
                    br = signs[index].br();
                    size = signs[index].size();
                    onScale = true;
                    selectedRect = index;
                    break;
                }

                if(signs[index].contains(pt))
                {
                    selectedRect = index;
                    shift = signs[index].tl() - pt;
                    onMove = true;
                    break;
                }               
            }
        }
        else if(event & cv::EVENT_LBUTTONUP)
        {
            onMove = false;
            onScale = false;
            isChanged = true;
        }
        else if(flags & cv::EVENT_FLAG_LBUTTON)
        {
            if(onMove)
            {
                if(shift.x + pt.x < 0)
                {
                    pt.x -= pt.x + shift.x;
                }
                if(shift.y + pt.y < 0)
                {
                    pt.y -= pt.y + shift.y;
                }
                if(signs[selectedRect].boundRect.width + shift.x + pt.x > frameCopy.cols)
                {
                    pt.x -= pt.x + shift.x + signs[selectedRect].boundRect.width - frameCopy.cols;
                }
                if(signs[selectedRect].boundRect.height + shift.y + pt.y > frameCopy.rows)
                {
                    pt.y -= pt.y + shift.y + signs[selectedRect].boundRect.height - frameCopy.rows;
                }

                signs[selectedRect].setTl(shift.x + pt.x, shift.y + pt.y);
            }
            else if(onScale)
            {
                if(pt.x < signs[selectedRect].boundRect.x)
                {
                    pt.x += signs[selectedRect].boundRect.x - pt.x + 1;
                }
                if(pt.y < signs[selectedRect].boundRect.y)
                {
                    pt.y += signs[selectedRect].boundRect.y - pt.y + 1;
                }
                if(pt.x > frameCopy.cols)
                {
                    pt.x -= pt.x - frameCopy.cols;
                }
                if(pt.y > frameCopy.rows)
                {
                    pt.y -= pt.y - frameCopy.rows;
                }

                signs[selectedRect].setSize(size.width + (pt.x - br.x), size.height + (pt.y - br.y));
            }
        }
        else if(event & cv::EVENT_RBUTTONDOWN)
        {
            const auto pos = std::remove_if(signs.begin(), signs.end(), [pt](const marker::Sign &sign)
            {
                return sign.contains(pt);
            });

            signs.erase(pos, signs.end());
            isChanged = true;
        }
    }

    std::string addSign(std::string& signNameSaved, const marker::SignTypes &signTypes)
    {
        int signsIndex {};

        std::vector<std::string> editHelpMessage
        {
            std::to_string( signsIndex),
            "saved:",
            "ctrl+ \'v\' - load object to frame",
            "ctrl - save object"
        };

        const std::string winName{"add object"};
        cv::namedWindow(winName);

        cv::Mat black(600, 640, CV_8UC3, cv::Scalar::all(0));

        cv::Point pt(10, 65);

        for(marker::SignTypes::const_iterator it = signTypes.cbegin(); it != signTypes.cend(); ++it)
        {
            cv::putText(black, it->second, pt, cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(0, 169, 255));

            pt.y += 15;
        }

        std::string str{};

        std::string signName{};

        cv::Mat frame;

        for(;;)
        {
            black.copyTo(frame);

            std::string input_message {"Введенные символы: " + str};
            cv::Scalar color_input_message{cv::Scalar(0, 255, 0)};
            std::string message{"Неизвестный объект: " + str};
            cv::Scalar color_message{cv::Scalar(0, 0, 255)};
            cv::Point drawPoint(black.cols - 350, black.rows - 20);

            bool condition {};

            if(!signNameSaved.empty())
            {
                editHelpMessage.erase(editHelpMessage.begin(), editHelpMessage.begin() + editHelpMessage.size() - 3);
                editHelpMessage.insert(editHelpMessage.begin(), signNameSaved);
            }
            for(std::vector<std::string>::const_iterator it = editHelpMessage.cbegin(); it != editHelpMessage.end(); ++it)
            {
                cv::putText(frame, *it, drawPoint, cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(0,0,255));
                drawPoint.y -= 30;
            }

            try
            {
                for(marker::SignTypes::const_iterator it = signTypes.begin(); it != signTypes.end(); ++it)
                {
                    if(str == it->second)
                    {
                        color_message = cv::Scalar(0, 255, 0);
                        message = "Новый объект: " + str;
                        condition = true;
                        break;
                    }
                }
            }
            catch(...){}

            cv::putText(frame, input_message, cv::Point(10,10), cv::FONT_HERSHEY_COMPLEX, 0.4, color_input_message, 1);
            cv::putText(frame, message, cv::Point(10,30), cv::FONT_HERSHEY_COMPLEX, 0.7, color_message, 1);

            cv::imshow(winName, frame);
            int key = cv::waitKey(0) & 0xff;

            if(key == 8)
            {
                if(str.length() > 0) // удалить символ
                {
                    str.erase(str.length() - 1);
                }
            }
            else if(key == 32 && condition == true) // добавить объект по space
            {
                signName = str;
                break;
            }
            else if(key >= 33 && key <= 127) // добавить символ
            {
                str +=  key;
            }
            else if(key == 227 && condition) // сохранить объект
            {
                signNameSaved = str;
            }
            else if(key == 27) //ECS
            {
                cv::destroyWindow(winName);
                throw internal::Cancel{};
            }
        }

        cv::destroyWindow(winName);

        return signName;
    }

    bool signChanged(const long& frameIndex, const std::vector<marker::Sign>& signs, marker::SignStorage& signStorage, const std::string &group)
    {
        std::vector<marker::Sign> signs_copy = signStorage.read(frameIndex);
        if((signs.size() < signs_copy.size())) // удалить объект со всех кадров
        {
            long objectIndex {};
            std::string signName {};
            for (std::vector<marker::Sign>::const_iterator it_signs_copy = signs_copy.begin(); it_signs_copy != signs_copy.end(); ++it_signs_copy) // поиск объекта по всему массиву
            {
                std::vector<marker::Sign>::const_iterator posObjectRemove = std::find_if(signs.begin(), signs.end(), [it_signs_copy](const marker::Sign &it_signs)
                {
                    if (it_signs.signName == it_signs_copy->signName && it_signs.boundRect == it_signs_copy->boundRect)
                    {
                        return true;
                    }
                });
                if (posObjectRemove == signs.end()) // объект найден
                {
                    objectIndex = it_signs_copy-signs_copy.begin();
                    signName = it_signs_copy->signName;
                }
            }

            for(long i = frameIndex; i > 1; --i) // удаление объекта со всего массива
            {
                signs_copy = signStorage.read(i);
                if(signs_copy.size() > objectIndex && signs_copy.at(objectIndex).signName == signName)
                    signs_copy.erase(signs_copy.begin() + objectIndex);
                else
                    break;
                signStorage.write(i, signs_copy, group);
            }
            return false;
        }
        else // добавить объект на кадр
        {
            signStorage.write(frameIndex, signs, group);
            return true;
        }
    }
}

void marker::markSigns(const std::string &video, const marker::SignTypes &signTypes, const std::string &group)
{
    marker::ReversePlayer cap{video};
    marker::SignStorage signStorage{video, group};

    int signsIndex {};

    const std::string frameCount {std::to_string(int(cap.get(cv::CAP_PROP_FRAME_COUNT)))};
    const std::string helpMessage {"press space to enable \'edit\' mode"};
    std::vector<std::string> editHelpMessage
    {
        std::to_string(signsIndex),
        "\'1-9\' - saved objects from frames, now:",
        "ctrl+ \'1-9\' - load signs from frames",
        "\'j\' - jump to particular frame",
        "\'n\' - next marked frame",
        "\'p\' - previous marked frame",
        "\'n\' - next frame",
        "\'p\' - previous frame",
        "mouse right click - delete object",
        "\'a\' - add new object",
        "shift + \'a\' enable/disable autotracking",
        "space - forward/pause"
    };
    std::string signName {};
    std::string signNameSaved {};

    const std::string winName {"marker"};
    cv::namedWindow(winName, CV_WINDOW_NORMAL);

    bool signChanged {};
    size_t click {}; // 0 - объект не добавлен, 1 - объект введен, 2 - объект добавлен
    cv::Mat frameCopy;
    std::vector<marker::Sign> signs{};
    std::map<int, std::pair<int, std::vector<marker::Sign>>> signsSaved {};
    std::vector<boost::any> flagAndSignsAndFrameSize;
    flagAndSignsAndFrameSize.emplace_back(&signChanged);
    flagAndSignsAndFrameSize.emplace_back(&click);
    flagAndSignsAndFrameSize.emplace_back(&signName);
    flagAndSignsAndFrameSize.emplace_back(&frameCopy);
    flagAndSignsAndFrameSize.emplace_back(&signs);
    flagAndSignsAndFrameSize.emplace_back(&signTypes);
    cv::setMouseCallback(winName, internal::mouseCallback, &flagAndSignsAndFrameSize);

    bool onPause {true};
    bool autoTracking {};
    bool autoTrackingPause {};
    bool help {};
    cv::Mat frame = cap.next();
    long frameIndex = static_cast<long>(cap.get(cv::CAP_PROP_POS_FRAMES));
    signs = signStorage.read(frameIndex);

    for(;;)
    {
        if(!onPause)
        {
            frame = cap.next();
            frameIndex = static_cast<long>(cap.get(cv::CAP_PROP_POS_FRAMES));
            signs = signStorage.read(frameIndex);
        }

        if(frame.empty())
            break;

        frame.copyTo(frameCopy);
        cv::putText(frameCopy, '#' + std::to_string(frameIndex) + '/' + frameCount, cv::Point(5,20), cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0,255,0), 2);

        if(autoTracking)
            cv::putText(frameCopy, "autotracking enable", cv::Point(5, 50), cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
        else
            cv::putText(frameCopy, "autotracking disable", cv::Point(5, 50), cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(255, 0, 0), 2);

        if(!onPause && help)
            cv::putText(frameCopy, helpMessage, cv::Point(frame.cols - 500, frame.rows - 100), cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(0,0,255));
        else if(onPause && help)
        {
            cv::Point drawPoint(frame.cols - 550, frame.rows - 20);

            if(!signsSaved.empty())
            {
                    editHelpMessage.erase(editHelpMessage.begin(), editHelpMessage.begin() + editHelpMessage.size() - 8);
                    size_t countIndex {1};
                    std::string str {};

                    for (auto itemSaved : signsSaved)
                    {
                        str += (str.empty() ? "" : "; ") + std::to_string(itemSaved.first + 1) + ", " + "frame: " + std::to_string(itemSaved.second.first);

                        if((countIndex == 1 || countIndex % MAX_SIZE != 0) && ((signsSaved.size() - countIndex) > 0))
                        {
                            ++countIndex;
                            continue;
                        }
                        else if(countIndex == 1 || countIndex % MAX_SIZE == 0 || ((signsSaved.size() - countIndex) == 0))
                        {
                            editHelpMessage.insert(editHelpMessage.begin(), str);
                            str.clear();
                        }

                        ++countIndex;
                    }

            }
            for(std::vector<std::string>::const_iterator it = editHelpMessage.cbegin(); it != editHelpMessage.end(); ++it)
            {
                cv::putText(frameCopy, *it, drawPoint, cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(0,0,255));
                drawPoint.y -= 30;
            }
        }


        marker::drawSigns(0, frameCopy, signs, signTypes);

        cv::imshow(winName, frameCopy);

        int key = cv::waitKey(25) & 0xff;

        if(key == 27) // esc
            break;
        else if(key == 32) // вперед/пауза по space
        {
            if(autoTracking)
                autoTrackingPause = !autoTrackingPause;
            else
                onPause = !onPause;
        }
        else if(key == 'h' || key == 210) // help
        {
            help ^= 1;
        }
        else if((key == 'p' || key == 218) && onPause) // предыдущий кадр
        {
            frame = cap.previous();
            frameIndex = static_cast<long>(cap.get(cv::CAP_PROP_POS_FRAMES));
            signs = signStorage.read(frameIndex);
        }
        else if((key == 'n' || key == 212) && onPause) // следующий кадр
        {
            frame = cap.next();
            frameIndex = static_cast<long>(cap.get(cv::CAP_PROP_POS_FRAMES));
            signs = signStorage.read(frameIndex);
        }
        else if((key == 'w' || key == 195) && onPause) // следующий кадр размеченный кадр
        {
            signs = signStorage.jumpForward(frameIndex);
            frame = cap.jump(frameIndex);
        }
        else if((key == 's' || key == 217) && onPause) // предыдущий кадр размеченный кадр
        {
            signs = signStorage.jumpBack(frameIndex, std::stoi(frameCount));
            frame = cap.jump(frameIndex);
        }
        else if((key == 'j' || key == 207) && onPause) // перепрыгнуть к кадру
        {
            try
            {
                frame = cap.jump(internal::jump(frameCount));
                frameIndex = static_cast<long>(cap.get(cv::CAP_PROP_POS_FRAMES));
                signs = signStorage.read(frameIndex);
            }
            catch(const internal::Cancel &){}
        }
        else if((key == 'a' || key == 198) && onPause) // добавить объект
        {
            try
            {
                signName = internal::addSign(signNameSaved, signTypes);
                signChanged = true;
                click = signs.size();
            }
            catch(const internal::Cancel &){}
        }
        else if((key == 'A' || key == 230) && onPause) // запись
        {
            autoTracking = !autoTracking;
            autoTrackingPause = false;
        }
        else if(((key >= 49 && key <= 57) || key == 227 || key == 118 || key == 205) && onPause) // сохранить/вставить объект
        {
            int previosKey = key;
            signsIndex = key - 49;
            key = cv::waitKey(200) & 0xff;

            if(key == 255 && (previosKey >= 49 && previosKey <= 57)) // вставить, по '1-9'
            {
                auto searchIndex = signsSaved.find(signsIndex);

                if(searchIndex != signsSaved.end())
                {
                    signs = signsSaved[signsIndex].second;

                    signChanged = true;
                }
            }
            else if(((key >= 49 && key <= 57) || key == 227) && abs(key-previosKey) > 8) // сохранить
            {
                signsIndex += key - 227;
                signsSaved[signsIndex].first = static_cast<int>(frameIndex);
                signsSaved[signsIndex].second = signs;
            }
            if((key == 118 || key == 205 || key == 227) && !signNameSaved.empty()) // вставить, по ctrl + 'v'
            {
                click = 1;
                signName = signNameSaved;
            }
        }
        if(autoTracking && !autoTrackingPause)
        {

            if(signs.empty())
            {
                autoTracking = false;
            }
            else
            {
                cv::Mat previousFrame;
                frame.copyTo(previousFrame);

                frame = cap.next();
                frameIndex = static_cast<long>(cap.get(cv::CAP_PROP_POS_FRAMES));

                for(std::vector<marker::Sign>::iterator sign = signs.begin(); sign != signs.end(); ++sign)
                {
                    const cv::Rect previousBB = sign->boundRect;

                    cv::TrackerMedianFlow::Params param;
                    param.pointsInGrid = 20;

                    cv::Ptr<cv::TrackerMedianFlow> tracker = cv::TrackerMedianFlow::create(param);

                    if(!tracker->init(previousFrame, previousBB))
                        continue;

                    cv::Rect2d bb;
                    if(tracker->update(frame, bb))
                        sign->boundRect = bb;
                    else
                        autoTrackingPause |= true;

                    internal::checkBorder(previousFrame.size(), sign->boundRect);
                }

                signChanged = true;
            }
        }

        if(signChanged)
        {
            signChanged = internal::signChanged(frameIndex, signs, signStorage, group);
        }
    }

    cv::destroyWindow(winName);
}
