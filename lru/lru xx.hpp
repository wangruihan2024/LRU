#ifndef SJTU_LRU_HPP
#define SJTU_LRU_HPP

#include "utility.hpp"
#include "exceptions.hpp"
#include "class-integer.hpp"
#include "class-matrix.hpp"
#include <vector>

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
template<class T>
class double_list{
private:
	struct Node{
		T data;
		Node *pre, *nxt;
		Node(const T& val): data(val), pre(nullptr), nxt(nullptr){};
	};
	Node *head, *tail;
	size_t s;

public:
	double_list(): head(nullptr), tail(nullptr), s(0) {}
	double_list(const double_list<T> &other){
		clear();
		head = tail = nullptr;
        Node *cur = other.head;
        while (cur) {
            insert_tail(cur->data);
            cur = cur->nxt;
        }
		s = other.s;
	}
	~double_list(){
		clear();
	}

	class iterator{
	public:
		Node *cur;
		iterator(){}
		iterator(Node *node): cur(node) {}
		iterator(const iterator &t){
			cur = t.cur;
		}
		~iterator(){}
 
		iterator operator++(int) {
			iterator old = *this;
			if(cur) cur = cur->nxt;
			else throw std::out_of_range("invalid");
			return old;
		}
		iterator &operator++() {
			if(cur) cur = cur->nxt;
			else throw std::out_of_range("invalid");
			return *this;
		}

		iterator operator--(int) {
			iterator old = *this;
			if((!cur)||(!(cur->pre))) throw std::out_of_range("invalid");
			else cur = cur->pre;
			return old;
		}
		iterator &operator--() {
			if((!cur)||(!(cur->pre))) throw std::out_of_range("invalid");
			else cur = cur->pre;
			return *this;
		}

		T &operator*() const {
			if(!cur) throw std::out_of_range("invalid");
			return cur->data;
		}

		T *operator->() const noexcept {
			if(!cur) return nullptr;
			return &(cur->data);
		}
		bool operator==(const iterator &rhs) const {
			return cur == rhs.cur;
    	}
		bool operator!=(const iterator &rhs) const {
			return cur != rhs.cur;
		}
	};

	iterator begin() const{
		return iterator(head);
	}
	iterator get_tail() const{
		return iterator(tail);
	}
	iterator end() const{
		return iterator(nullptr);
	}

	iterator erase(iterator pos){
		if(!pos.cur) return end();
		Node *tmp = pos.cur;
		if(tmp->pre) tmp->pre->nxt = tmp->nxt;
		else head = tmp->nxt;
		if(tmp->nxt) tmp->nxt->pre = tmp->pre;
		else tail = tmp->pre;
		iterator next = iterator(tmp->nxt);
		delete tmp;
		s--;
		return next;
	}

	void insert_head(const T &val){
		Node *newNode = new Node(val);
        if (!head){
            head = tail = newNode;
        }
		else{
            head->pre = newNode;
            newNode->nxt = head;
            head = newNode;
        }
		s++;
	}
	void insert_tail(const T &val){
		Node *newNode = new Node(val);
        if (!tail){
            head = tail = newNode;
        }
		else{
            tail->nxt = newNode;
            newNode->pre = tail;
            tail = newNode;
        }
		s++;
	}
	void delete_head(){
		if (!head) return;
        Node *tmp = head;
        if (head == tail){
            head = tail = nullptr;
        }
		else{
            head = head->nxt;
            head->pre = nullptr;
        }
        delete tmp;
		s--;
	}
	void delete_tail(){
		if (!tail) return;
        Node *tmp = tail;
        if (head == tail){
            head = tail = nullptr;
        }
		else{
            tail = tail->pre;
            tail->nxt = nullptr;
        }
        delete tmp;
		s--;
	}

	bool empty() const {
		return !s;
	}
	size_t size() const {
		return s;
	}
	void clear() {
		Node *cur = head;
		while(cur) {
            Node *tmp = cur;
            cur = cur->nxt;
            delete tmp;
        }
		s=0;
		head = tail = nullptr;
	}
};

template < class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>>
class hashmap{
public:
	using value_type = pair<Key, T>;
private:
	static const size_t INIT_SIZE = 16;
    std::vector<std::vector<value_type>> buckets;
    size_t ele_cnt;
    Hash hasher;
    Equal equaler;
public:	
	hashmap(): ele_cnt(0), buckets(INIT_SIZE) {}
	hashmap(const hashmap &other): ele_cnt(other.ele_cnt), buckets(other.buckets), hasher(other.hasher), equaler(other.equaler) {}
	~hashmap(){}

