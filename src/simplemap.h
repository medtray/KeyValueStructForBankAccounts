#pragma once
#include "KeyValueUnit.h"
#include <cstddef>
#include <mutex>
#include <shared_mutex>
#include<vector>
#include <regex>
#include <numeric>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

using namespace std;

template <typename Key, typename Value>

class simplemap
{

    public:

    int tableSize;

    simplemap(int tab_size):tableSize(tab_size), //constructor of the class
    MainStruct(tableSize),mutexes(tableSize)  
        
    {
    }

    std::vector<KeyValueUnit<Key, Value>*> MainStruct;  //define the main structure of key/value stores
    std::vector<boost::shared_mutex> mutexes; //define the mutexes

    


    public:
    
    //get a value by key
    std::pair<Value, bool>  lookup(const Key &key)
    {
        Value value;
        
        unsigned long hashValue = key%tableSize;
        //lock the bucket
        boost::shared_lock<boost::shared_mutex> m(mutexes[hashValue]);
        KeyValueUnit<Key, Value> *unit = MainStruct[hashValue];
            

        while (unit != NULL) {
            //if the key of the unit is equal to the given key, return the value
            if (unit->getKey() == key) {
                value = unit->getValue();
                return std::make_pair(value, true);
            }

            unit = unit->getpoinTo();
        }
        m.unlock();
        return std::make_pair(0, false);
    }

 
    //put a key/value entity in the key/value structure
    bool insert(const Key &key, const Value &value)
    {

        //std::hash<std::string> hash_fn;
        //unsigned long hashValue = hash_fn((key))%tableSize;
        unsigned long hashValue = key%tableSize;
        boost::unique_lock<boost::shared_mutex> lock(mutexes[hashValue]);

        KeyValueUnit<Key, Value> *previousUnit = NULL;
        KeyValueUnit<Key, Value> *unit = MainStruct[hashValue];

        while (unit != NULL && unit->getKey() != key) {
            previousUnit = unit;
            unit = unit->getpoinTo();
        }

        if (unit == NULL) {
            unit = new KeyValueUnit<Key, Value>(key, value);

            if (previousUnit == NULL) {
                // the linked list is empty, so insert the entity as the first element in the bucket
                MainStruct[hashValue] = unit;

            } else {
                //in a given linked list, the last element points to the new entity
                previousUnit->setpoinTo(unit);
            }


        lock.unlock();
        return true;

        } else {
            lock.unlock();
            return false;
        }
        
    }


     //update a key/value entity in the key/value structure
    bool update(const Key &key, const Value &value)
    {

        //std::hash<std::string> hash_fn;
        //unsigned long hashValue = hash_fn((key))%tableSize;
        unsigned long hashValue = key%tableSize;

        
        boost::unique_lock<boost::shared_mutex> lock(mutexes[hashValue]);
        KeyValueUnit<Key, Value> *previousUnit = NULL;
        KeyValueUnit<Key, Value> *unit = MainStruct[hashValue];

        while (unit != NULL && unit->getKey() != key) {
            previousUnit = unit;
            unit = unit->getpoinTo();
        }

        if (unit == NULL) {
           
        return false;

        } else {
            // if the key already exists, just update the value of this key
            unit->setValue(value);
            return true;
        }
        
    }

    float sum_map()
    {
        float sum=0;
        for (int i=0;i<tableSize;i++)

        {

             KeyValueUnit<Key, Value> *unit = MainStruct[i];

        while (unit != NULL) {
            sum+=unit->getValue();
            unit = unit->getpoinTo();
        }

        }

        return sum;
    }

