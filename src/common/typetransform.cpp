
#include "typetransform.h"

#include <stdio.h>
#include <string.h>

TypeTransform::TypeTransform(int v)
{
	sprintf(buf,"%d",v);
}

TypeTransform::TypeTransform(float v)
{
	sprintf(buf,"%f",v);
}

TypeTransform::TypeTransform(double v)
{
	sprintf(buf,"%0.0f",v);
}
TypeTransform::TypeTransform(unsigned int v)
{
	sprintf(buf,"%d",v);
}
TypeTransform::TypeTransform(short v)
{
	sprintf(buf,"%d",v);
}
TypeTransform::TypeTransform(bool v)
{
	if(true == v)
	{
		strcpy(buf,"true");
	}
	else
	{
		strcpy(buf,"false");
	}
}
TypeTransform::TypeTransform(long long v)
{
	#ifdef _WIN32
	sprintf(buf,"%I64d",v);
	#else
	sprintf(buf,"%lld",v);
	#endif
}
TypeTransform::~TypeTransform()
{
}

const char *TypeTransform::ToString()
{
	return buf;
}

