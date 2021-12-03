
#include "tlskey.h"

#ifdef WIN32
TLSKey::TLSKey()
{
	m_key = TlsAlloc();
}

TLSKey::~TLSKey()
{
	TlsFree(m_key);
}

void *TLSKey::GetKey()
{
	return TlsGetValue(m_key);
}

void TLSKey::SetKey(const void *value)
{
	TlsSetValue(m_key, (LPVOID)value);
}
#endif

#ifdef __linux__
TLSKey::TLSKey()
{
	pthread_key_create(&m_key, 0);
}

TLSKey::~TLSKey()
{
	pthread_key_delete(m_key);
}

void *TLSKey::GetKey()
{
	return pthread_getspecific(m_key);
}

void TLSKey::SetKey(const void *value)
{
	pthread_setspecific(m_key, value);
}
#endif
