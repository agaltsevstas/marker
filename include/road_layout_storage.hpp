#ifndef __ROAD_LAYOUT_STORAGE__
#define __ROAD_LAYOUT_STORAGE__

#include <opencv2/opencv.hpp>

#include "road_layout.hpp"

namespace marker
{

/*! @brief Класс отвечающий за сохранение/чтения дорожной разметки в/из xml файла
*/
class RoadLayoutStorage
{
public:
    /*! @brief Конструктор
     * @param _pathToXML - путь до xml файла для сохранения/чтения дорожной разметки
    */
    RoadLayoutStorage(const std::string &_pathToXML);

    /*! @brief Выполняет сохранение дорожной разметки конретного кадра
     * @param frameIndex - номер кадра
     * @param layout - вектор содержащий элементы дорожной разметки кадра
     * frameIndex
    */
    void write(long frameIndex, const std::vector<marker::RoadLayout> &layout);

    /*! @brief Выполняет чтение элементов дорожной разметки соответствующего
     * кадра.
     * @param frameIndex - номер кадра
     * @return вектор содержащий элементы дорожной разметки
    */
    std::vector<marker::RoadLayout> read(long frameIndex);

    ~RoadLayoutStorage();

private:
    const std::string pathToXML;
    std::map<long, std::vector<marker::RoadLayout>> storage;
};

}
#endif
