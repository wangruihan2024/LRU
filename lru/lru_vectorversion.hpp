#ifndef SJTU_LRU_HPP
#define SJTU_LRU_HPP

#include "utility.hpp"
#include "exceptions.hpp"
#include "class-integer.hpp"
#include "class-matrix.hpp"
class Hash {
public:
	unsigned int operator () (Integer lhs) const {
		int val = lhs.val;
		return std::hash<int>()(val);
	}
};
class Equal {
public:
	bool operator () (const Integer &lhs, const Integer &rhs) const {
		return lhs.val == rhs.val;
	}
};

namespace sjtu {
template<class T> class double_list{
public:
	struct Node{
		T data;
		Node *pre, *next;
		Node(const T& data):data(data), pre(nullptr), next(nullptr){}
	};
	Node *head, *tail;
	size_t s;
	// --------------------------
	double_list():head(nullptr), tail(nullptr), s(0){
	}
	double_list(const double_list<T> &other){
		clear();
		head = nullptr;
		tail = nullptr;
		Node *tmp = other.head;
		while(tmp) {
			insert_tail(tmp->data);
			tmp = tmp->next;
		}
		s = other.s;
	}
	~double_list(){
		clear();
	}

	class iterator{
	public:
		Node *current;
		// --------------------------
		iterator(){}
		iterator(Node* t) : current(t){}
		iterator(const iterator &t){
			current = t.current;
		}
		~iterator(){}
        /**
		 * iter++
		 */
		iterator operator++(int) {
			iterator old = *this;
			if(current)
				current = current->next;
			else
				throw std::out_of_range("invalid++");
			return old;
		}
        /**
		 * ++iter
		 */
		iterator &operator++() {
			if(current)
				current = current -> next;
			else
				throw std::out_of_range("++invalid");
			return *this;	
		}
        /**
		 * iter--
		 */
		iterator operator--(int) {
			iterator old = *this;
			if(current)
				current = current->pre;
			else
				throw std::out_of_range("invalid--");
			return old;
		}
        /**
		 * --iter
		 */
		iterator &operator--() {
			if(!current || !(current->pre))
				throw std::out_of_range("--invalid");
			else {
				current = current->pre;
				return *this;
			}	
			
		}
		/**
		 * if the iter didn't point to a value
		 * throw " invalid"
		*/
		T &operator*() const {
			if(!current)
				throw std::out_of_range("invalid");
			return this->current->data;
		}
        /**
         * other operation
        */
		T *operator->() const noexcept {
			if(!current)
				throw std::out_of_range("invalid");
			return &(current->data);
		}
		bool operator==(const iterator &rhs) const {
			return current == rhs.current;
		}
		bool operator!=(const iterator &rhs) const {
			return current != rhs.current;
		}
	};
	/**
	 * return an iterator to the beginning
	 */
	iterator begin() const{
		return iterator(head);
	}
	/**
	 * return an iterator to the ending
	 * in fact, it returns the iterator point to nothing,
	 * just after the last element.
	 */
	iterator get_tail() const{
		return iterator(tail);
	}
	iterator end() const{
		return iterator(nullptr);
	}
	/**
	 * if the iter didn't point to anything, do nothing,
	 * otherwise, delete the element pointed by the iter
	 * and return the iterator point at the same "index"
	 * e.g.
	 * 	if the origin iterator point at the 2nd element
	 * 	the returned iterator also point at the
	 *  2nd element of the list after the operation
	 *  or nothing if the list after the operation
	 *  don't contain 2nd elememt.
	*/
	iterator erase(iterator pos){
		if(!pos.current)
			return end();
		Node *tmp = pos.current;
		if(tmp->pre)
			tmp->pre->next = tmp->next;
		else
			head = tmp->next;
		if(tmp->next)
			tmp->next->pre = tmp->pre;
		else 
			tail = tmp->pre;
		iterator tmp_next = iterator(tmp->next);
		delete tmp;
		s--;
		return tmp_next;
	}

	/**
	 * the following are operations of double list
	*/
	void insert_head(const T &val){
		Node *new_node = new Node(val);
		if(!head)
			head = tail = new_node;
		else {
			head->pre = new_node;
			new_node->next = head;
			head = new_node;
		}
		s++;
	}
	void insert_tail(const T &val){
		Node *new_node = new Node(val);
		if(!tail)
			head = tail = new_node;
		else {
			tail->next = new_node;
			new_node->pre = tail;
			tail = new_node;
		}
		s++;
	}
	void delete_head(){
		if(!head)
			return;
		if(s == 1)
			head = tail = nullptr;
		else {
			Node* tmp = head;
			head = head->next;
			head->pre = nullptr;
			s--;
			delete tmp;
		}
	}
	void delete_tail(){
		if(!tail)
			return;
		if(s == 1)
			head = tail = nullptr;
		else {
			Node *tmp = tail;
			tail = tail->pre;
			tail->next = nullptr;
			s--;
			delete tmp;
		}
	}
	bool empty() const{
		return s == 0;
	}
	void clear() {
		Node* tmp = head;
		while(tmp) {
			Node* tmp_aft = tmp;
			tmp = tmp->next;
			delete tmp_aft;
		}
		head = nullptr;
		tail = nullptr;
		s = 0;
	}
	void print() const {
		Node *tmp = head;
		while(tmp) {
			std::cout << tmp->data << " ";
			tmp = tmp->next;
		}
		std::cout << std::endl;
	}
	size_t size() const { return s; }
};

static const int initial_size = 10;
template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class hashmap{
public:
	using value_type = pair<Key, T>;
	std::vector<std::vector<value_type>> bucket;
	size_t size; // record the number of elements
	Hash hash_function;
	Equal equal_function; 
// --------------------------
	hashmap() {
		size = 0;
		bucket = std::vector<std::vector<value_type>>(initial_size);
	}
	hashmap(const hashmap &other){
		size = other.size;
		equal_function = other.equal_function;
		hash_function = other.hash_function;
		bucket = other.bucket;
	}
	~hashmap() { clear(); }
	hashmap & operator=(const hashmap &other){
		if(this != &other) {
			clear();
			size = other.size;
			equal_function = other.equal_function;
			hash_function = other.hash_function;
			bucket = other.bucket;
		}
		return *this;
	}
	class iterator{
	public:
		std::vector<std::vector<value_type>>* bucket_ptr; //桶指针
		size_t bucket_iter;
		size_t size_iter;
		bool end;
		// --------------------------
		iterator():bucket_ptr(nullptr), bucket_iter(0), size_iter(0), end(true){}
		iterator(const iterator &t){
			bucket_ptr = t.bucket_ptr;
			bucket_iter = t.bucket_iter;
			size_iter = t.size_iter;
			end = t.end;
		}
		iterator(std::vector<std::vector<value_type>>* bucket_ptr, size_t bucket_iter, size_t size_iter, bool end)
            : bucket_ptr(bucket_ptr), bucket_iter(bucket_iter), size_iter(size_iter), end(end){}
		~iterator() {}

		/**
		 * if point to nothing
		 * throw 
		*/
		value_type &operator*() const {
			if(this->end)
				throw std::out_of_range("point to nothing");
			return (*bucket_ptr)[bucket_iter][size_iter];
		}

        /**
		 * other operation
		*/

		value_type *operator->() const noexcept {
			if(this->end)
				throw std::out_of_range("point to nothing");
			return &((*bucket_ptr)[bucket_iter][size_iter]);
		}
		bool operator==(const iterator &rhs) const {
			if(end == rhs.end)
				return true;
			return bucket_ptr == rhs.bucket_ptr && bucket_iter == rhs.bucket_iter && size_iter == rhs.size_iter;
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
	};

	void clear(){
		for(size_t i = 0; i < bucket.size(); i++) {
			bucket[i].clear();
		}
		size = 0;
	}
	/**
	 * you need to expand the hashmap dynamically
	*/
	void expand(){
		size_t new_size = 2 * bucket.size();
		std::vector<std::vector<value_type>> new_bucket(new_size);
		for (size_t i = 0; i < bucket.size(); i++) {
			for (const auto& item : bucket[i]) {
                size_t hash_val = hash_function(item.first) % new_size;
                new_bucket[hash_val].push_back(item);
            }
		}
		bucket = std::move(new_bucket);
	}

    /**
     * the iterator point at nothing
    */
	iterator end() {
		return iterator(&bucket, bucket.size(), 0, true);
	}
	/**
	 * find, return a pointer point to the value
	 * not find, return the end (point to nothing)
	*/
	iterator find(const Key &key) {
		if(bucket.empty())
			return end();
		size_t index = hash_function(key) % bucket.size();
		for (size_t i = 0; i < bucket[index].size(); i++) {
			if (equal_function(bucket[index][i].first, key)) {
				return iterator(&bucket, index, i, false);
			}
		}
        return end();
	}
	/**
	 * already have a value_pair with the same key
	 * -> just update the value, return false
	 * not find a value_pair with the same key
	 * -> insert the value_pair, return true
	*/
	sjtu::pair<iterator,bool> insert(const value_type &value_pair) {
		if (size >= bucket.size() * 2) 
			expand();
        size_t index = hash_function(value_pair.first) % bucket.size();
        for (size_t i = 0; i < bucket[index].size(); i++)
            if (equal_function(bucket[index][i].first, value_pair.first)){
				bucket[index][i].second = value_pair.second;
                return sjtu::pair(iterator(&bucket, index, i, false), false);
			}
        bucket[index].push_back(value_pair);
		size++;
		return pair(iterator(&bucket, index, bucket[index].size() - 1, false), true);
	}
	/**
	 * the value_pair exists, remove and return true
	 * otherwise, return false
	*/
	bool remove(const Key &key){
		size_t index = hash_function(key) % bucket.size();
		for(int i = 0 ; i < bucket[index].size() ; i++)
			if(equal_function(bucket[index][i].first, key)){
				bucket[index].erase(bucket[index].begin() + i);
				size--;
				return true;
			}
		return false;
	}
};

template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class linked_hashmap :public hashmap<Key,T,Hash,Equal>{
public:
	typedef pair< Key, T> value_type;
	double_list<value_type> order;
	hashmap<Key, typename double_list<value_type>::iterator, Hash, Equal> maps;
	// --------------------------
	class const_iterator;
	class iterator{
	public:
		typename double_list<value_type>::iterator it;
		// --------------------------
		iterator(){}
		iterator(typename double_list<value_type>::iterator it):it(it) {}
		iterator(const iterator &other):it(other.it){}
		~iterator(){}
		/**
		 * iter++
		 */
		iterator operator++(int) {
			iterator old = *this;
			it++;
			return old;
		}
		/**
		 * ++iter
		 */
		iterator &operator++() {
			it++;
			return *this;
		}
		/**
		 * iter--
		 */
		iterator operator--(int) {
			iterator old = *this;
			it--;
			return old;
		}
		/**
		 * --iter
		 */
		iterator &operator--() {
			it--;
			return *this;
		}

		/**
		 * if the iter didn't point to a value
		 * throw "star invalid"
		*/
		value_type &operator*() const {
			return *it;
		}
		value_type *operator->() const noexcept {
			return &(*it);
		}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {return it == rhs.it;}
		bool operator!=(const iterator &rhs) const {return it != rhs.it;}
		bool operator==(const const_iterator &rhs) const {return it == rhs.it;}
		bool operator!=(const const_iterator &rhs) const {return it != rhs.it;}
	};
 
	class const_iterator {
		public:
			typename double_list<value_type>::iterator it;
    // --------------------------   
		const_iterator() {}
		const_iterator(typename double_list<value_type>::iterator it):it(it) {}
		const_iterator(const iterator &other):it(other.it){}
		/**
		 * iter++
		 */
		const_iterator operator++(int) {
			const_iterator old = *this;
			it++;
			return old;
		}
		/**
		 * ++iter
		 */
		const_iterator &operator++() {
			it++;
			return *this;
		}
		/**
		 * iter--
		 */
		const_iterator operator--(int) {
			const_iterator old = *this;
			it--;
			return old;
		}
		/**
		 * --iter
		 */
		const_iterator &operator--() {
			it--;
			return *this;
		}

		/**
		 * if the iter didn't point to a value
		 * throw 
		*/
		const value_type &operator*() const {
			return *it;
		}
		const value_type *operator->() const noexcept {
			return &(*it);
		}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const { return it == rhs.it; }
		bool operator!=(const iterator &rhs) const { return it != rhs.it; }
		bool operator==(const const_iterator &rhs) const { return it == rhs.it; }
		bool operator!=(const const_iterator &rhs) const { return it != rhs.it; }
	};
 
	linked_hashmap() {}
	linked_hashmap(const linked_hashmap &other):maps(other.maps){
		for(auto it = other.order.begin(); it != other.order.end(); it++)
			order.insert_tail(*it);
	}
	~linked_hashmap() {
		order.clear();
		maps.clear();
	}
	linked_hashmap & operator=(const linked_hashmap &other) {
		if(this != &other) {
			maps = other.maps;
			order.clear();
			for(auto it = other.order.begin(); it != other.order.end(); it++) 
				order.insert_tail(*it);
		}
		return *this;
	}

 	/**
	 * return the value connected with the Key(O(1))
	 * if the key not found, throw 
	*/
	T & at(const Key &key) {
		auto it = maps.find(key);
		if(it == maps.end())
			throw std::out_of_range("key is not valid");
		return it->second->second;
	}
	const T & at(const Key &key) const {
		auto it = maps.find(key);
		if(it == maps.end())
			throw std::out_of_range("key is not valid");
		return it->second->second;
	}
	T & operator[](const Key &key) {
		auto it = maps.find(key);
		if(it == maps.end())
			throw std::out_of_range("key is not valid");
		return it->second->second;
	}
	const T & operator[](const Key &key) const {
		auto it = maps.find(key);
		if(it == maps.end())
			throw std::out_of_range("key is not valid");
		return it->second->second;
	}

	/**
	 * return an iterator point to the first 
	 * inserted and existed element
	 */
	iterator begin() {
		return iterator(order.begin());
	}
	const_iterator cbegin() const {
		return const_iterator(order.begin());
	}
    /**
	 * return an iterator after the last inserted element
	 */
	iterator end() {
		return iterator(order.end());
	}
	const_iterator cend() const {
		return const_iterator(order.end());
	}
  	/**
	 * if didn't contain anything, return true, 
	 * otherwise false.
	 */
	bool empty() const {
		return order.empty();
	}

    void clear(){
		order.clear();
		maps.clear();
	}

	size_t size() const {
		return order.size();
	}
 	/**
	 * insert the value_piar
	 * if the key of the value_pair exists in the map
	 * update the value instead of adding a new element，
     * then the order of the element moved from inner of the 
     * list to the head of the list
	 * and return false
	 * if the key of the value_pair doesn't exist in the map
	 * add a new element and return true
	*/
	pair<iterator, bool> insert(const value_type &value) {
		auto it = maps.find(value.first);
		if(it != maps.end()) {
			order.erase(it->second);
			order.insert_tail(value);
			auto new_pos = order.get_tail();
			it->second = new_pos;
			return sjtu::pair(iterator(new_pos), false);
		}else {
			order.insert_tail(value);
			auto new_it = order.get_tail();
			maps.insert(sjtu::pair(value.first, new_it));
			return sjtu::pair(iterator(new_it), true);
		}
	}
 	/**
	 * erase the value_pair pointed by the iterator
	 * if the iterator points to nothing
	 * throw 
	*/
	void remove(iterator pos) {
		if(pos == end())
			throw std::out_of_range("iterator points to nothing");
		maps.remove(pos->first);
		order.erase(pos.it);
	}
	/**
	 * return how many value_pairs consist of key
	 * this should only return 0 or 1
	*/
	size_t count(const Key &key) {
		auto it = maps.find(key);
		if(it == maps.end())
			return 0;
		return 1;
	}
	/**
	 * find the iterator points at the value_pair
	 * which consist of key
	 * if not find, return the iterator 
	 * point at nothing
	*/
	iterator find(const Key &key) {
		auto it = maps.find(key);
		if(it == maps.end())
			return end();
		return iterator(it->second);
	}

};

class lru{
    using lmap = sjtu::linked_hashmap<Integer,Matrix<int>,Hash,Equal>;
    using value_type = sjtu::pair<const Integer, Matrix<int> >;
public:
	size_t capacity;
	mutable lmap maps;
	lru(size_t size):capacity(size){}
    ~lru(){}
    /**
     * save the value_pair in the memory
     * delete something in the memory if necessary
    */
    void save(const value_type &v) {
		if(maps.count(v.first)) {
			maps.remove(maps.find(v.first));
		}
		if(maps.size() >= capacity) {
			auto ol = maps.begin();
			maps.remove(ol);
		}
		maps.insert(v);
	}
    /**
     * return a pointer contain the value
    */
    Matrix<int>* get(const Integer &v) {
		auto it = maps.find(v);
		if(it == maps.end())
			return nullptr;
		auto value = it->second;
		maps.remove(it);
		maps.insert(sjtu::pair(v, value));
		return &(maps.find(v)->second);
	}
    /**
     * just print everything in the memory
     * to debug or test.
     * this operation follows the order, but don't
     * change the order.
    */
    void print(){
		for (auto it = maps.begin(); it != maps.end(); ++it) {
        	std::cout << it->first.val << " " << it->second << std::endl;
		}
    }
};
}

#endif