#ifndef __ROAD_SCENE_STORAGE__
#define __ROAD_SCENE_STORAGE__
#include <opencv2/opencv.hpp>

#include "road_scene.hpp"

namespace marker
{

/*! @brief Класс реализующий сохранение/чтение объектов дорожной сцены в/из xml файла
*/
class RoadSceneStorage
{
public:
    /*! @brief Конструктор
     * @param _pathToXML - путь до xml файла для сохранения/чтения объектов дорожной сцены
    */
    RoadSceneStorage(const std::string &_pathToXML);

    /*! @brief Выполняет сохранение объектов дорожной сцены конретного кадра
     * @param frameIndex - номер кадра
     * @param scene - вектор содержащий объекты дорожной сцены конкретного
     * кадра
    */
    void write(long frameIndex, const std::vector<marker::RoadScene> &scene);

    /*! @brief Выполняет чтение объектов дорожной сцены соответствующего кадра
     * @param frameIndex - номер кадра
    */
    std::vector<marker::RoadScene> read(long frameIndex);

    ~RoadSceneStorage();

private:
    const std::string pathToXML;
    std::map<long, std::vector<marker::RoadScene>> storage;
};

}
#endif

