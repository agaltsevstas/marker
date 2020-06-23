#ifndef __SIGN_STORAGE__
#define __SIGN_STORAGE__

#include <opencv2/opencv.hpp>

#include "sign.hpp"
#include "util.hpp"
#include "tinyxml2.h"

namespace marker
{

/*! @brief Класс отвечающий за сохранение/чтения дорожных знаков в/из xml файла
*/
class SignStorage
{
public:

    /*! @brief Конструктор
     * @param _pathToXML - путь до xml файла для сохранения/чтения дорожных знаков
    */
    SignStorage(const std::string &_pathToXML, const std::string &group);

    /*! @brief Выполняет сохранение дорожных знаков конретного кадра
     * @param frameIndex - номер кадра
     * @param signs - вектор содержащий дорожные знаки конкретного кадра
    */
    void write(long frameIndex, const std::vector<marker::Sign> &signs, const std::string group);

    /*! @brief Выполняет чтение дорожных знаков соответствующего кадра.
     * @param frameIndex - номер кадра
    */
    std::vector<marker::Sign> read(long frameIndex);

    /*! @brief Выполняет прыжок вперед к размеченному кадру дорожных знаков.
     * @param frameIndex - номер кадра
    */
    std::vector<marker::Sign> jumpForward(long &frameIndex);

    /*! @brief Выполняет прыжок назад к размеченному кадру дорожных знаков.
     * @param frameIndex - номер кадра
    */
    std::vector<marker::Sign> jumpBack(long &frameIndex, const long frameCount);


private:

    const std::string path;
    const char* rootTag = "marker";
    const char* inputTag = "video";
    const char* frameTag = "frame";
    const char* objectTag ="object";
    std::vector<marker::Sign> objects_in_frame;
    std::map<long, std::vector<marker::Sign>> storage;
};

}
#endif
