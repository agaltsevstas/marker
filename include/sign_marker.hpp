#ifndef __SIGN_MARKER_HPP__
#define __SIGN_MARKER_HPP__

#include <string>
#include "sign.hpp"

//namespace internal
//{
///*! @brief Структура, созданная специально для
// * прерывания операции ввода данных от пользователя
// */
//struct Cancel{};
//}

namespace marker
{
/*! @brief маркер дорожных знаков
 * @param video - путь к размечаемому видео
 * @param signTypes - библиотека знаков
*/
void markSigns(const std::string &video, const marker::SignTypes &signTypes, const std::string &group);

/*! @brief экспортер дорожных знаков
 * @param video - путь к размечаному видео
* @param signTypes - библиотека знаков
*/
void exportSigns(const std::string &video, const std::string &group);

/*! @brief экспортер дорожных знаков
 * @param video - путь к размечаному видео
 * @param signTypes - библиотека знаков
*/
void exportSignsWithTXT(const std::string &video, const std::string &group);
}

#endif
