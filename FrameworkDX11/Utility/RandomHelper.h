#pragma once
#ifndef RANDOMHELPER_H
#define RANDOMHELPER_H

#include <random>

/// <summary>
/// Used to generate random numbers with seeding.
/// </summary>
namespace RandomHelper
{
    template<typename T>
    inline T Random( T range_from, T range_to )
    {
        std::random_device rand_dev;
        std::mt19937 generator( rand_dev() );
        std::uniform_int_distribution<T> distr( range_from, range_to );
        return distr( generator );
    }

    template<typename T>
    inline T RandomFloat( T range_from, T range_to )
    {
        std::random_device rand_dev;
        std::mt19937 generator( rand_dev() );
        std::uniform_real_distribution<T> distr( range_from, range_to );
        return distr( generator );
    }

    static std::mt19937 generatorStatic;
    static int m_iSeed = 0;

    template<typename T>
    inline T Random( T range_from, T range_to, int seed )
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
    inline T RandomFloat( T range_from, T range_to, int seed )
    {
        if ( seed != m_iSeed )
        {
            generatorStatic.seed( seed );
            m_iSeed = seed;
        }
        std::uniform_real_distribution<T> distr( range_from, range_to );
        return distr( generatorStatic );
    }

    inline void SetSeed( int seed ) noexcept
    {
        generatorStatic.seed( 0 );
        m_iSeed = seed;
        generatorStatic.seed( m_iSeed );
    }

    inline int GetSeed() noexcept { return m_iSeed; }
};

#endif