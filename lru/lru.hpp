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
		head = nullptr;
		tail = nullptr;
		Node *tmp = other.head;
		while(tmp) {
			insert_tail(tmp->data);
			tmp = tmp->next;
		}
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
			if(current)
				current = current->pre;
			else
				throw std::out_of_range("--invalid");
			return *this;
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
	iterator begin(){
		return iterator(head);
	}
	/**
	 * return an iterator to the ending
	 * in fact, it returns the iterator point to nothing,
	 * just after the last element.
	 */
	iterator get_tail() {
		return iterator(tail);
	}
	iterator end(){
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
			return iterator(nullptr);
		Node *tmp = pos.current;
		if(tmp->pre)
			tmp->pre->next = tmp->next;
		else
			head = tmp->next;
		if(tmp->next)
			tmp->next->pre = tmp->pre;
		else {
			tail = tmp->pre;
			return iterator(nullptr);
		}
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
	bool empty(){
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
};

static const int initial_size = 10;
template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class hashmap{
public:
	using value_type = pair<const Key, T>;
	std::vector<double_list<value_type>*> bucket;
	size_t size; // record the number of elements
	Hash hash_function;
	Equal equal_function; 
// --------------------------
	hashmap() {
		size = 0;
		bucket = std::vector<double_list<value_type> *>(initial_size, nullptr);
	}
	hashmap(const hashmap &other){
		size = other.size;
		equal_function = other.equal_function;
		hash_function = other.hash_function;
		bucket.resize(other.bucket.size(), nullptr);
		for(size_t i = 0; i < other.bucket.size(); i++) 
			if(other.bucket[i])
				bucket[i] = new double_list<value_type>(*other.bucket[i]);
	}
	~hashmap(){}
	hashmap & operator=(const hashmap &other){
		if(this != &other) {
			clear();
			size = other.size;
			equal_function = other.equal_function;
			hash_function = other.hash_function;
			for(size_t i = 0; i < other.bucket.size(); i++)
				if(other.bucket[i])
					bucket[i] = new double_list<value_type>(*other.bucket[i]);
		}
		return *this;
	}
	class iterator{
	public:
		double_list<value_type> **bucket_ptr; //桶指针
		double_list<value_type> **end_ptr;
		typename double_list<value_type>::Node *current_node; //链表节点
		// --------------------------
		iterator():bucket_ptr(nullptr), end_ptr(nullptr), current_node(nullptr){}
		iterator(const iterator &t){
			bucket_ptr = t.bucket_ptr;
			end_ptr = t.end_ptr;
			current_node = t.current_node;
		}
		iterator(double_list<value_type>** start, double_list<value_type>** end_ptr, typename double_list<value_type>::Node* node)
            : bucket_ptr(start), end_ptr(end_ptr), current_node(node) {}
		~iterator(){}

        /**
		 * if point to nothing
		 * throw 
		*/
		value_type &operator*() const {
			if(!current_node)
				throw std::out_of_range("point to nothing");
			return current_node->data;
		}

        /**
		 * other operation
		*/

		value_type *operator->() const noexcept {
			if(!current_node)
				throw std::out_of_range("point to nothing");
			return &(current_node->data);
		}
		bool operator==(const iterator &rhs) const {
			return bucket_ptr == rhs.bucket_ptr && current_node == rhs.current_node;
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
	};

	void clear(){
		for(size_t i = 0; i < bucket.size(); i++) {
			if (bucket[i]) {
                delete bucket[i];
                bucket[i] = nullptr;
            }
		}
		size = 0;
	}
	/**
	 * you need to expand the hashmap dynamically
	*/
	void expand(){
		size_t new_size = 2 * bucket.size();
		std::vector<double_list<value_type> *> new_bucket(new_size, nullptr);
		for (size_t i = 0; i < bucket.size(); i++) {
			if(bucket[i] == nullptr)
				continue;
			for (auto it = bucket[i]->begin(); it != bucket[i]->end(); it++) {
				size_t hash_value = hash_function(it->first) % new_size;
				if (!new_bucket[hash_value]) 
					new_bucket[hash_value] = new double_list<value_type>();
				new_bucket[hash_value]->insert_tail(*it);
			}
		}
		clear();
		bucket = std::move(new_bucket);
	}

    /**
     * the iterator point at nothing
    */
	iterator end() const{
		return iterator(const_cast<double_list<value_type> **>(bucket.data()) + bucket.size(),
						const_cast<double_list<value_type> **>(bucket.data()) + bucket.size(),
						nullptr);
	}
	/**
	 * find, return a pointer point to the value
	 * not find, return the end (point to nothing)
	*/
	iterator find(const Key &key)const{
		size_t index = hash_function(key) % bucket.size();
        if (bucket[index]) {
            for (auto it = bucket[index]->begin(); it != bucket[index]->end(); it++) {
                if (equal_function(it->first, key)) {
                    return iterator(const_cast<double_list<value_type>**>(bucket.data()) + index,
					const_cast<double_list<value_type>**>(bucket.data()) + bucket.size(), it.current);
                }
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
	sjtu::pair<iterator,bool> insert(const value_type &value_pair){
		iterator tmp_it = find(value_pair.first);
		if(tmp_it != end()) {
			tmp_it->second = value_pair.second;
			return sjtu::pair(tmp_it, false);
		}else {
			size_t index = hash_function(value_pair.first) % bucket.size();
			if(!bucket[index]) {
				bucket[index] = new double_list<value_type>();
			}
			bucket[index]->insert_tail(value_pair);
			size++;
			typename double_list<value_type>::Node* inserted_node = bucket[index]->tail;
			if(size > bucket.size()) {
				expand();
				index = hash_function(value_pair.first) % bucket.size();
				inserted_node = bucket[index]->tail;
			}	
			return pair(iterator(bucket.data() + index, bucket.data() + bucket.size(), inserted_node), true);
		}
	}
	/**
	 * the value_pair exists, remove and return true
	 * otherwise, return false
	*/
	bool remove(const Key &key){
		iterator tmp_it = find(key);
		if(tmp_it == end() || tmp_it.current_node == nullptr) 
			return false;
		size_t index = hash_function(key) % bucket.size();
		if(bucket[index]) {
			bucket[index]->erase(tmp_it.current_node);
			size--;
			if(bucket[index]->empty()) {
				delete bucket[index];
				bucket[index] = nullptr;
			}
		}
		return true;
	}
};

template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class linked_hashmap :public hashmap<Key,T,Hash,Equal>{
public:
	typedef pair<const Key, T> value_type;
	/**
	 * elements
	 * add whatever you want
	*/
// --------------------------
	class const_iterator;
	class iterator{
	public:
    	/**
         * elements
         * add whatever you want
        */
    // --------------------------
		iterator(){
		}
		iterator(const iterator &other){
		}
		~iterator(){
		}

		/**
		 * iter++
		 */
		iterator operator++(int) {}
		/**
		 * ++iter
		 */
		iterator &operator++() {}
		/**
		 * iter--
		 */
		iterator operator--(int) {}
		/**
		 * --iter
		 */
		iterator &operator--() {}

		/**
		 * if the iter didn't point to a value
		 * throw "star invalid"
		*/
		value_type &operator*() const {
		}
		value_type *operator->() const noexcept {
		}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {}
		bool operator!=(const iterator &rhs) const {}
		bool operator==(const const_iterator &rhs) const {}
		bool operator!=(const const_iterator &rhs) const {}
	};
 
	class const_iterator {
		public:
        	/**
             * elements
             * add whatever you want
            */
    // --------------------------   
		const_iterator() {
		}
		const_iterator(const iterator &other) {
		}

		/**
		 * iter++
		 */
		const_iterator operator++(int) {}
		/**
		 * ++iter
		 */
		const_iterator &operator++() {}
		/**
		 * iter--
		 */
		const_iterator operator--(int) {}
		/**
		 * --iter
		 */
		const_iterator &operator--() {}

		/**
		 * if the iter didn't point to a value
		 * throw 
		*/
		const value_type &operator*() const {
		}
		const value_type *operator->() const noexcept {
		}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {}
		bool operator!=(const iterator &rhs) const {}
		bool operator==(const const_iterator &rhs) const {}
		bool operator!=(const const_iterator &rhs) const {}
	};
 
	linked_hashmap() {
	}
	linked_hashmap(const linked_hashmap &other){
	}
	~linked_hashmap() {
	}
	linked_hashmap & operator=(const linked_hashmap &other) {
	}

 	/**
	 * return the value connected with the Key(O(1))
	 * if the key not found, throw 
	*/
	T & at(const Key &key) {
	}
	const T & at(const Key &key) const {
	}
	T & operator[](const Key &key) {
	}
	const T & operator[](const Key &key) const {
	}

	/**
	 * return an iterator point to the first 
	 * inserted and existed element
	 */
	iterator begin() {
	}
	const_iterator cbegin() const {
	}
    /**
	 * return an iterator after the last inserted element
	 */
	iterator end() {
	}
	const_iterator cend() const {
	}
  	/**
	 * if didn't contain anything, return true, 
	 * otherwise false.
	 */
	bool empty() const {
	}

    void clear(){
	}

	size_t size() const {
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
	}
 	/**
	 * erase the value_pair pointed by the iterator
	 * if the iterator points to nothing
	 * throw 
	*/
	void remove(iterator pos) {
	}
	/**
	 * return how many value_pairs consist of key
	 * this should only return 0 or 1
	*/
	size_t count(const Key &key) const {
	}
	/**
	 * find the iterator points at the value_pair
	 * which consist of key
	 * if not find, return the iterator 
	 * point at nothing
	*/
	iterator find(const Key &key) {
	}

};

class lru{
    using lmap = sjtu::linked_hashmap<Integer,Matrix<int>,Hash,Equal>;
    using value_type = sjtu::pair<const Integer, Matrix<int> >;
public:
    lru(int size){
    }
    ~lru(){
    }
    /**
     * save the value_pair in the memory
     * delete something in the memory if necessary
    */
    void save(const value_type &v) const{
    }
    /**
     * return a pointer contain the value
    */
    Matrix<int>* get(const Integer &v) const{
    }
    /**
     * just print everything in the memory
     * to debug or test.
     * this operation follows the order, but don't
     * change the order.
    */
    void print(){
    }
};
}

#endif