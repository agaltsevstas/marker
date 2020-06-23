#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <opencv2/opencv.hpp>

namespace internal
{
/*! @brief Выводит диалоговое окно цифрового ввода
 * данных от пользователя
 * @param frameCount - количество кадров
 */
long jump(const std::string frameCount);

/*! @brief Структура, созданная специально для
 * прерывания операции ввода данных от пользователя
 */
struct Cancel{};

/*! @brief Генерирует путь для сохранения в xml файле
 * @param video - путь до видео файла
 */
std::string getPath(const std::string &video);

/*! @brief Генерирует имя файлом
 * @param video - путь до видео файла
 */
std::string getFileName(const std::string &video);

/*! @brief Генерирует путь к папке с файлом
 * @param video - путь до видео файла
 */
std::string getSceneXMLPath(const std::string &video);

/*! @brief Генерирует путь для сохранения/чтения xml файла
 * с дорожными знаками по входному видео файлу
 * @param video - путь до видео файла
 */
std::string getSignXMLPath(const std::string &video, const std::string &group);

/*! @brief Генерирует путь для сохранения/чтения xml файла
 * с дорожной разметкой по входному видео файлу
 * @param video - путь до видео файла
 */
std::string getLayoutXMLPath(const std::string &video);

}

#endif
