#ifndef __ROAD_LAYOUT_HPP__
#define __ROAD_LAYOUT_HPP__

#include <list>

#include <opencv2/opencv.hpp>


namespace marker
{

using RoadLayoutTypes = std::map<int, std::string>;

const RoadLayoutTypes roadLayoutTypes{{0, "contin"}, {1, "double_contin"}};

/*! @brief Структура реализующая объект: дорожная разметка
*/
struct RoadLayout
{
    /*! @brief конструктор
     * @param _type - код дорожной разметки, один из roadLayoutTypes
   */
    RoadLayout(int _type);

    /*! @brief конструктор
     * @param serialized - cv::Mat содержая элемент дорожной разметки, сериализованный с помощью метода @ref toMat()
   */
    RoadLayout(const cv::Mat &serialized);

    /*! @brief Сериализует объект в cv::Mat для последующего сохранения в xml
   */
    cv::Mat toMat() const;

    int type; ///< код дорожной разметки, один из roadLayoutTypes
    std::vector<cv::Point> points; ///< вектор точек, определяющий дорожную разметку
};

void drawRoadLayout(cv::Mat &frame, const std::vector<marker::RoadLayout> &roadLayouts);

}

#endif
