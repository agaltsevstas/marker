#ifdef DEEPLABV2_FOUND

#include <opencv2/opencv.hpp>

namespace marker
{

/*! @brief Абстрактный класс реализующий автоматическую сегментацию дорожной сцены
 */
    class Segmentation
    {
    public:

        /*! @brief Производит автоматическую сегментацию
         * @param frame - входное изображение дорожной сцены
         * @return cv::Mat, пиксель которой соотвествует коду наиболее вероятного объекта
        */
        virtual cv::Mat segment(const cv::Mat &frame) = 0;

        /*! @brief Создает экземпляр автоматического разметчика
         * @param prototxt - путь к prototxt файлу модели DeepLabV2 (лежит в папке data, deploy_4.prototxt)
         * @param weights - путь к caffemodel файлу весов DeepLabV2 (лежит в папке data, train_iter_20000_4.caffemodel)
         * @param internalSize - размер подоваемого на вход кадра
         * @param deviceId - id устройства на котором выполнять расчеты
        */
        static cv::Ptr<Segmentation> create(const std::string &prototxt, const std::string &caffemodel,
                                            const cv::Size &internalSize, int deviceId = 0);


        virtual ~Segmentation() {}
    };
}
#endif
