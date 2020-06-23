#ifndef __ROAD_SCENE_HPP__
#define __ROAD_SCENE_HPP__

#include <opencv2/opencv.hpp>
#include <vector>

namespace marker
{

/*! @brief Сегментируемые объекты и их код
*/
const std::map<int, std::string> objectTypes{{0, "human"}, {1, "car"}, {2, "road"}};
const std::map<int, cv::Scalar> object2color{{0, cv::Scalar(0,0,255)}, {1, cv::Scalar(0,255,0)}, {2, cv::Scalar::all(50)}};

/*! @brief Структура реализующая объект: элемент дорожной сцены
*/
struct RoadScene
{

    /*! @brief Конструктор
     * @param _objectType - код объекта, один из objectTypes
    */
    RoadScene(int _objectType);

    /*! @brief Конструктор
     * @param _objectType - код объекта, код объекта, один из objectTypes
     * @param _points - точки окаймляющего контура объекта
    */
    RoadScene(int _objectType, const std::vector<cv::Point> &_points): type{_objectType}, points{_points}{}

    /*! @brief Конструктор, предназначенный для востановления объекта из
     * его сериализации
     * @param serialized - cv::Mat содержая объект, сериализованный с помощью метода @ref toMat()
    */
    RoadScene(cv::Mat &serialized);

    /*! @brief Сериализует объект в cv::Mat для последующего сохранения в xml
    */
    cv::Mat toMat() const;

    int type; ///< код объекта, один из objectTypes
    std::vector<cv::Point> points; ///< точки окаймляющего контура объекта
};

/*! @brief отладочная функция раскраски сегментов(объектов)
 * @param segmented - cv::Mat, каждый пиксель которой код объекта из objectTypes
*/

cv::Mat colorize(const cv::Mat &segmented);

/*! @brief вспомогательная функция отрисовки знаков
 * @param roadScene - вектор объектов для отрисовки
 * @param frame - картинка, на которой будут отрисованны знаки
*/
void drawRoadScene(const std::vector<marker::RoadScene> &roadScene, cv::Mat &frame);

}

#endif
