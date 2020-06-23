#ifndef __SIGN_HPP__
#define __SIGN_HPP__

#include <opencv2/opencv.hpp>
#include <utility>

namespace marker
{

using SignTypes = std::map<int, std::string>;

/*! @brief Название поддерживаемых знаком и их код */
//const SignTypes signTypes{{0, "уступи дорогу"}, {1, "главная дорога"}, {2, "stop"}, {3, "пешеход. переход"}, {4, "огр. скорости 50"}};

/*! @brief Структура реализующая объект: дорожный знак
*/
struct Sign
{
    /*! @brief Конструктор, предназначенный для создания пустого объекта */
    Sign(){}

    /*! @brief Конструктор
     * @param _signType - код знака, должен соответствовать коду из signTypes
     * @param _boundRect - прямоугольник окаймляющий дорожный знак
    */

    Sign(std::string _signType, const cv::Rect &_boundRect);

    /*! @brief Конструктор, предназначенный для востановления знака из
     * его сериализации
     * @param serialized - cv::Mat содержая знак, сериализованный с помощью метода @ref toMat()
    */
    Sign(const cv::Mat &serialized);

    /*! @brief Сериализует знак в cv::Mat для последующего сохранения в xml
    */
    cv::Mat toMat() const;


    /*! @brief Проверяет содержиться ли точка pt внутри окаймляющего прямоугольника
     * @param pt - точка
    */
    inline bool contains (const cv::Point pt) const {return boundRect.contains(pt);}

    /*! @brief Обертка над cv::Rect::br() - возвращает правый нижний угол
     * окаймляющего прямоугольник
    */
    inline cv::Point br() const {return boundRect.br();}

    /*! @brief Обертка над cv::Rect::tl() - возвращает левый верхний угол
     * окаймляющего прямоугольник
    */
    inline cv::Point tl() const {return boundRect.tl();}

    /*! @brief Обертка над cv::Rect::size() - возвращает размер окаймляющего прямоугольник
    */
    inline cv::Size size() const {return boundRect.size();}

    /*! @brief Устанавливает координаты верхнего левого угла окаймляющего прямоугольника
     * @param x - x координата левого верхнего угла
     * @param y - y координата левого верхнего угла
    */
    inline void setTl(int x, int y) {boundRect.x = x; boundRect.y = y;}

    /*! @brief Устанавливает размер окаймляющего прямоугольника
     * @param width - ширина окаймляющего прямоугольника
     * @param height - высота окаймляющего прямоугольника
    */
    inline void setSize(int width, int height) {boundRect.width = width; boundRect.height = height;}

    std::string signName; ///< номер знака
    cv::Rect boundRect; ///< окаймляющий прямоугольник
};

/*! @brief Перегрузка оператора вывода в поток
*/
std::ostream & operator << (std::ostream &oss, const marker::Sign &sign);

/*! @brief Вспомогательная функция отрисовки знаков
 * @param indexSign - Индекс знака, с которого начинается отрисовка
 * @param frame - картинка на которой будут отрисованны знаки
 * @param signs - вектор знаков для отрисовки
 * @param signTypes - библиотека знаков
*/
void drawSigns(size_t indexSign, cv::Mat &frame, const std::vector<marker::Sign> &signs, const marker::SignTypes &signTypes);

/*! @brief Вспомогательная функция для сериализации ветора знаков в
 * cv::Mat
 * @param signs - вектор знаков
*/
cv::Mat convert(const std::vector<marker::Sign> &signs);

/*! @brief Вспомогательная функция для десериализации cv::Mat в
 * вектор знаков
 * @param values - сериализованный вектор дорожных знаков
*/

marker::Sign convert(std::string, cv::Rect);

/*! @brief Вспомогательная функция для загрузки типов знаков
 * из текстого файла
 * @param path - путь до текстового файла
*/
SignTypes loadSignTypes(const std::string &path);

}
#endif