	hashmap & operator=(const hashmap &other){
		if (this != &other) {
            buckets = other.buckets;
            ele_cnt = other.ele_cnt;
            hasher = other.hasher;
            equaler = other.equaler;
        }
        return *this;
	}

	class iterator{
	private:
        std::vector<std::vector<value_type>>* buckets_ptr;
        size_t bucket_idx;
        size_t element_idx;
		bool is_end;
	public:
		iterator(std::vector<std::vector<value_type>>* ptr, size_t bidx, size_t eidx, bool is_end = false)
            : buckets_ptr(ptr), bucket_idx(bidx), element_idx(eidx), is_end(is_end) {}
		iterator(const iterator &t): buckets_ptr(t.buckets_ptr), bucket_idx(t.bucket_idx), element_idx(t.element_idx), is_end(t.is_end) {}
		~iterator(){}

		value_type &operator*() const {
			if(this->is_end) throw std::out_of_range("invalid");
			return (*buckets_ptr)[bucket_idx][element_idx];
		}
		value_type *operator->() const noexcept {
			return &((*buckets_ptr)[bucket_idx][element_idx]);
		}
		bool operator==(const iterator &rhs) const {
			if(is_end && rhs.is_end) return true;
			return (buckets_ptr==rhs.buckets_ptr)&&(bucket_idx==rhs.bucket_idx)&&(element_idx==rhs.element_idx);
    	}
		bool operator!=(const iterator &rhs) const {
			return !(*this==rhs);
		}
	};

	void clear(){
		for (int i=0;i<buckets.size();i++) {
            buckets[i].clear();
        }
        ele_cnt = 0;
	}

	void expand(){
		size_t new_size = buckets.size() * 2;
        std::vector<std::vector<value_type>> new_buckets(new_size);
        for (size_t i = 0; i < buckets.size(); i++) {
            for (const auto& item : buckets[i]) {
                size_t hash_val = hasher(item.first) % new_size;
                new_buckets[hash_val].push_back(item);
            }
        }
        buckets = std::move(new_buckets);
	}

	iterator end() {
		return iterator(&buckets, buckets.size(), 0, true);
	}
	iterator find(const Key &key) {
		size_t hash_val = hasher(key)%buckets.size();
		for(int i=0;i<buckets[hash_val].size();i++)
			if(equaler(buckets[hash_val][i].first, key))
				return iterator(&buckets, hash_val, i, false);
		return end();
	}
    pair<iterator, bool> insert(const value_type& value_pair) {
        if (ele_cnt >= buckets.size() * 2) expand();
        size_t hash_val = hasher(value_pair.first) % buckets.size();
        for (size_t i = 0; i < buckets[hash_val].size(); i++)
            if (equaler(buckets[hash_val][i].first, value_pair.first)){
				buckets[hash_val][i].second=value_pair.second;
                return pair(iterator(&buckets, hash_val, i), false);
			}
        buckets[hash_val].push_back(value_pair);
        ++ele_cnt;
        return pair(iterator(&buckets, hash_val, buckets[hash_val].size() - 1, false), true);
    }
	bool remove(const Key &key){
		size_t hash_val = hasher(key)%buckets.size();
		for(int i=0;i<buckets[hash_val].size();i++)
			if(equaler(buckets[hash_val][i].first, key)){
				buckets[hash_val].erase(buckets[hash_val].begin()+i);
				ele_cnt--;
				return true;
			}
		return false;
	}
};

template<class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>>
class linked_hashmap {
public:
	typedef pair<Key, T> value_type;
private:
	double_list<value_type> kkey;
	hashmap<Key, typename double_list<value_type>::iterator, Hash, Equal> mp;
public:
	class const_iterator;
	class iterator{
	public:
		typename double_list<value_type>::iterator it;
		iterator(){}
		iterator(typename double_list<value_type>::iterator it) : it(it) {}
		iterator(const iterator &other): it(other.it) {}
		~iterator(){}

		iterator operator++(int) {
			iterator old=*this;
			it++;
			return old;
		} // iter++
		iterator &operator++() {
			it++;
			return *this;
		}
		iterator operator--(int) {
			iterator old=*this;
			it--;
			return old;
		} // throw
		iterator &operator--() {
			it--;
			return *this;
		}
		value_type &operator*() const {
			return *it;
		} // throw
		value_type *operator->() const noexcept {
			return &(*it);
		}

