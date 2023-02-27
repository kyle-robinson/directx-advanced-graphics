#pragma once
#ifndef CPPHELPERFUNCTIONS_H
#define CPPHELPERFUNCTIONS_H

#include <random>
#include <iostream>
#include <vector>
#include <queue>
#include <map>

namespace RandomGen
{
    // Random int
    template<typename T>
    T random( T range_from, T range_to )
    {
        std::random_device rand_dev;
        std::mt19937 generator( rand_dev() );
        std::uniform_int_distribution<T> distr( range_from, range_to );
        return distr( generator );
    }

    // Random float/double
    template<typename T>
    T randomFloat( T range_from, T range_to )
    {
        std::random_device rand_dev;
        std::mt19937 generator( rand_dev() );
        std::uniform_real_distribution<T> distr( range_from, range_to );
        return distr( generator );
    }

    static std::mt19937 generatorStatic;
    static int m_iSeed = 0;

    template<typename T>
    T random( T range_from, T range_to, int seed )
    {
        if ( seed != m_iSeed )
        {
            generatorStatic.seed( seed );
            m_iSeed = seed;
        }
        std::uniform_int_distribution<T> distr( range_from, range_to );
        return distr( generatorStatic );
    }

    template<typename T>
    T randomFloat( T range_from, T range_to, int seed )
    {
        //std::random_device   rand_dev;
        if ( seed != m_iSeed )
        {
            generatorStatic.seed( seed );
            m_iSeed = seed;
        }
        std::uniform_real_distribution<T> distr( range_from, range_to );
        return distr( generatorStatic );
    }

    inline void SetSeed( int seed )
    {
        generatorStatic.seed( 0 );
        m_iSeed = seed;
        generatorStatic.seed( m_iSeed );
    }

    inline int GetSeed() noexcept { return m_iSeed; }
}

namespace ListHelpers
{
    // Remove data from vectors
    template<typename T>
    bool RemoveFromVector( std::vector<T>* list, T* toRemove )
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

    // Remove data from multimap
    template<typename FirstType, typename SecondType>
    bool RemoveFromMutiMap( std::multimap<FirstType, SecondType>* map, SecondType* toRemove )
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
}

#include <locale>
#include <codecvt>
namespace StringHelpers
{
    inline std::wstring s2ws( const std::string& str )
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
        return converterX.from_bytes( str );
    }

    inline std::string ws2s( const std::wstring& wstr )
    {
        std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t> converterX;
        return converterX.to_bytes( wstr );
    }
}

#endif