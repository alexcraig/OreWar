#include "MemoryMgr.h"

// ========================================================================
// MemoryRecord Implementation
// ========================================================================
MemoryRecord::MemoryRecord(char * start, char * pageStart, int size)
	: mp_pageStart(pageStart), mp_start(start), m_size(size)
{
}

MemoryRecord::MemoryRecord(const MemoryRecord& copy)
	: mp_pageStart(copy.mp_pageStart), mp_start(copy.mp_start), m_size(copy.m_size)
{
}

char * MemoryRecord::page() const
{
	return mp_pageStart;
}

char * MemoryRecord::startAddress() const
{
	return mp_start;
}

int MemoryRecord::size() const
{
	return m_size;
}



// ========================================================================
// PagedMemoryPool Implementation
// ========================================================================
PagedMemoryPool::PagedMemoryPool(int pageSize, int initialPages)
	: mp_pages(), m_records(), m_nextPage(0), mp_nextByte(NULL), m_pageSize(pageSize),
	m_allocatedBytes(0)
{
	if(initialPages < 1) {
		initialPages = 1;
	}

	for(int i = 0; i < initialPages; i++) {
		addPage();
	}

	mp_nextByte = mp_pages[0];
}

void PagedMemoryPool::addPage()
{
	char * newPage = new char[m_pageSize];
	mp_pages.push_back(newPage);
	m_records.push_back(std::vector<MemoryRecord>());
}

int PagedMemoryPool::numPages() const 
{
	return mp_pages.size();
}

int PagedMemoryPool::currentPage() const 
{
	return m_nextPage;
}

int PagedMemoryPool::allocatedBytes() const
{
	return m_allocatedBytes;
}

int PagedMemoryPool::totalBytes() const
{
	return m_pageSize * mp_pages.size();
}