#ifndef __FORWARD_PLAYER__
#define __FORWARD_PLAYER__

#include <opencv2/opencv.hpp>

namespace marker
{

/*! @brief Класс обертка над cv::VideoCapture обспечивающий
 * удобный интерфейс доступа к кадрам при проигровании видео
 * в прямом направлении
 */
class ForwardPlayer
{
public:

    /*! @brief Конструктор
     * @param videoFile - путь до видео файла
     */
    ForwardPlayer(const std::string &videoFile);
    /*! @brief Возвращает следующий кадр, при проигровании видео
     * в прямом направлении. Возвращает пустую cv::Mat,
     * если достигнут конец видео файла
     */
    cv::Mat next();

    /*! @brief Возвращает предыдущий кадр, при проигровании видео
     * в прямом направлении. Возвращает пустую cv::Mat,
     * если достигнут конец видео файла
     */
    cv::Mat previous();

    /*! @brief Возвращает запрошенный кадр.
     * Дальнейшее воспроизведние ведется относительно
     * запрошенного кадра. Возвращает пустую cv::Mat в
     * случаии, если кадра с таким номер нет в видео файле
     * @param jumpFrameIndex - номер запрашиваемого кадра
     */
    cv::Mat jump(long jumpFrameIndex);

    /*! @brief Обертка над cv::VideoCapture::get(int propId).
     * Для детальной информации смотри документацию по OpenCV
     */
    inline double get(int propId) const {return cap.get(propId);}


private:
    cv::VideoCapture cap;
    long frameIndex;

private:
    cv::Mat getFrame(long frameIndex);


};

}

#endif
