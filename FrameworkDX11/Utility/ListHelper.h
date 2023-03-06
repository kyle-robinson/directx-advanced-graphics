#pragma once
#ifndef LISTHELPER_H
#define LISTHELPER_H

#include <vector>
#include <map>

/// <summary>
/// Primarily used to convert remove items from lists.
/// </summary>
namespace ListHelper
{
    template<typename T>
    inline bool RemoveFromVector( std::vector<T>* list, T* toRemove )
    {
        std::vector<T>::template iterator iter = list->begin();
        while ( iter != list->end() )
        {
            if ( *iter == *toRemove )
            {
                iter = list->erase( iter );
                return true;
            }
            else
            {
                ++iter;
            }
        }
        return false;
    }

    template<typename FirstType, typename SecondType>
    inline bool RemoveFromMutiMap( std::multimap<FirstType, SecondType>* map, SecondType* toRemove )
    {
        std::multimap<FirstType, SecondType>::template iterator iter = map->begin();
        while ( iter != map->end() )
        {
            if ( iter->second == *toRemove )
            {
                iter = map->erase( iter );
            }
            else
            {
                ++iter;
            }
        }
        return true;
    }
};

#endif