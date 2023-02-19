#pragma once
#include <random>
#include<iostream>
#include<vector>
#include<queue>
#include<map>

using namespace std;

namespace RandomGen {
   
    //randome int
    template<typename T>
    T random(T range_from, T range_to) {
        std::random_device   rand_dev;
        std::mt19937                        generator(rand_dev());
        std::uniform_int_distribution<T>    distr(range_from, range_to);
        return distr(generator);
    }
    //randome float,double
    template<typename T>
    T randomFloat(T range_from, T range_to) {
        std::random_device   rand_dev;
        std::mt19937                        generator(rand_dev());
        std::uniform_real_distribution<T>    distr(range_from, range_to);
        return distr(generator);
    }

    static std::mt19937 generatorStatic;
    static int seed = 0;

    template<typename T>
    T random(T range_from, T range_to, int Seed) {
        //std::random_device   rand_dev;
        if (Seed != seed) {
            generatorStatic.seed(Seed);
            seed = Seed;
        }
        std::uniform_int_distribution<T>    distr(range_from, range_to);
        return distr(generatorStatic);
    }
    

    template<typename T>
    T randomFloat(T range_from, T range_to, int Seed) {
        //std::random_device   rand_dev;
        if (Seed != seed) {
            generatorStatic.seed(Seed);
            seed = Seed;
        }
        std::uniform_real_distribution<T>    distr(range_from, range_to);
        return distr(generatorStatic);
    }

    inline void SetSeed(int Seed) {
        generatorStatic.seed(0);
        
        seed = Seed;
        generatorStatic.seed(seed);
    }
   inline int GetSeed() { return seed; }
}

namespace ListHelpers {
  
   //remove data from vector
    template<typename T>
     bool RemoveFromVector(vector<T>* List, T *ToRemove) {
        vector<T>::template  iterator iter = List->begin();
        while (iter != List->end()) {
            if (*iter == *ToRemove) {
                iter = List->erase(iter);
                return true;
            }
            else
            {
                ++iter;
            }
        }
        return false;
    }

    //remove data from MutiMap
    template<typename FirstType, typename SecondType>
    bool RemoveFromMutiMap(std::multimap<FirstType, SecondType> *Map, SecondType *ToRemove) {
        std::multimap<FirstType, SecondType>::template iterator iter = Map->begin();
        while (iter != Map->end()) {
            if (iter->second == *ToRemove) {
                iter = Map->erase(iter);
            }
            else
            {
                ++iter;
            }
        }
        return true;
    }





}


#include<locale>
#include<codecvt>
namespace StringHelpers {

   inline std::wstring s2ws(const std::string& str)
    {
        //using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

        return converterX.from_bytes(str);
    }

  inline  std::string ws2s(const std::wstring& wstr)
    {
        //using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t> converterX;

        return converterX.to_bytes(wstr);
    }

}
