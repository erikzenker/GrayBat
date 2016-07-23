#pragma once

// STL
#include <map>                /* std::map */
#include <functional>         /* std::function */
#include <condition_variable> /* std::condition_variable */
#include <mutex>              /* std::mutex, std::lock_guard, std::unique_lock*/
#include <queue>              /* std::queue */
#include <utility>            /* std::forward */
#include <chrono>             /* std::chrono::milliseconds */
#include <atomic>             /* std::atomic */

// BOOST
#include <boost/optional.hpp>

// HANA
#include <boost/hana.hpp>
namespace hana = boost::hana;

namespace utils {

	template <typename... Tail> struct MultiKeyMapType;

	template <typename Key, typename Value>
	struct MultiKeyMapType<Key, Value> {
		typedef std::map<Key, Value> type;
	};

	template <typename Key, typename... Tail>
	struct MultiKeyMapType<Key, Tail...>  {
		typedef std::map<Key, typename MultiKeyMapType<Tail...>::type > type;
	};


	/**
     * @brief Collects all values of a map subtree into the values container and its
     *        corresponding keys into the key container
     *
     */
	template <int KeysLeft>
	struct SubTreeValues {

		template <typename T_Map, typename T_ValuesCT, typename T_KeysCT, typename T_Tuple>
		void operator()(T_Map &map, T_ValuesCT & values, T_KeysCT& keys, T_Tuple & tuple){
			auto begin = map.begin();
			auto end   = map.end();
			for(auto it = begin; it != end; ++it){
				auto nextTuple = hana::concat(tuple, hana::make_tuple(it->first));
				SubTreeValues<KeysLeft-1>()(it->second, values, keys, nextTuple);
			}

		}

		template <typename T_Map, typename T_ValuesCT, typename T_Keys>
		void operator()(T_Map &map, T_ValuesCT &values, T_Keys& keys){
			auto begin = map.begin();
			auto end   = map.end();
			for(auto it = begin; it != end; ++it){
				auto t = hana::make_tuple(it->first);
				SubTreeValues<KeysLeft-1>()(it->second, values, keys, t);
			}

		}

	};

	template <>
	struct SubTreeValues <0> {

		template <typename T_Value, typename T_ValuesCT, typename T_KeysCT, typename T_Tuple>
		void operator()(T_Value &value, T_ValuesCT& values, T_KeysCT& keys, T_Tuple &tuple){
			keys.push_back(tuple);
			values.push_back(value);
		}

	};

	/**
     * @brief Traverse through a map of map of ... and 
     *        return the value of the last key.
     *
     */
	auto at = [] (auto &map,  const auto &key) -> auto& {
		return map[key];
	};


	template <typename T_Map, typename T_Keys>
	auto& traverse(T_Map& map, T_Keys const& keys) {
		return hana::fold_left(keys, map, at);
	}



	/**
     * @brief A map with multiple keys, implemented by cascading several 
     *        std::maps.
     *
     * The MultiKeyMap has the goal to provide the look and feel of
     * a usual std::map by the at and () methods. Furthermore, it
     * provides the possibility to retrieve the values of subtrees
     * by specifying not all keys by the values method.
     *
     */
	template <typename T_Value, typename... T_Keys>
	struct MultiKeyMap {

		typename MultiKeyMapType<T_Keys..., T_Value>::type multiKeyMap;

		MultiKeyMap() {

		}

		T_Value& operator()(const T_Keys... keys){
			return traverse(multiKeyMap, hana::make_tuple(keys...));
		}

		/***************************************************************************
             * at
             ***************************************************************************/
		template <typename T_KeysTpl>
		T_Value& at(const T_KeysTpl keysTuple){
			return atImpl(keysTuple);
		}


		T_Value& at(const T_Keys... keys){
			return atImpl(hana::make_tuple(keys...));

		}

		template <typename T_KeysTpl>
		T_Value& atImpl(const T_KeysTpl keysTuple){
			auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
			auto firstKeys     = hana::take_front_c<firstKeysSize>(keysTuple);
			auto lastKey       = hana::back(keysTuple);


			return traverse(multiKeyMap, firstKeys).at(lastKey);
		}

		/***************************************************************************
             * test
             ***************************************************************************/
		bool test(const T_Keys ...keys) {
			return testImpl(hana::make_tuple(keys...));
		}

		template <typename T_KeysTpl>
		bool test(const T_KeysTpl keysTuple) {
			return testImpl(keysTuple);
		}


		template <typename T_KeysTpl>
		bool testImpl(const T_KeysTpl keysTuple) {
			auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
			auto firstKeys     = hana::take_front_c<firstKeysSize>(keysTuple);
			auto lastKey       = hana::back(keysTuple);

			auto &subMap = traverse(multiKeyMap, firstKeys);
			auto it  = subMap.find(lastKey);
			auto end = subMap.end();

			return (it == end) ? false : true;

		}

		/***************************************************************************
             * erase
             ***************************************************************************/
		bool erase(const T_Keys ...keys) {
			return eraseImpl(hana::make_tuple(keys...));
		}

		template <typename T_KeysTPL>
		bool erase(const T_KeysTPL keysTuple) {
			return eraseImpl(keysTuple);
		}


		template <typename T_KeysTpl>
		bool eraseImpl(const T_KeysTpl keysTuple) {
			auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
			auto firstKeys     = hana::take_front_c<firstKeysSize>(keysTuple);
			auto lastKey       = hana::back(keysTuple);


			auto &lastMap = traverse(multiKeyMap, firstKeys);
			auto it  = lastMap.find(lastKey);
			auto end = lastMap.end();

			if(it == end) return false;
			else {
				lastMap.erase(it);
				return true;
			}

		}