    //remove a key/value entity from the key/value structure
    bool remove(const Key &key)
    {
       
        unsigned long hashValue = key%tableSize;
        boost::unique_lock<boost::shared_mutex> lock(mutexes[hashValue]);
        KeyValueUnit<Key, Value> *previousUnit = NULL;
        KeyValueUnit<Key, Value> *unit = MainStruct[hashValue];

        while (unit != NULL && unit->getKey() != key) {
            previousUnit = unit;
            unit = unit->getpoinTo();
        }

        if (unit == NULL) {
            // the key is not found
            lock.unlock();
            return false;

        } else {
            if (previousUnit == NULL) {
                // the bucket contains the key/value entity that is pointed to by the deleted
                //entity
                MainStruct[hashValue] = unit->getpoinTo();

            } else {
                // the previous unit points to the key/value entity that is pointed to by the deleted
                //entity

                previousUnit->setpoinTo(unit->getpoinTo());
            }

            delete unit;
            lock.unlock();
            return true;
        }
        
    }

    void deposit(const Key &key1,const Key &key2,float d)
    {

        unsigned long hashValue1 = key1%tableSize;
        unsigned long hashValue2 = key2%tableSize;


        boost::unique_lock<boost::shared_mutex> m1(mutexes[hashValue1], boost::defer_lock);
        boost::unique_lock<boost::shared_mutex> m2(mutexes[hashValue2], boost::defer_lock);

        

       std::vector<int> locks_prepare;

        locks_prepare.push_back(0);
        locks_prepare.push_back(0);

        int success=0;
        while(!success)
        {
            bool mm1=m1.try_lock();
            bool mm2=m2.try_lock();
            if (mm1) {
                locks_prepare[0]=1;
        } else {
            locks_prepare[0]=0; 
        }

        if (mm2) {
                locks_prepare[1]=1;
        } else {
            locks_prepare[1]=0; 
        }

        if (locks_prepare[0]==1&&locks_prepare[1]==1)
        {

            KeyValueUnit<Key, Value> *unit = MainStruct[hashValue1];
            while (unit != NULL) {
                if (unit->getKey() == key1) {
                    unit->setValue(unit->getValue()-d);
                    unit=NULL;
                }
                else
                    unit = unit->getpoinTo();
            }

            KeyValueUnit<Key, Value> *unit2 = MainStruct[hashValue2];
            while (unit2 != NULL) {
                if (unit2->getKey() == key2) {
                    unit2->setValue(unit2->getValue()+d);
                    unit2=NULL;
                }
                else
                    unit2 = unit2->getpoinTo();
            }

            m1.unlock();
            m2.unlock();

            success=1;

        }

        else
        {
            if (locks_prepare[0]==1)
                m1.unlock();

            if (locks_prepare[1]==1)
                m2.unlock();
            
            
        }

    }
    }


    void one_thread_deposit(const Key &key1,const Key &key2,float d)
    {

        unsigned long hashValue1 = key1%tableSize;
        unsigned long hashValue2 = key2%tableSize;

        KeyValueUnit<Key, Value> *unit = MainStruct[hashValue1];
            while (unit != NULL) {
                if (unit->getKey() == key1) {
                    unit->setValue(unit->getValue()-d);
                    unit=NULL;
                }
                else
                    unit = unit->getpoinTo();
            }

            KeyValueUnit<Key, Value> *unit2 = MainStruct[hashValue2];
            while (unit2 != NULL) {
                if (unit2->getKey() == key2) {
                    unit2->setValue(unit2->getValue()+d);
                    unit2=NULL;
                }
                else
                    unit2 = unit2->getpoinTo();
            }

    }

    

    float balance()

    {
        std::vector<int> locks_prepare(tableSize, 0);

        float summ=0;

        std::vector<boost::shared_lock<boost::shared_mutex>> m(tableSize);

        for (int i=0;i<tableSize;i++)
        {
           m[i]=static_cast<boost::shared_lock<boost::shared_mutex>>(mutexes[i]);

        }
        
         summ=sum_map();

        return summ;

    }


    void apply(void (*f)(Key, Value)) {
        

        for (int i=0;i<tableSize;i++)

        {

             KeyValueUnit<Key, Value> *unit = MainStruct[i];

        while (unit != NULL) {
            
            f(unit->getKey(), unit->getValue());
            unit = unit->getpoinTo();
        }

        }
    }


   
};