		bool operator==(const iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const iterator &rhs) const {return it!=rhs.it;}
		bool operator==(const const_iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const const_iterator &rhs) const {return it!=rhs.it;}
	};
 
	class const_iterator {
	public:
		typename double_list<value_type>::iterator it;
		const_iterator() {}
		const_iterator(typename double_list<value_type>::iterator it) : it(it) {}
		const_iterator(const iterator &other): it(other.it){}

		const_iterator operator++(int) {
			const_iterator old=*this;
			it++;
			return old;
		}
		const_iterator &operator++() {
			it++;
			return *this;
		}
		const_iterator operator--(int) {
			const_iterator old=*this;
			it--;
			return old;
		}
		const_iterator &operator--() {
			it--;
			return *this;
		}
		const value_type &operator*() const {
			return *it;
		}
		const value_type *operator->() const noexcept {
			return &(*it);
		} // throw

		bool operator==(const iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const iterator &rhs) const {return it!=rhs.it;}
		bool operator==(const const_iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const const_iterator &rhs) const {return it!=rhs.it;}
	};
 
	linked_hashmap() {}
	linked_hashmap(const linked_hashmap &other): mp(other.mp){
		for(auto it=other.kkey.begin();it!=other.kkey.end();it++)
			kkey.insert_tail(*it);
	}
	~linked_hashmap() {
		kkey.clear();
		mp.clear();
	}
	linked_hashmap & operator=(const linked_hashmap &other) {
		if(this != &other){
			mp = other.mp;
			kkey.clear();
			for(auto it=other.kkey.begin();it!=other.kkey.end();it++)
				kkey.insert_tail(*it);
		}
		return *this;
	}

	T & at(const Key &key) {
        auto it = mp.find(key);
        if (it == mp.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}
	const T & at(const Key &key) const {
    	auto it = mp.find(key);
        if (it == mp.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}
	T & operator[](const Key &key) {
        auto it = mp.find(key);
        if (it == mp.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}
	const T & operator[](const Key &key) const {
        auto it = mp.find(key);
        if (it == mp.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}

	iterator begin() {
		return iterator(kkey.begin());
	}
	const_iterator cbegin() const {
		return const_iterator(kkey.begin());
	}
	iterator end() {
		return iterator(kkey.end());
	}
	const_iterator cend() const {
		return const_iterator(kkey.end());
	}
	bool empty() const {
		return kkey.empty();
	}
    void clear(){
		mp.clear();
		kkey.clear();
	}
	size_t size() const {
		return kkey.size();
	}

	pair<iterator, bool> insert(const value_type &value) {
		auto it = mp.find(value.first);
		if(it!=mp.end()){
			kkey.erase(it->second);
			kkey.insert_tail(value);
			auto new_pos = kkey.get_tail();
        	it->second = new_pos;
    	    return {iterator(new_pos), false};
		}
		else{
			kkey.insert_tail(value);
			auto lit = kkey.get_tail();
        	mp.insert({value.first, lit});
        	return {iterator(lit), true};
		}
	}

	void remove(iterator pos) {
		if(pos==end()) throw std::out_of_range("Key not found");
		mp.remove(pos->first);
		kkey.erase(pos.it);
	}
	size_t count(const Key &key) {
		auto it = mp.find(key);
		if(it==mp.end())
			return 0;
		return 1;
	}
	iterator find(const Key &key) {
		auto it = mp.find(key);
		if(it==mp.end()) return end();
		return iterator(it->second);
	}
};

class lru{
    using lmap = sjtu::linked_hashmap<Integer,Matrix<int>,Hash,Equal>;
    using value_type = sjtu::pair<const Integer, Matrix<int> >;

	size_t capacity;
	lmap mp;
public:
    lru(size_t size): capacity(size){}
    ~lru(){}

    void save(const value_type &v) {
		if (mp.count(v.first)) {
    		mp.remove(mp.find(v.first));
    	}
    	if (mp.size() >= capacity) {
        	auto oldest = mp.begin();
        	mp.remove(oldest);
    	}
    	mp.insert(v);
    }
    Matrix<int>* get(const Integer &v) {
		auto it=mp.find(v);
		if(it!=mp.end()) {
			auto value = it->second;
			mp.remove(it);
			mp.insert({v,value});
			return &(mp.find(v)->second);
		}
		return nullptr;
    } 
    void print(){
		for (auto it = mp.begin(); it != mp.end(); ++it) {
        	std::cout << it->first.val << " " << it->second << std::endl;
    	}
    }
};
};

#endif