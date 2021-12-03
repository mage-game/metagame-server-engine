
#ifndef BUFFER_H
#define BUFFER_H



class Buffer
{
 public:
	Buffer();
	~Buffer();
	Buffer(const Buffer&);
	Buffer& operator=(const Buffer&);

	void resize(unsigned long size);
	void grow_to(unsigned long size);
	void release();
	unsigned long size()const;
	
	char* ptr();
	const char* ptr()const;

 private:
	char* m_buffer;
	unsigned long m_size;
};


#endif /* BUFFER_H */
