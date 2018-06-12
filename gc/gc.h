#pragma once
#include <cstdio>


class CollectedHeap;

//Any object that inherits from collectable can be created and tracked by the garbage collector.
class Collectable {
public:
	virtual ~Collectable() {}
	virtual string toString() = 0;

	Collectable() :  marked(false), next(NULL), prev(NULL) {}
	
private:
	//Any private fields you add to the Collectable class will be accessible by the CollectedHeap
	//(since it is declared as friend below). You can think of these fields as the header for the object,
	//which will include metadata that is useful for the garbage collector.

	bool marked; // for mark and sweep
	Collectable* next; // for storing a linked list of collectables in the heap
	Collectable* prev;

protected:
	/*
	The mark phase of the garbage collector needs to follow all pointers from the collectable objects, check
	if those objects have been marked, and if they have not, mark them and follow their pointers.
	The simplest way to implement this is to require that collectable objects implement a follow method
	that calls heap.markSuccessors( ) on all collectable objects that this object points to.
	markSuccessors() is the one responsible for checking if the object is marked and marking it.
	*/
	virtual void follow(CollectedHeap& heap) = 0;
	virtual int getSize() = 0;
	friend CollectedHeap;
};

/*
This class keeps track of the garbage collected heap. The class must do all of the following:
	- provide an interface to allocate objects that will be supported by garbage collection.
	- keep track of all currently allocated objects.
	- keep track of the number of currently allocated objects.
	-
*/
class CollectedHeap {

public:

	Collectable* head = NULL; //stores a linked list of all allocated objects
	Collectable* tail = NULL;
	int co = 0;
	bool first = true; // to update head not tail

	int maxmem; //max memory in bytes
	int used;   //used memory in bytes

	~CollectedHeap() {
		//cout << used << " " << maxmem << " " << co << endl;

		Collectable* curr = head;
		while (curr != NULL) {
			Collectable* temp = curr;
			curr = curr->next;
			
			used -= temp->getSize();
			delete temp;
			co -= 1;
		}

		//cout << used << " " << maxmem << " " << co << endl << endl;
	}

	/*
	The constructor should take as an argument the maximum size of the garbage collected heap.
	You get to decide what the units of this value should be. Your VM should compute
	this value based on the -mem parameter passed to it. Keep in mind, however, that
	your VM could be using some extra memory that is not managed by the garbage collector, so
	make sure you account for this.
	*/
	CollectedHeap(int maxmem)
	{
		//maxMem is in MB
		this->maxmem = maxmem*1000*1000;
		this->used = 0;
		this->co = 0;

	}

	bool full() {
		return (used > 0.77*((float) maxmem));
	}


	/*
	return number of objects in the heap.
	This is different from the size of the heap, which should also be tracked
	by the garbage collector.
	*/
	int count()
	{
		return co;
	}

	int usage() {
		return used;
	}

	/*
	This method allocates an object of type T using the default constructor (with no parameters).
	T must be a subclass of Collectable. Before returning the object, it should be registered so that
	it can be deallocated later.
	*/
	template<typename T>
	T* allocate()
	{
		T* object = new T();

		if (first) {
			first = false;
			head = object;
			tail = object;

			this->used += object->getSize();
			co++;
			return object;
		}

		object->prev = this->tail;

		this->tail->next = (Collectable*) object; // add object to linked list of Collectables
		this->tail = (Collectable*) object;

		this->used += object->getSize();

		co++;
		return object;

	}


	/*
	A variant of the method above; this version of allocate can be used to allocate objects whose constructor
	takes one parameter. Useful when allocating Integer or String objects.
	*/
	template<typename T, typename ARG>
	T* allocate(ARG a)
	{
		T* object = new T(a);

		if (first) {
			first = false;
			head = object;
			tail = object;

			this->used += object->getSize();
			co++;
			return object;
		}

		object->prev = this->tail;

		this->tail->next = (Collectable*) object; // add object to linked list of Collectables
		this->tail = (Collectable*) object;

		this->used += object->getSize();

		co++;
		return object;
	}

	/*
	For performance reasons, you may want to implement specialized allocate methods to allocate particular kinds of objects.

	*/


	/*
	This is the method that is called by the follow(...) method of a Collectable object. This
	is how a Collectable object lets the garbage collector know about other Collectable otjects pointed to
	by itself.
	*/
	inline void markSuccessors(Collectable* next)
	{	
		if (next == NULL) { return; }
		if (next->marked) { return; }

		next->marked = true;

		//cout << next->toString() << endl;
		
		next->follow(*this);

	}

	/*
	The gc method should be called by your VM (or by other methods in CollectedHeap)
	whenever the VM decides it is time to reclaim memory. This method
	triggers the mark and sweep process.

	The ITERATOR type should support comparison, assignment and the ++ operator.
	I should also be able to dereference an interator to get a Collectable object.
	This code will take iterators marking the [begin, end) range of the rootset

	*/
	template<typename ITERATOR>
	void gc(ITERATOR begin, ITERATOR end)
	{	

		//cout << "Collecting Garbage... " << c << " " << used << endl;

		for (ITERATOR it = begin; it != end; ++it) {
			markSuccessors(*it);
		}

		Collectable* c = head;
		while( c != NULL ) {
			
			if (c->marked) {
				c->marked = false; //unmark for next GC call
				c = c->next;
				continue;
			}

			if (c->prev != NULL) {c->prev->next = c->next;} //update the linked list
			if (c->next != NULL) {c->next->prev = c->prev;}
			
			Collectable* temp = c;
			c = c->next;

			//cout << temp << " " << temp->toString() <<endl;

			used -= temp->getSize();

			delete temp; temp = NULL;
			
			this->co -= 1;


		}

		//cout << "Now: " << c << " " << used << endl << endl;

	}

};