		/***************************************************************************
             * values
             ***************************************************************************/
		template <typename T_ValuesCT, typename T_KeysCT>
		void values(T_Value &values, T_KeysCT& keys){
			constexpr size_t keysTupleSize = std::tuple_size<std::tuple<T_Keys...>>::value;
			SubTreeValues<keysTupleSize>()(multiKeyMap, values, keys);
		}

		template <typename T_ValuesCT, typename T_KeysCT, typename ...T_Sub_Keys>
		void values(T_ValuesCT &values, T_KeysCT& keys, const T_Sub_Keys... subKeys){
			constexpr size_t subKeysTupleSize = std::tuple_size<std::tuple<T_Sub_Keys...>>::value;
			constexpr size_t keysTupleSize    = std::tuple_size<std::tuple<T_Keys...>>::value;
			constexpr size_t subTreeSize      = keysTupleSize - subKeysTupleSize;
			auto tuple = hana::make_tuple(subKeys...);
			SubTreeValues<subTreeSize>()(traverse(multiKeyMap, tuple), values, keys, tuple);

		}

	};


	template <typename T_Value, typename... T_Keys>
	struct MessageBox {

		MessageBox(size_t const maxBufferSize) :
				maxBufferSize(maxBufferSize),
				bufferSize(0){
		}

		size_t maxBufferSize;
		std::atomic<size_t> bufferSize;
		std::mutex access;
		std::mutex writeNotify;
		std::mutex readNotify;
		std::condition_variable writeCondition;
		std::condition_variable readCondition;

		using Queue = std::queue<T_Value>;
		MultiKeyMap<Queue, T_Keys...> multiKeyMap;

		auto enqueue(T_Value&& value, const T_Keys... keys) -> void {
			{
				while(maxBufferSize < bufferSize + value.size()){
					std::unique_lock<std::mutex> notifyLock(readNotify);
					readCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
				}

				bufferSize += value.size();
				//std::cout << bufferSize << std::endl;
				//std::cout << "Try to enqueue message." << std::endl;
				std::lock_guard<std::mutex> accessLock(access);
				multiKeyMap(keys...).push(std::forward<T_Value>(value));
			}
			//std::cout << "notify on condition variable." << std::endl;
			writeCondition.notify_one();
		}

		auto waitDequeue(const T_Keys... keys) -> T_Value {

			bool test = false;

			{
				std::lock_guard<std::mutex> accessLock(access);
				test = !multiKeyMap.test(keys...);
			}

			while(test){
				std::unique_lock<std::mutex> notifyLock(writeNotify);
				//std::cout << "wait for multikeymap enqueue." << std::endl;
				writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
				{
					std::lock_guard<std::mutex> accessLock(access);
					test = !multiKeyMap.test(keys...);
				}

			}

			while(multiKeyMap.at(keys...).empty()){
				std::unique_lock<std::mutex> notifyLock(writeNotify);
				//std::cout << "wait for queue to be filled." << std::endl;
				writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));

			}

			{
				std::lock_guard<std::mutex> accessLock(access);
				//std::cout << "get message from queue" << std::endl;
				T_Value value = std::move(multiKeyMap.at(keys...).front());
				multiKeyMap.at(keys...).pop();
				bufferSize -= value.size();
				readCondition.notify_one();
				return value;
			}

		}

		template <typename... SubKeys>
		auto waitDequeue(hana::tuple<T_Keys...> &allKeys, const SubKeys... someKeys) -> T_Value {

			while(true){

				std::vector<std::reference_wrapper<Queue>> values;
				std::vector<hana::tuple<T_Keys...> > keysList;
				while(values.empty()){
					std::unique_lock<std::mutex> notifyLock(writeNotify);
					{
						std::lock_guard<std::mutex> accessLock(access);
						multiKeyMap.values(values, keysList, someKeys...);

					}
					writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
				}

				{
					std::lock_guard<std::mutex> accessLock(access);
					for(auto &keys : keysList ){
						auto& queue = multiKeyMap.at(keys);
						if(!queue.empty()) {
							allKeys = keys;
							T_Value value = std::move(queue.front());
							queue.pop();
							bufferSize -= value.size();
							readCondition.notify_one();
							return value;
						}

					}
				}
				std::unique_lock<std::mutex> notifyLock(writeNotify);
				writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));

			}

		}

		auto tryDequeue(bool &result, const T_Keys... keys) -> T_Value {

			{
				std::lock_guard<std::mutex> accessLock(access);
				if(!multiKeyMap.test(keys...)){
					result = false;
					return T_Value();
				}
			}

			if(multiKeyMap.at(keys...).empty()){
				result = false;
				return T_Value();
			}

			// //while(test){
			//     std::unique_lock<std::mutex> notifyLock(writeNotify);
			//     //std::cout << "wait for multikeymap enqueue." << std::endl;
			//     writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
			//     {
			//         std::lock_guard<std::mutex> accessLock(access);
			//         test = !multiKeyMap.test(keys...);
			//     }
			//     //}

			{
				std::lock_guard<std::mutex> accessLock(access);
				//std::cout << "get message from queue" << std::endl;
				T_Value value = std::move(multiKeyMap.at(keys...).front());
				multiKeyMap.at(keys...).pop();
				bufferSize -= value.size();
				readCondition.notify_one();
				result = true;
				return value;
			}

		}

	};

} /* utils */
