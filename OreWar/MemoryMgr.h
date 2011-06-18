#ifndef __MemoryMgr_h_
#define __MemoryMgr_h_

#include <vector>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

/**
 * The MemoryRecord class is used to record details on an allocated block
 * in the memory pool such as starting address and size.
 */
class MemoryRecord
{
private:
	/** A pointer to the first address of the page occupied by this block */
	char * mp_pageStart;

	/** A pointer to the first address in the block */
	char * mp_start;

	/** The number of bytes allocated */
	int m_size;
public:
	/** Constructor */ 
	MemoryRecord(char * start, char * pageStart, int size);

	/** Copy Constructor */
	MemoryRecord(const MemoryRecord& copy);

	/** @return A pointer to the first address of the page occupied by this block */
	char * page() const;

	/** @return A pointer to the first address in the block */
	char * startAddress() const;

	/** @return The number of bytes allocated */
	int size() const;
};


/**
 * The PagedMemoryPool class provides a heap allocated, paged memory
 * services. Memory is batch allocated on construction, and whenever
 * the existing pages are full. The allocation scheme is round robin, 
 * first fit.
 */
class PagedMemoryPool
{
private:
	/** A list of pointers to the dynamically allocated pages */
	std::vector<char *> mp_pages;

	/** 
	 * A list of records of all blocks which have been allocated,
	 * stored in order of ascending starting address.
	 */
	std::vector<std::vector<MemoryRecord>> m_records;

	/** The index of the next page that should be used for allocation */
	int m_nextPage;

	/** The next byte that should be considered for allocation */
	char * mp_nextByte;

	/** The size of pages that should be batch allocated (in bytes) */
	int m_pageSize;

	/** The total number of bytes currently allocated */
	int m_allocatedBytes;

	/** Fetches a new page from memory (should be used when all pages are full) */
	void addPage();

	/** Adds a memory record to the saved list, ensuring ascending order of
	 * address is maintained. */
	void addMemoryRecord(int pageIndex, const MemoryRecord& record);

public:
	/** Constructor */
	PagedMemoryPool(int pageSize, int initialPages);

	/** @return The number of memory pages currently allocated */
	int numPages() const;

	/** @return The index of the page next up for allocation */
	int currentPage() const;

	/** @return The number of bytes currently allocated through this memory pool */
	int allocatedBytes() const;

	/** @return The total number of bytes allocated from the OS (not allocated from
	 * this memory pool */
	int totalBytes() const;

	/**
	 * Creates a copy of the passed object in the paged memory pool,
	 * and returns a pointer to the newly stored copy. Returns NULL
	 * if the size of the passed object exceeds the page size.
	 */
	template <class T>
	inline T * storeObject(const T & object)
	{
		// TODO: Should factor out the actual allocation once a position
		// is found to avoid copypasta code
		int requiredSpace = sizeof T;

		if(requiredSpace > m_pageSize) {
			return NULL;
		}

		int pageIndex = m_nextPage;
		bool firstAllocation = true;

		for(int i = 0; i < mp_pages.size(); i++) {
			int freeSpace = 0;
			char * curByte;
			
			if(firstAllocation) {
				curByte = mp_nextByte;
				firstAllocation = false;
			} else {
				curByte = mp_pages[pageIndex];
			}
			
			bool foundRecord = false;
			for(std::vector<MemoryRecord>::iterator recordIter = m_records[pageIndex].begin(); 
				recordIter != m_records[pageIndex].end();
				recordIter++)
			{
				foundRecord = true;
				int freeSpace = (*recordIter).startAddress() - curByte;
				if(requiredSpace <= freeSpace) {
					T * newT = new (curByte) T(object);
					addMemoryRecord(pageIndex, MemoryRecord(curByte, mp_pages[pageIndex], requiredSpace));
					mp_nextByte = curByte + requiredSpace;
					m_allocatedBytes += requiredSpace;
					return newT;
				} else {
					curByte = (*recordIter).startAddress() + (*recordIter).size();
				}
			}

			if(!foundRecord) {
				// Page must be empty
				T * newT = new (curByte) T(object);
				addMemoryRecord(pageIndex, MemoryRecord(curByte, curByte, requiredSpace));
				mp_nextByte = curByte + requiredSpace;
				m_allocatedBytes += requiredSpace;
				return newT;
			}

			// Check for remaining space at the end of the page
			int byteOffset = (curByte - ((char *)mp_pages[pageIndex]));
			int remainingSpace = (m_pageSize - byteOffset);
			if(remainingSpace > requiredSpace) {
				T * newT = new (curByte) T(object);
				addMemoryRecord(pageIndex, MemoryRecord(curByte, mp_pages[pageIndex], requiredSpace));
				mp_nextByte = curByte + requiredSpace;
				m_allocatedBytes += requiredSpace;
				return newT;
			}

			// Page was full, start on the next one next time
			m_nextPage = (m_nextPage + 1) % mp_pages.size();

			pageIndex = (pageIndex + 1) % mp_pages.size();
		}

		// No room available, add a page
		addPage();
		char * curByte = mp_pages.back();
		T * newT = new (curByte) T(object);
		addMemoryRecord(mp_pages.size() - 1, MemoryRecord(curByte, curByte, requiredSpace));
		mp_nextByte = curByte + requiredSpace;
		m_allocatedBytes += requiredSpace;
		return newT;
	}

	/**
	 * If the passed pointer is found in the record of allocated blocks
	 * the memory is deallocated and available for reuse.
	 * // TODO: Have to find a better way to store the records
	 * @return True if the block was found and deallocated, false if no
	 * record could be found.
	 */
	template <class T>
	inline bool destroyObject(T * object)
	{
		for(std::vector<std::vector<MemoryRecord>>::iterator recordIter = m_records.begin(); 
			recordIter != m_records.end();
			recordIter++)
		{
			for(std::vector<MemoryRecord>::iterator innerRecordIter = (*recordIter).begin(); 
				innerRecordIter != (*recordIter).end();
				innerRecordIter++)
			{
				if((char *)object == (*innerRecordIter).startAddress()) {
					object->~T();
					m_allocatedBytes -= (*innerRecordIter).size();
					(*recordIter).erase(innerRecordIter);
					return true;
				}
			}
		}

		return false;
	}
};

#endif